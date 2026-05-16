QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    api_client.cpp \
    constitution.cpp \
    eatwhat.cpp \
    foodlist.cpp \
    main.cpp \
    logindialog.cpp \
    mainpage.cpp \
    realeat.cpp \
    runlist.cpp \
    runpage.cpp \
    showrun.cpp \
    today.cpp \
    tourist.cpp

HEADERS += \
    api_client.h \
    constitution.h \
    runlist.h \
    runpage.h \
    showrun.h \
    viewmodels/login_viewmodel.h \
    viewmodels/tourist_viewmodel.h \
    viewmodels/mainpage_viewmodel.h \
    viewmodels/eatwhat_viewmodel.h \
    viewmodels/foodlist_viewmodel.h \
    viewmodels/realeat_viewmodel.h \
    viewmodels/today_viewmodel.h \
    viewmodels/constitution_viewmodel.h \
    eatwhat.h \
    foodlist.h \
    logindialog.h \
    mainpage.h \
    realeat.h \
    today.h \
    tourist.h

FORMS += \
    constitution.ui \
    eatwhat.ui \
    foodlist.ui \
    logindialog.ui \
    mainpage.ui \
    realeat.ui \
    runlist.ui \
    runpage.ui \
    showrun.ui \
    today.ui \
    tourist.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
