-- ============================================
-- 中医九种体质问卷模块 - 独立建表脚本
-- ============================================

DROP TABLE IF EXISTS tcm_constitution_results;
DROP TABLE IF EXISTS tcm_question_options;
DROP TABLE IF EXISTS tcm_questions;

CREATE TABLE tcm_questions (
    question_id INT PRIMARY KEY AUTO_INCREMENT,
    question_text VARCHAR(200) NOT NULL COMMENT '题目文本',
    sort_order INT NOT NULL COMMENT '排序序号'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='中医体质问卷题目表';

CREATE TABLE tcm_question_options (
    id INT PRIMARY KEY AUTO_INCREMENT,
    question_id INT NOT NULL COMMENT '关联题目ID',
    option_key CHAR(1) NOT NULL COMMENT '选项标识 A/B/C/D/E',
    option_text VARCHAR(20) NOT NULL COMMENT '选项文本',
    score_pinghe DECIMAL(5,2) DEFAULT 0 COMMENT '平和质分值',
    score_qixu   DECIMAL(5,2) DEFAULT 0 COMMENT '气虚质分值',
    score_yangxu DECIMAL(5,2) DEFAULT 0 COMMENT '阳虚质分值',
    score_yinxu  DECIMAL(5,2) DEFAULT 0 COMMENT '阴虚质分值',
    score_tanshi DECIMAL(5,2) DEFAULT 0 COMMENT '痰湿质分值',
    score_shire  DECIMAL(5,2) DEFAULT 0 COMMENT '湿热质分值',
    score_xueyu  DECIMAL(5,2) DEFAULT 0 COMMENT '血瘀质分值',
    score_qiyu   DECIMAL(5,2) DEFAULT 0 COMMENT '气郁质分值',
    score_tebing DECIMAL(5,2) DEFAULT 0 COMMENT '特禀质分值',
    FOREIGN KEY (question_id) REFERENCES tcm_questions(question_id) ON DELETE CASCADE,
    UNIQUE KEY uk_qid_opt (question_id, option_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='问卷选项及九种体质分值表';

CREATE TABLE tcm_constitution_results (
    result_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL COMMENT '用户ID',
    score_pinghe DECIMAL(5,2) DEFAULT 0 COMMENT '平和质总分',
    score_qixu   DECIMAL(5,2) DEFAULT 0 COMMENT '气虚质总分',
    score_yangxu DECIMAL(5,2) DEFAULT 0 COMMENT '阳虚质总分',
    score_yinxu  DECIMAL(5,2) DEFAULT 0 COMMENT '阴虚质总分',
    score_tanshi DECIMAL(5,2) DEFAULT 0 COMMENT '痰湿质总分',
    score_shire  DECIMAL(5,2) DEFAULT 0 COMMENT '湿热质总分',
    score_xueyu  DECIMAL(5,2) DEFAULT 0 COMMENT '血瘀质总分',
    score_qiyu   DECIMAL(5,2) DEFAULT 0 COMMENT '气郁质总分',
    score_tebing DECIMAL(5,2) DEFAULT 0 COMMENT '特禀质总分',
    primary_type VARCHAR(20) NOT NULL COMMENT '判定主体质',
    test_date DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '测评时间',
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户体质测评结果表';

-- ============================================
-- 10道问卷题目
-- 偏颇质题目: A=从不(1分) B=偶尔(2分) C=有时(3分) D=经常(4分) E=总是(5分)
--   对应偏颇质加对应分, 平和质加反向分(5→1)
-- 平和质题目: A=总是(5分) B=经常(4分) C=有时(3分) D=偶尔(2分) E=从不(1分)
--   对应平和质加正向分, 偏颇质不加分
-- ============================================

INSERT INTO tcm_questions (question_id, question_text, sort_order) VALUES
(1,  '您容易感到疲乏吗？',               1),
(2,  '您手脚发凉吗？',                   2),
(3,  '您感觉口干咽燥吗？',               3),
(4,  '您感到胸闷或腹部胀满吗？',          4),
(5,  '您面部或鼻部有油腻感或油亮发光吗？', 5),
(6,  '您皮肤常在不知不觉中出现青紫瘀斑吗？', 6),
(7,  '您感到闷闷不乐、情绪低沉吗？',      7),
(8,  '您容易过敏（对药物、食物、气味、花粉等）吗？', 8),
(9,  '您精力充沛吗？',                   9),
(10, '您适应外界自然和社会环境变化吗？',   10);

-- Q1 气虚质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(1, 'A', '从不', 5, 1, 0, 0, 0, 0, 0, 0, 0),
(1, 'B', '偶尔', 4, 2, 0, 0, 0, 0, 0, 0, 0),
(1, 'C', '有时', 3, 3, 0, 0, 0, 0, 0, 0, 0),
(1, 'D', '经常', 2, 4, 0, 0, 0, 0, 0, 0, 0),
(1, 'E', '总是', 1, 5, 0, 0, 0, 0, 0, 0, 0);

-- Q2 阳虚质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(2, 'A', '从不', 5, 0, 1, 0, 0, 0, 0, 0, 0),
(2, 'B', '偶尔', 4, 0, 2, 0, 0, 0, 0, 0, 0),
(2, 'C', '有时', 3, 0, 3, 0, 0, 0, 0, 0, 0),
(2, 'D', '经常', 2, 0, 4, 0, 0, 0, 0, 0, 0),
(2, 'E', '总是', 1, 0, 5, 0, 0, 0, 0, 0, 0);

-- Q3 阴虚质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(3, 'A', '从不', 5, 0, 0, 1, 0, 0, 0, 0, 0),
(3, 'B', '偶尔', 4, 0, 0, 2, 0, 0, 0, 0, 0),
(3, 'C', '有时', 3, 0, 0, 3, 0, 0, 0, 0, 0),
(3, 'D', '经常', 2, 0, 0, 4, 0, 0, 0, 0, 0),
(3, 'E', '总是', 1, 0, 0, 5, 0, 0, 0, 0, 0);

-- Q4 痰湿质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(4, 'A', '从不', 5, 0, 0, 0, 1, 0, 0, 0, 0),
(4, 'B', '偶尔', 4, 0, 0, 0, 2, 0, 0, 0, 0),
(4, 'C', '有时', 3, 0, 0, 0, 3, 0, 0, 0, 0),
(4, 'D', '经常', 2, 0, 0, 0, 4, 0, 0, 0, 0),
(4, 'E', '总是', 1, 0, 0, 0, 5, 0, 0, 0, 0);

-- Q5 湿热质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(5, 'A', '从不', 5, 0, 0, 0, 0, 1, 0, 0, 0),
(5, 'B', '偶尔', 4, 0, 0, 0, 0, 2, 0, 0, 0),
(5, 'C', '有时', 3, 0, 0, 0, 0, 3, 0, 0, 0),
(5, 'D', '经常', 2, 0, 0, 0, 0, 4, 0, 0, 0),
(5, 'E', '总是', 1, 0, 0, 0, 0, 5, 0, 0, 0);

-- Q6 血瘀质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(6, 'A', '从不', 5, 0, 0, 0, 0, 0, 1, 0, 0),
(6, 'B', '偶尔', 4, 0, 0, 0, 0, 0, 2, 0, 0),
(6, 'C', '有时', 3, 0, 0, 0, 0, 0, 3, 0, 0),
(6, 'D', '经常', 2, 0, 0, 0, 0, 0, 4, 0, 0),
(6, 'E', '总是', 1, 0, 0, 0, 0, 0, 5, 0, 0);

-- Q7 气郁质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(7, 'A', '从不', 5, 0, 0, 0, 0, 0, 0, 1, 0),
(7, 'B', '偶尔', 4, 0, 0, 0, 0, 0, 0, 2, 0),
(7, 'C', '有时', 3, 0, 0, 0, 0, 0, 0, 3, 0),
(7, 'D', '经常', 2, 0, 0, 0, 0, 0, 0, 4, 0),
(7, 'E', '总是', 1, 0, 0, 0, 0, 0, 0, 5, 0);

-- Q8 特禀质题目
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(8, 'A', '从不', 5, 0, 0, 0, 0, 0, 0, 0, 1),
(8, 'B', '偶尔', 4, 0, 0, 0, 0, 0, 0, 0, 2),
(8, 'C', '有时', 3, 0, 0, 0, 0, 0, 0, 0, 3),
(8, 'D', '经常', 2, 0, 0, 0, 0, 0, 0, 0, 4),
(8, 'E', '总是', 1, 0, 0, 0, 0, 0, 0, 0, 5);

-- Q9 平和质题目（正向计分：A=总是得5分...E=从不得1分）
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(9, 'A', '总是', 5, 0, 0, 0, 0, 0, 0, 0, 0),
(9, 'B', '经常', 4, 0, 0, 0, 0, 0, 0, 0, 0),
(9, 'C', '有时', 3, 0, 0, 0, 0, 0, 0, 0, 0),
(9, 'D', '偶尔', 2, 0, 0, 0, 0, 0, 0, 0, 0),
(9, 'E', '从不', 1, 0, 0, 0, 0, 0, 0, 0, 0);

-- Q10 平和质题目（正向计分）
INSERT INTO tcm_question_options (question_id, option_key, option_text, score_pinghe, score_qixu, score_yangxu, score_yinxu, score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES
(10, 'A', '总是', 5, 0, 0, 0, 0, 0, 0, 0, 0),
(10, 'B', '经常', 4, 0, 0, 0, 0, 0, 0, 0, 0),
(10, 'C', '有时', 3, 0, 0, 0, 0, 0, 0, 0, 0),
(10, 'D', '偶尔', 2, 0, 0, 0, 0, 0, 0, 0, 0),
(10, 'E', '从不', 1, 0, 0, 0, 0, 0, 0, 0, 0);
