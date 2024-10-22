VERSION = 1.1.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
ANDROID_VERSION_NAME = $$VERSION
ANDROID_VERSION_CODE = 2

QT += quick qml concurrent

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

CONFIG += qmltypes
QML_IMPORT_NAME = com.LFWQSP2641.qmlcomponents
QML_IMPORT_MAJOR_VERSION = 1

win32{
QMAKE_TARGET_PRODUCT = "Star"

QMAKE_TARGET_COPYRIGHT = "Copyright © 2024 - 2024 LFWQSP2641.All Rights Reserved."

RC_LANG = 0x0004
}


INCLUDEPATH += \
    src

HEADERS += \
    src/LaunchingBall.h \
    src/ThreeBodyMotion.h

SOURCES += \
        src/LaunchingBall.cpp \
        src/ThreeBodyMotion.cpp \
        src/main.cpp

RESOURCES += \
    resource/ui.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
