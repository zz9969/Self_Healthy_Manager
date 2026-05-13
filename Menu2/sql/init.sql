-- ============================================================
-- 饮食/运动健康管理系统 - 数据库初始化脚本 v4.0
-- 字符集: utf8mb4
-- 含中医体质辨识、智能推荐、关联表、缓存支持
-- v4.0 变更:
--   1. city_region_mapping 增加 weather_city_id (心知天气城市ID)
--   2. 新增 schema_migrations 版本管理表
--   3. password_hash 注释更新为 bcrypt
--   4. session_id 注释更新为 256bit 密码学安全随机数
-- ============================================================

CREATE DATABASE IF NOT EXISTS health_management
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_general_ci;

USE health_management;

-- -----------------------------------------------------------
-- 1. 用户表
-- -----------------------------------------------------------
CREATE TABLE users (
    user_id         INT AUTO_INCREMENT   PRIMARY KEY,
    username        VARCHAR(50)          NOT NULL UNIQUE,
    password_hash   VARCHAR(255)         NOT NULL COMMENT 'bcrypt哈希(cost=12)',
    gender          TINYINT              NOT NULL COMMENT '0-女 1-男',
    age             INT                  NOT NULL,
    height          DOUBLE               NOT NULL COMMENT '身高(cm)',
    weight          DOUBLE               NOT NULL COMMENT '体重(kg)',
    activity_level  TINYINT              DEFAULT 1 COMMENT '1-久坐 2-轻度 3-中度 4-高度 5-极高',
    create_time     DATETIME             DEFAULT CURRENT_TIMESTAMP,
    update_time     DATETIME             DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- -----------------------------------------------------------
-- 2. 用户会话表
-- -----------------------------------------------------------
CREATE TABLE user_sessions (
    session_id  VARCHAR(128) PRIMARY KEY COMMENT '256bit密码学安全随机数(64字符hex)',
    user_id     INT NOT NULL,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    expire_time DATETIME NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_user (user_id),
    INDEX idx_expire (expire_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户会话表';

-- -----------------------------------------------------------
-- 3. 中医体质问卷表 (5选项)
-- -----------------------------------------------------------
CREATE TABLE tcm_questionnaire (
    question_id   INT AUTO_INCREMENT   PRIMARY KEY,
    category      VARCHAR(20)          NOT NULL COMMENT '体质类别: 平和/气虚/阳虚/阴虚/痰湿/湿热/血瘀/气郁/特禀',
    question_text VARCHAR(500)         NOT NULL COMMENT '题目文本',
    option_1      VARCHAR(200)         NOT NULL COMMENT '选项1文本',
    option_1_score INT                 NOT NULL DEFAULT 1 COMMENT '选项1分值',
    option_2      VARCHAR(200)         NOT NULL COMMENT '选项2文本',
    option_2_score INT                 NOT NULL DEFAULT 2 COMMENT '选项2分值',
    option_3      VARCHAR(200)         NOT NULL COMMENT '选项3文本',
    option_3_score INT                 NOT NULL DEFAULT 3 COMMENT '选项3分值',
    option_4      VARCHAR(200)         NOT NULL COMMENT '选项4文本',
    option_4_score INT                 NOT NULL DEFAULT 4 COMMENT '选项4分值',
    option_5      VARCHAR(200)         NOT NULL COMMENT '选项5文本',
    option_5_score INT                 NOT NULL DEFAULT 5 COMMENT '选项5分值',
    sort_order    INT                  NOT NULL DEFAULT 0 COMMENT '排序号',
    is_active     TINYINT              DEFAULT 1 COMMENT '是否启用'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='中医体质问卷表';

INSERT INTO tcm_questionnaire (category, question_text, option_1, option_1_score, option_2, option_2_score, option_3, option_3_score, option_4, option_4_score, option_5, option_5_score, sort_order) VALUES
('气虚', '您容易感到疲乏、气短或说话无力吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 1),
('阳虚', '您手脚发凉、怕冷或吃凉的东西容易拉肚子吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 2),
('阴虚', '您感到手脚心发热、口干咽燥或皮肤干燥吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 3),
('痰湿', '您感到身体沉重、腹部胀满或嘴里发黏吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 4),
('湿热', '您面部油腻、易生痤疮或感到口苦吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 5),
('血瘀', '您皮肤容易出现青紫瘀斑、面色晦暗或有疼痛吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 6),
('气郁', '您容易闷闷不乐、精神紧张或无缘无故叹气吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 7),
('特禀', '您容易过敏（打喷嚏、起荨麻疹或皮肤一抓就红）吗？', '完全没有', 1, '很少', 2, '有时', 3, '经常', 4, '总是', 5, 8),
('平和', '您精力充沛、睡眠好吗？', '总是', 5, '经常', 4, '有时', 3, '很少', 2, '完全没有', 1, 9),
('平和', '您容易适应环境变化、心情愉快吗？', '总是', 5, '经常', 4, '有时', 3, '很少', 2, '完全没有', 1, 10);

-- -----------------------------------------------------------
-- 4. 用户问卷答题记录表
-- -----------------------------------------------------------
CREATE TABLE tcm_questionnaire_answers (
    answer_id   INT AUTO_INCREMENT   PRIMARY KEY,
    user_id     INT NOT NULL,
    question_id INT NOT NULL,
    score       INT NOT NULL COMMENT '用户选择的分值1-5',
    answer_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (question_id) REFERENCES tcm_questionnaire(question_id),
    UNIQUE KEY uk_user_question (user_id, question_id),
    INDEX idx_user (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户问卷答题记录表';

-- -----------------------------------------------------------
-- 5. 体质饮食指导参考表 (v3.0 新增)
-- -----------------------------------------------------------
CREATE TABLE constitution_diet_guides (
    guide_id     INT AUTO_INCREMENT   PRIMARY KEY,
    constitution VARCHAR(20)          NOT NULL UNIQUE COMMENT '体质类型',
    diet_advice  TEXT                  NOT NULL COMMENT '饮食建议概述',
    avoid_foods  TEXT                  NOT NULL COMMENT '忌口食物(JSON数组)',
    suitable_flavors TEXT              NOT NULL COMMENT '适宜性味(JSON数组)',
    suitable_food_categories TEXT      NOT NULL COMMENT '适宜食物类别(JSON数组)'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='体质饮食指导参考表';

INSERT INTO constitution_diet_guides (constitution, diet_advice, avoid_foods, suitable_flavors, suitable_food_categories) VALUES
('平和', '饮食均衡，不偏不倚，顺应四时', '[]', '["甘","淡"]', '["谷物","蔬菜","肉蛋","水果","奶豆"]'),
('气虚', '补气健脾，宜食温补益气之品', '["萝卜","空心菜","生冷食物"]', '["甘","微辛"]', '["谷物","肉蛋","汤粥","药膳"]'),
('阳虚', '温补阳气，宜食温热之品', '["冷饮","西瓜","苦瓜","绿豆"]', '["甘辛","温"]', '["汤粥","肉蛋","药膳"]'),
('阴虚', '滋阴润燥，宜食清补滋润之品', '["辣椒","羊肉","韭菜","煎炸食物"]', '["甘","微酸"]', '["汤粥","蔬菜","水果","奶豆"]'),
('痰湿', '健脾祛湿，宜食清淡利湿之品', '["肥肉","甜食","酒类","糯米"]', '["淡","微苦"]', '["蔬菜","谷物","汤粥"]'),
('湿热', '清热祛湿，宜食清淡凉性之品', '["辣椒","羊肉","酒类","甜食"]', '["苦","甘淡"]', '["蔬菜","汤粥","水果"]'),
('血瘀', '活血化瘀，宜食温通之品', '["寒凉食物","油腻食物"]', '["辛","酸甘"]', '["药膳","汤粥","蔬菜"]'),
('气郁', '疏肝理气，宜食行气解郁之品', '["咖啡","浓茶","辛辣刺激"]', '["辛","甘微苦"]', '["药膳","汤粥","蔬菜"]'),
('特禀', '益气固表，宜食平和抗敏之品', '["海鲜","芒果","花生","酒精"]', '["甘","淡"]', '["谷物","蔬菜","肉蛋"]');

-- -----------------------------------------------------------
-- 6. 用户体质结果表 (v3.0: 移除 diet_advice/avoid_foods/suitable_flavors)
-- -----------------------------------------------------------
CREATE TABLE tcm_constitution_results (
    result_id       INT AUTO_INCREMENT   PRIMARY KEY,
    user_id         INT                  NOT NULL,
    primary_type    VARCHAR(20)          NOT NULL COMMENT '主体质',
    secondary_type  VARCHAR(20)          DEFAULT NULL COMMENT '偏颇体质',
    score_pinghe    DOUBLE               DEFAULT 0,
    score_qixu      DOUBLE               DEFAULT 0,
    score_yangxu    DOUBLE               DEFAULT 0,
    score_yinxu     DOUBLE               DEFAULT 0,
    score_tanshi    DOUBLE               DEFAULT 0,
    score_shire     DOUBLE               DEFAULT 0,
    score_xueyu     DOUBLE               DEFAULT 0,
    score_qiyu      DOUBLE               DEFAULT 0,
    score_tebing    DOUBLE               DEFAULT 0,
    analyze_time    DATETIME             DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_user (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户体质结果表';

-- -----------------------------------------------------------
-- 7. 中医食谱表 (统一食物+食谱)
-- 注: meridian 为食谱固有属性，仅展示用途，保留逗号分隔格式
-- -----------------------------------------------------------
CREATE TABLE tcm_recipes (
    recipe_id          INT AUTO_INCREMENT   PRIMARY KEY,
    name               VARCHAR(100)         NOT NULL COMMENT '菜名',
    category           VARCHAR(20)          NOT NULL COMMENT '谷物/蔬菜/肉蛋/汤粥/药膳/水果/奶豆/其他',
    nature             VARCHAR(10)          NOT NULL DEFAULT '平' COMMENT '性: 寒/凉/平/温/热',
    flavor             VARCHAR(50)          DEFAULT '' COMMENT '味: 酸/苦/甘/辛/咸(可组合)',
    meridian           VARCHAR(100)         DEFAULT NULL COMMENT '归经(逗号分隔，固有属性仅展示)',
    efficacy           VARCHAR(200)         DEFAULT '' COMMENT '功效',
    calories_per_100g  DOUBLE               NOT NULL DEFAULT 0 COMMENT '每100g热量(kcal)',
    serving_size       DOUBLE               NOT NULL DEFAULT 100 COMMENT '每份克数',
    calories           DOUBLE               NOT NULL DEFAULT 0 COMMENT '每份热量(kcal)',
    protein            DOUBLE               DEFAULT 0 COMMENT '蛋白质(g/份)',
    fat                DOUBLE               DEFAULT 0 COMMENT '脂肪(g/份)',
    carbs              DOUBLE               DEFAULT 0 COMMENT '碳水化合物(g/份)',
    ingredients        TEXT                  DEFAULT NULL COMMENT '食材清单(JSON)',
    cooking_method     TEXT                  DEFAULT NULL COMMENT '做法描述',
    image_url          VARCHAR(255)         DEFAULT NULL COMMENT '图片路径',
    create_time        DATETIME             DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_category (category),
    INDEX idx_nature (nature)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='中医食谱表(统一)';

INSERT INTO tcm_recipes (name, category, nature, flavor, meridian, efficacy, calories_per_100g, serving_size, calories, protein, fat, carbs, ingredients, cooking_method) VALUES
('当归生姜羊肉汤', '汤粥', '温', '甘辛', '脾,肝', '温中补血,散寒止痛', 95, 400, 380, 28.5, 18.0, 15.2, '[{"name":"羊肉","amount":"200g"},{"name":"当归","amount":"10g"},{"name":"生姜","amount":"15g"}]', '羊肉焯水，与当归、生姜同炖1.5小时，加盐调味'),
('黄芪炖鸡', '汤粥', '温', '甘', '脾,肺', '补气固表,健脾益胃', 84, 500, 420, 35.0, 15.0, 22.0, '[{"name":"鸡肉","amount":"250g"},{"name":"黄芪","amount":"15g"},{"name":"红枣","amount":"5枚"}]', '鸡肉与黄芪、红枣同炖2小时，加盐调味'),
('红枣桂圆粥', '汤粥', '温', '甘', '脾,心', '补脾益气,养血安神', 70, 400, 280, 6.5, 3.0, 52.0, '[{"name":"大米","amount":"80g"},{"name":"红枣","amount":"8枚"},{"name":"桂圆","amount":"15g"}]', '大米煮粥，加入红枣、桂圆同煮30分钟'),
('绿豆薏米汤', '汤粥', '凉', '甘', '脾,胃', '清热解毒,利湿消肿', 36, 500, 180, 7.0, 1.5, 35.0, '[{"name":"绿豆","amount":"50g"},{"name":"薏米","amount":"30g"},{"name":"冰糖","amount":"适量"}]', '绿豆薏米浸泡后煮1小时，加冰糖'),
('冬瓜排骨汤', '汤粥', '凉', '甘淡', '肺,胃', '清热利湿,消肿解毒', 52, 500, 260, 18.0, 12.0, 14.0, '[{"name":"排骨","amount":"150g"},{"name":"冬瓜","amount":"200g"},{"name":"姜片","amount":"3片"}]', '排骨焯水，与冬瓜同炖1小时'),
('凉拌黄瓜', '蔬菜', '凉', '甘', '胃,大肠', '清热利水,解毒消肿', 23, 200, 45, 1.5, 0.5, 8.0, '[{"name":"黄瓜","amount":"200g"},{"name":"蒜","amount":"适量"},{"name":"醋","amount":"适量"}]', '黄瓜拍碎，加蒜末、醋、盐拌匀'),
('银耳百合莲子羹', '汤粥', '平', '甘', '肺,心', '滋阴润肺,养心安神', 39, 500, 195, 4.5, 2.0, 40.0, '[{"name":"银耳","amount":"15g"},{"name":"百合","amount":"10g"},{"name":"莲子","amount":"15g"},{"name":"冰糖","amount":"适量"}]', '银耳泡发，与百合莲子同煮1小时'),
('枸杞山药粥', '汤粥', '平', '甘', '脾,肺,肾', '滋阴补肾,健脾益胃', 48, 500, 240, 7.0, 2.5, 48.0, '[{"name":"大米","amount":"80g"},{"name":"山药","amount":"100g"},{"name":"枸杞","amount":"10g"}]', '大米与山药同煮粥，出锅前加枸杞'),
('薏米红豆粥', '汤粥', '平', '甘', '脾,胃', '健脾祛湿,利水消肿', 42, 500, 210, 7.5, 1.5, 42.0, '[{"name":"薏米","amount":"50g"},{"name":"红豆","amount":"50g"}]', '薏米红豆浸泡后煮1.5小时'),
('陈皮白术猪肚汤', '汤粥', '温', '苦辛甘', '脾,胃', '健脾理气,燥湿化痰', 62, 500, 310, 25.0, 14.0, 12.0, '[{"name":"猪肚","amount":"200g"},{"name":"陈皮","amount":"5g"},{"name":"白术","amount":"10g"}]', '猪肚洗净，与陈皮白术同炖2小时'),
('玫瑰花茶', '药膳', '温', '甘微苦', '肝,脾', '理气解郁,活血散瘀', 1, 200, 5, 0, 0, 1.0, '[{"name":"干玫瑰花","amount":"5g"},{"name":"冰糖","amount":"适量"}]', '玫瑰花沸水冲泡，加冰糖调味'),
('山楂红糖水', '药膳', '温', '酸甘', '肝,脾', '活血化瘀,温经散寒', 17, 500, 85, 0.5, 0.1, 20.0, '[{"name":"山楂","amount":"30g"},{"name":"红糖","amount":"15g"}]', '山楂煮水20分钟，加红糖'),
('五谷杂粮饭', '谷物', '平', '甘', '脾,胃', '健脾益气,营养均衡', 128, 250, 320, 9.0, 3.5, 60.0, '[{"name":"糙米","amount":"40g"},{"name":"小米","amount":"30g"},{"name":"红豆","amount":"20g"},{"name":"薏米","amount":"20g"}]', '杂粮浸泡2小时后蒸煮'),
('清蒸鲈鱼', '肉蛋', '平', '甘', '脾,肝', '健脾益胃,补肝肾', 74, 250, 185, 22.0, 8.5, 4.0, '[{"name":"鲈鱼","amount":"250g"},{"name":"葱姜","amount":"适量"},{"name":"蒸鱼豉油","amount":"适量"}]', '鲈鱼处理干净，清蒸8分钟，淋豉油'),
('番茄鸡蛋面', '谷物', '平', '酸甜', '脾,胃', '健脾开胃,营养均衡', 76, 500, 380, 14.0, 8.0, 58.0, '[{"name":"面条","amount":"150g"},{"name":"番茄","amount":"100g"},{"name":"鸡蛋","amount":"1个"}]', '番茄炒出汁，加水煮面，打入蛋花'),
('小米南瓜粥', '汤粥', '温', '甘', '脾,胃', '健脾和胃,补中益气', 44, 500, 220, 5.5, 2.0, 45.0, '[{"name":"小米","amount":"60g"},{"name":"南瓜","amount":"100g"}]', '小米与南瓜同煮30分钟'),
('豆浆配全麦馒头', '谷物', '平', '甘', '脾,胃', '补虚润燥,营养均衡', 62, 500, 310, 12.0, 5.0, 52.0, '[{"name":"豆浆","amount":"300ml"},{"name":"全麦馒头","amount":"100g"}]', '豆浆现磨，馒头蒸热即可'),
('山药粥', '汤粥', '平', '甘', '脾,肺,肾', '健脾补肺,益肾固精,抗过敏', 40, 500, 200, 5.0, 1.5, 42.0, '[{"name":"大米","amount":"60g"},{"name":"山药","amount":"80g"}]', '大米与山药同煮粥40分钟'),
('胡萝卜炒鸡胸肉', '肉蛋', '平', '甘', '脾,肺', '健脾益肺,增强免疫', 88, 250, 220, 24.0, 6.0, 12.0, '[{"name":"鸡胸肉","amount":"150g"},{"name":"胡萝卜","amount":"100g"}]', '鸡胸肉切片炒熟，加胡萝卜翻炒'),
('米饭', '谷物', '平', '甘', '脾,胃', '补中益气,健脾养胃', 116, 200, 232, 5.2, 0.6, 51.2, '[{"name":"大米","amount":"200g"}]', '大米加水蒸煮'),
('面条(煮)', '谷物', '平', '甘', '脾,胃', '补中益气', 110, 200, 220, 7.0, 1.0, 46.0, '[{"name":"面条","amount":"200g"}]', '清水煮面'),
('牛奶', '奶豆', '平', '甘', '脾,胃,肺', '补虚损,益肺胃', 54, 300, 162, 9.0, 9.6, 10.2, '[{"name":"牛奶","amount":"300ml"}]', '加热饮用'),
('鸡蛋', '肉蛋', '平', '甘', '脾,胃', '滋阴润燥,养血安胎', 144, 50, 72, 6.7, 4.4, 1.4, '[{"name":"鸡蛋","amount":"1个"}]', '煮/蒸/炒均可'),
('苹果', '水果', '平', '甘酸', '脾,肺', '生津润肺,健脾益胃', 53, 200, 106, 0.4, 0.4, 27.0, '[{"name":"苹果","amount":"200g"}]', '生食');

-- -----------------------------------------------------------
-- 8. 食谱-适配体质关联表 (v3.0: PK修正为 recipe_id+constitution)
-- -----------------------------------------------------------
CREATE TABLE recipe_constitution (
    recipe_id     INT NOT NULL,
    constitution  VARCHAR(20) NOT NULL COMMENT '气虚/阳虚/阴虚/痰湿/湿热/血瘀/气郁/特禀/平和',
    is_avoid      TINYINT DEFAULT 0 COMMENT '0-适配 1-忌用',
    PRIMARY KEY (recipe_id, constitution),
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id) ON DELETE CASCADE,
    INDEX idx_constitution (constitution, is_avoid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='食谱-适配体质关联表';

INSERT INTO recipe_constitution (recipe_id, constitution, is_avoid) VALUES
(1,'阳虚',0),(1,'气虚',0),(1,'阴虚',1),(1,'湿热',1),
(2,'气虚',0),(2,'阳虚',0),(2,'湿热',1),
(3,'气虚',0),(3,'血瘀',0),(3,'湿热',1),(3,'痰湿',1),
(4,'湿热',0),(4,'痰湿',0),(4,'阳虚',1),
(5,'湿热',0),(5,'痰湿',0),(5,'阳虚',1),
(6,'湿热',0),(6,'阴虚',0),(6,'阳虚',1),(6,'气虚',1),
(7,'阴虚',0),(7,'气郁',0),(7,'痰湿',1),(7,'阳虚',1),
(8,'阴虚',0),(8,'气虚',0),(8,'湿热',1),
(9,'痰湿',0),(9,'湿热',0),(9,'阴虚',1),
(10,'痰湿',0),(10,'气虚',0),(10,'阴虚',1),
(11,'气郁',0),(11,'血瘀',0),
(12,'血瘀',0),(12,'气郁',0),(12,'湿热',1),
(13,'平和',0),(13,'气虚',0),
(14,'平和',0),(14,'气虚',0),(14,'阴虚',0),
(15,'平和',0),(15,'气虚',0),
(16,'气虚',0),(16,'阳虚',0),(16,'平和',0),(16,'湿热',1),
(17,'平和',0),(17,'气虚',0),
(18,'特禀',0),(18,'气虚',0),(18,'平和',0),
(19,'特禀',0),(19,'气虚',0),(19,'平和',0),
(20,'平和',0),(20,'气虚',0),(20,'阳虚',0),
(21,'平和',0),(21,'气虚',0),
(22,'平和',0),(22,'阴虚',0),(22,'气虚',0),
(23,'平和',0),(23,'气虚',0),
(24,'平和',0),(24,'阴虚',0),(24,'气郁',0);

-- -----------------------------------------------------------
-- 9. 食谱-适配天气关联表
-- -----------------------------------------------------------
CREATE TABLE recipe_weather (
    recipe_id    INT NOT NULL,
    weather_label VARCHAR(20) NOT NULL COMMENT '严寒/酷暑/潮湿/干燥/常温',
    PRIMARY KEY (recipe_id, weather_label),
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id) ON DELETE CASCADE,
    INDEX idx_weather (weather_label)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='食谱-适配天气关联表';

INSERT INTO recipe_weather (recipe_id, weather_label) VALUES
(1,'严寒'),(1,'常温'),
(2,'严寒'),(2,'常温'),
(3,'严寒'),(3,'常温'),
(4,'酷暑'),(4,'潮湿'),
(5,'酷暑'),(5,'潮湿'),
(6,'酷暑'),(6,'干燥'),
(7,'干燥'),(7,'常温'),
(8,'干燥'),(8,'常温'),
(9,'潮湿'),(9,'常温'),
(10,'潮湿'),(10,'常温'),
(11,'常温'),
(12,'严寒'),(12,'常温'),
(13,'常温'),
(14,'常温'),
(15,'常温'),
(16,'严寒'),(16,'常温'),
(17,'常温'),
(18,'常温'),
(19,'常温'),
(20,'常温'),
(21,'常温'),
(22,'常温'),
(23,'常温'),
(24,'常温');

-- -----------------------------------------------------------
-- 10. 食谱-适配地域关联表
-- -----------------------------------------------------------
CREATE TABLE recipe_region (
    recipe_id    INT NOT NULL,
    region_label VARCHAR(20) NOT NULL COMMENT '北方/南方/湿热区/干燥区/通用',
    PRIMARY KEY (recipe_id, region_label),
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id) ON DELETE CASCADE,
    INDEX idx_region (region_label)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='食谱-适配地域关联表';

INSERT INTO recipe_region (recipe_id, region_label) VALUES
(1,'北方'),(1,'通用'),
(2,'北方'),(2,'通用'),
(3,'北方'),(3,'通用'),
(4,'南方'),(4,'湿热区'),
(5,'南方'),(5,'湿热区'),
(6,'南方'),(6,'通用'),
(7,'干燥区'),(7,'通用'),
(8,'干燥区'),(8,'通用'),
(9,'湿热区'),(9,'南方'),
(10,'湿热区'),(10,'南方'),
(11,'通用'),
(12,'通用'),
(13,'通用'),
(14,'通用'),
(15,'通用'),
(16,'北方'),(16,'通用'),
(17,'通用'),
(18,'通用'),
(19,'通用'),
(20,'通用'),
(21,'通用'),
(22,'通用'),
(23,'通用'),
(24,'通用');

-- -----------------------------------------------------------
-- 11. 食谱-适配季节关联表
-- -----------------------------------------------------------
CREATE TABLE recipe_season (
    recipe_id INT NOT NULL,
    season    VARCHAR(10) NOT NULL COMMENT '春/夏/秋/冬/全',
    PRIMARY KEY (recipe_id, season),
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='食谱-适配季节关联表';

INSERT INTO recipe_season (recipe_id, season) VALUES
(1,'冬'),(1,'秋'),
(2,'冬'),(2,'春'),
(3,'冬'),(3,'秋'),
(4,'夏'),(4,'春'),
(5,'夏'),
(6,'夏'),
(7,'秋'),(7,'冬'),
(8,'秋'),(8,'冬'),(8,'春'),
(9,'夏'),(9,'春'),
(10,'春'),(10,'夏'),
(11,'春'),(11,'夏'),
(12,'冬'),(12,'秋'),
(13,'全'),
(14,'全'),
(15,'全'),
(16,'秋'),(16,'冬'),
(17,'全'),
(18,'全'),
(19,'全'),
(20,'全'),
(21,'全'),
(22,'全'),
(23,'全'),
(24,'全');

-- -----------------------------------------------------------
-- 12. 食谱-适配餐次关联表
-- -----------------------------------------------------------
CREATE TABLE recipe_meal (
    recipe_id INT NOT NULL,
    meal_type TINYINT NOT NULL COMMENT '0-早餐 1-午餐 2-晚餐 3-全',
    PRIMARY KEY (recipe_id, meal_type),
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='食谱-适配餐次关联表';

INSERT INTO recipe_meal (recipe_id, meal_type) VALUES
(1,1),(1,2),
(2,1),(2,2),
(3,0),
(4,1),(4,0),
(5,1),(5,2),
(6,1),(6,2),
(7,0),(7,2),
(8,0),
(9,0),
(10,1),(10,2),
(11,0),(11,1),
(12,0),(12,1),
(13,1),(13,2),
(14,1),(14,2),
(15,1),(15,0),
(16,0),
(17,0),
(18,0),
(19,1),(19,2),
(20,1),(20,2),
(21,1),(21,0),
(22,0),
(23,0),(23,1),(23,2),
(24,3);

-- -----------------------------------------------------------
-- 13. 城市-地域气候映射表
-- -----------------------------------------------------------
CREATE TABLE city_region_mapping (
    mapping_id      INT AUTO_INCREMENT   PRIMARY KEY,
    city_name       VARCHAR(50)          NOT NULL UNIQUE COMMENT '城市名称',
    province        VARCHAR(50)          DEFAULT NULL COMMENT '省份',
    region_label    VARCHAR(20)          NOT NULL COMMENT '地域标签: 北方/南方/湿热区/干燥区/通用',
    weather_city_id VARCHAR(20)          DEFAULT NULL COMMENT '心知天气城市ID',
    climate_feature VARCHAR(100)         DEFAULT NULL COMMENT '气候特征描述',
    tcm_advice      VARCHAR(200)         DEFAULT NULL COMMENT '中医饮食建议',
    INDEX idx_province (province)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='城市-地域气候映射表';

INSERT INTO city_region_mapping (city_name, province, region_label, weather_city_id, climate_feature, tcm_advice) VALUES
('默认', NULL, '通用', NULL, '未识别城市', '饮食平和，根据体质和天气调整'),
('哈尔滨', '黑龙江', '北方', 'HRB', '冬季严寒漫长', '宜温补散寒，多食羊肉、红枣、生姜'),
('长春', '吉林', '北方', 'CGQ', '冬季寒冷', '宜温补，多食温热食物'),
('沈阳', '辽宁', '北方', 'SHE', '冬季寒冷干燥', '宜温补润燥'),
('北京', '北京', '北方', 'PEK', '四季分明，冬寒干燥', '秋冬宜温补润燥'),
('天津', '天津', '北方', 'TSN', '冬寒夏热', '冬宜温补，夏宜清淡'),
('石家庄', '河北', '北方', 'SJW', '冬寒干燥', '宜温补润燥'),
('济南', '山东', '北方', 'TNA', '冬寒夏热', '冬宜温补，夏宜清热'),
('太原', '山西', '北方', 'TYN', '干燥寒冷', '宜温补润燥'),
('呼和浩特', '内蒙古', '干燥区', 'HET', '干燥少雨，冬寒', '宜润燥养阴，多食梨、银耳'),
('乌鲁木齐', '新疆', '干燥区', 'URC', '极端干燥，昼夜温差大', '宜润燥养阴，忌辛辣燥热'),
('兰州', '甘肃', '干燥区', 'LHW', '干燥少雨', '宜润燥养阴'),
('银川', '宁夏', '干燥区', 'INC', '干燥少雨', '宜润燥养阴'),
('西宁', '青海', '干燥区', 'XNN', '高寒干燥', '宜温补润燥'),
('上海', '上海', '南方', 'SHA', '湿热多雨', '宜清热祛湿，饮食清淡'),
('南京', '江苏', '南方', 'NKG', '夏热冬冷', '夏宜清热，冬宜温补'),
('杭州', '浙江', '南方', 'HGH', '湿热多雨', '宜清热祛湿'),
('广州', '广东', '南方', 'CAN', '长夏湿热', '宜清热祛湿，多食薏米、冬瓜'),
('深圳', '广东', '南方', 'SZX', '长夏湿热', '宜清热祛湿'),
('南宁', '广西', '南方', 'NNG', '湿热多雨', '宜清热祛湿'),
('海口', '海南', '南方', 'HAK', '热带湿热', '宜清热祛湿，多食清凉食物'),
('福州', '福建', '南方', 'FOC', '湿热', '宜清热祛湿'),
('成都', '四川', '湿热区', 'CTU', '盆地湿热', '宜健脾祛湿，多食薏米、陈皮'),
('重庆', '重庆', '湿热区', 'CKG', '湿热多雾', '宜健脾祛湿，少食辛辣'),
('长沙', '湖南', '湿热区', 'CSX', '夏热多雨', '宜清热祛湿'),
('武汉', '湖北', '湿热区', 'WUH', '夏热冬冷，湿度大', '夏宜祛湿，冬宜温补'),
('南昌', '江西', '湿热区', 'KHN', '夏热多雨', '宜清热祛湿'),
('贵阳', '贵州', '湿热区', 'KWE', '湿润多雨', '宜祛湿健脾'),
('昆明', '云南', '南方', 'KMG', '四季如春', '饮食平和，适度清热'),
('西安', '陕西', '北方', 'SIA', '四季分明，偏干燥', '宜润燥，秋冬温补'),
('郑州', '河南', '北方', 'CGO', '四季分明', '冬宜温补，夏宜清淡'),
('合肥', '安徽', '南方', 'HFE', '夏热冬冷', '夏宜清热，冬宜温补');

-- -----------------------------------------------------------
-- 14. 用户食谱评分表
-- -----------------------------------------------------------
CREATE TABLE user_recipe_ratings (
    rating_id    INT AUTO_INCREMENT   PRIMARY KEY,
    user_id      INT                  NOT NULL,
    recipe_id    INT                  NOT NULL,
    rating       TINYINT              NOT NULL COMMENT '评分1-5',
    rating_time  DATETIME             DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id) ON DELETE CASCADE,
    UNIQUE KEY uk_user_recipe (user_id, recipe_id),
    INDEX idx_user (user_id),
    INDEX idx_recipe (recipe_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户食谱评分表';

-- -----------------------------------------------------------
-- 15. 用户个性化权重表
-- -----------------------------------------------------------
CREATE TABLE user_weight_profiles (
    user_id     INT PRIMARY KEY,
    w1 DOUBLE DEFAULT 0.25 COMMENT '体质权重',
    w2 DOUBLE DEFAULT 0.15 COMMENT '天气权重',
    w3 DOUBLE DEFAULT 0.10 COMMENT '地域权重',
    w4 DOUBLE DEFAULT 0.15 COMMENT '热量权重',
    w5 DOUBLE DEFAULT 0.10 COMMENT '营养权重',
    w6 DOUBLE DEFAULT 0.10 COMMENT '协同权重',
    w7 DOUBLE DEFAULT 0.05 COMMENT '季节权重',
    w8 DOUBLE DEFAULT 0.10 COMMENT '多样性权重',
    update_time DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户个性化权重';

-- -----------------------------------------------------------
-- 16. 运动类型表
-- -----------------------------------------------------------
CREATE TABLE exercise_types (
    type_id     INT AUTO_INCREMENT   PRIMARY KEY,
    name        VARCHAR(50)          NOT NULL COMMENT '运动名称',
    met_value   DOUBLE               NOT NULL COMMENT 'MET代谢当量',
    category    VARCHAR(20)          NOT NULL COMMENT '有氧/力量/柔韧',
    description VARCHAR(200)         DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='运动类型表';

INSERT INTO exercise_types (name, met_value, category, description) VALUES
('步行(慢)',     2.5,  '有氧', '3-4km/h 慢速步行'),
('步行(快)',     3.5,  '有氧', '5-6km/h 快速步行'),
('跑步(慢)',     7.0,  '有氧', '6-8km/h 慢跑'),
('跑步(快)',     11.0, '有氧', '10-12km/h 快跑'),
('游泳',         8.0,  '有氧', '自由泳中等强度'),
('骑行',         6.8,  '有氧', '中等速度骑行'),
('跳绳',         10.0, '有氧', '中等速度跳绳'),
('瑜伽',         3.0,  '柔韧', '哈他瑜伽'),
('力量训练',     5.0,  '力量', '综合力量训练'),
('篮球',         6.5,  '有氧', '一般强度篮球'),
('羽毛球',       5.5,  '有氧', '休闲羽毛球'),
('乒乓球',       4.0,  '有氧', '休闲乒乓球'),
('登山',         7.5,  '有氧', '中等坡度登山'),
('健身操',       6.0,  '有氧', '中等强度健身操');

-- -----------------------------------------------------------
-- 17. 运动记录表
-- -----------------------------------------------------------
CREATE TABLE exercise_records (
    record_id       INT AUTO_INCREMENT   PRIMARY KEY,
    user_id         INT                  NOT NULL,
    type_id         INT                  NOT NULL,
    duration        INT                  NOT NULL COMMENT '运动时长(分钟)',
    intensity       TINYINT              NOT NULL DEFAULT 2 COMMENT '1-低 2-中 3-高',
    calories_burned DOUBLE               NOT NULL COMMENT '消耗热量(kcal)',
    record_date     DATE                 NOT NULL,
    create_time     DATETIME             DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (type_id) REFERENCES exercise_types(type_id),
    INDEX idx_user_date (user_id, record_date)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='运动记录表';

-- -----------------------------------------------------------
-- 18. 饮食推荐计划表
-- -----------------------------------------------------------
CREATE TABLE meal_plans (
    plan_id         INT AUTO_INCREMENT   PRIMARY KEY,
    user_id         INT                  NOT NULL,
    plan_date       DATE                 NOT NULL,
    meal_type       TINYINT              NOT NULL COMMENT '0-早餐 1-午餐 2-晚餐',
    target_calories DOUBLE               NOT NULL COMMENT '本顿目标热量(kcal)',
    constitution_type VARCHAR(20)        DEFAULT NULL COMMENT '推荐时体质快照',
    weather_label   VARCHAR(20)          DEFAULT NULL COMMENT '推荐时天气标签快照',
    region_label    VARCHAR(20)          DEFAULT NULL COMMENT '推荐时地域标签快照',
    season_label    VARCHAR(20)          DEFAULT NULL COMMENT '推荐时季节快照',
    create_time     DATETIME             DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    UNIQUE KEY uk_user_date_meal (user_id, plan_date, meal_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='饮食推荐计划表';

-- -----------------------------------------------------------
-- 19. 推荐计划食谱项表
-- -----------------------------------------------------------
CREATE TABLE meal_plan_items (
    item_id     INT AUTO_INCREMENT   PRIMARY KEY,
    plan_id     INT                  NOT NULL,
    recipe_id   INT                  NOT NULL,
    quantity    DOUBLE               NOT NULL COMMENT '建议份数',
    calories    DOUBLE               NOT NULL COMMENT '该项热量(kcal)',
    score       DOUBLE               DEFAULT 0 COMMENT 'AI打分',
    match_reason TEXT                 DEFAULT NULL COMMENT '适配原因(JSON)',
    sort_order  INT                  DEFAULT 0,
    FOREIGN KEY (plan_id) REFERENCES meal_plans(plan_id) ON DELETE CASCADE,
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id),
    INDEX idx_plan (plan_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='推荐计划食谱项表';

-- -----------------------------------------------------------
-- 20. 饮食记录表 (引用 tcm_recipes)
-- -----------------------------------------------------------
CREATE TABLE diet_records (
    record_id   INT AUTO_INCREMENT   PRIMARY KEY,
    user_id     INT                  NOT NULL,
    meal_type   TINYINT              NOT NULL COMMENT '0-早餐 1-午餐 2-晚餐 3-加餐',
    recipe_id   INT                  NOT NULL COMMENT '关联tcm_recipes',
    quantity    DOUBLE               NOT NULL COMMENT '实际摄入份数',
    calories    DOUBLE               NOT NULL COMMENT '摄入热量(kcal)',
    protein     DOUBLE               DEFAULT 0 COMMENT '蛋白质(g)',
    fat         DOUBLE               DEFAULT 0 COMMENT '脂肪(g)',
    carbs       DOUBLE               DEFAULT 0 COMMENT '碳水化合物(g)',
    input_mode  TINYINT              NOT NULL DEFAULT 1 COMMENT '0-自动填充 1-手动输入',
    record_date DATE                 NOT NULL,
    create_time DATETIME             DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (recipe_id) REFERENCES tcm_recipes(recipe_id),
    INDEX idx_user_date (user_id, record_date),
    INDEX idx_user_date_meal (user_id, record_date, meal_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='饮食记录表';

-- -----------------------------------------------------------
-- 21. 每日报告表
-- -----------------------------------------------------------
CREATE TABLE daily_reports (
    report_id       INT AUTO_INCREMENT   PRIMARY KEY,
    user_id         INT                  NOT NULL,
    report_date     DATE                 NOT NULL,
    target_calories DOUBLE               NOT NULL COMMENT '目标热量(kcal)',
    total_calories_in  DOUBLE            NOT NULL DEFAULT 0 COMMENT '总摄入热量(kcal)',
    total_calories_out DOUBLE            NOT NULL DEFAULT 0 COMMENT '运动消耗热量(kcal)',
    net_calories    DOUBLE               NOT NULL DEFAULT 0 COMMENT '净热量(kcal)',
    total_protein   DOUBLE               DEFAULT 0 COMMENT '总蛋白质(g)',
    total_fat       DOUBLE               DEFAULT 0 COMMENT '总脂肪(g)',
    total_carbs     DOUBLE               DEFAULT 0 COMMENT '总碳水化合物(g)',
    completion_rate DOUBLE               DEFAULT 0 COMMENT '达标率(%)',
    is_stale        TINYINT              DEFAULT 0 COMMENT '0-最新 1-需刷新',
    create_time     DATETIME             DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    UNIQUE KEY uk_user_date (user_id, report_date)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='每日报告表';

-- -----------------------------------------------------------
-- 22. 数据库版本管理表 (v4.0 新增)
-- -----------------------------------------------------------
CREATE TABLE schema_migrations (
    version     INT PRIMARY KEY,
    description VARCHAR(200) NOT NULL,
    applied_at  DATETIME DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='数据库版本管理表';

INSERT INTO schema_migrations (version, description) VALUES
(1, 'v4.0 初始化: 22张表, bcrypt密码, 心知天气城市ID, 关联表规范化');
