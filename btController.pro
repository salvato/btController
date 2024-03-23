#Copyright (C) 2023  Gabriele Salvato

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT += core
QT += gui
QT += widgets
QT += bluetooth


CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    btclient.cpp \
    button.cpp \
    edit.cpp \
    generalsetuparguments.cpp \
    generalsetupdialog.cpp \
    main.cpp \
    btscorecontroller.cpp \
    utility.cpp \
    volleyapplication.cpp \
    volleycontroller.cpp

HEADERS += \
    btclient.h \
    btscorecontroller.h \
    button.h \
    edit.h \
    generalsetuparguments.h \
    generalsetupdialog.h \
    panelorientation.h \
    utility.h \
    volleyapplication.h \
    volleycontroller.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/drawable-xhdpi/icon.png \
    android/res/drawable-xxhdpi/icon.png \
    android/res/drawable-xxxhdpi/icon.png \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml \
    resources/Logo_SSD_UniMe.png \
    resources/Logo_UniMe.png \
    resources/ball0.png \
    resources/ball1.png \
    resources/buttonIcons/Camera.png \
    resources/buttonIcons/ExchangeVolleyField.png \
    resources/buttonIcons/GeneralSetup.png \
    resources/buttonIcons/Minus.png \
    resources/buttonIcons/New-Game-Volley.png \
    resources/buttonIcons/New-Set-Volley.png \
    resources/buttonIcons/PanelSetup.png \
    resources/buttonIcons/PlaySlides.png \
    resources/buttonIcons/PlaySpots.png \
    resources/buttonIcons/Plus.png \
    resources/buttonIcons/image-missing.png \
    resources/buttonIcons/scad Models/Arrow.scad \
    resources/buttonIcons/scad Models/BasketField.scad \
    resources/buttonIcons/scad Models/ExchangeBasketField.scad \
    resources/buttonIcons/scadFiles/ExchangeVolleyField.scad \
    resources/buttonIcons/scadFiles/GeneralSetup.scad \
    resources/buttonIcons/scadFiles/NewGameVolley.scad \
    resources/buttonIcons/scadFiles/NewSetVolley.scad \
    resources/buttonIcons/scadFiles/PanelSetup.scad \
    resources/buttonIcons/scadFiles/Setup.scad \
    resources/buttonIcons/scadFiles/Slides.png \
    resources/buttonIcons/scadFiles/Slides.scad \
    resources/buttonIcons/scadFiles/Spots.scad \
    resources/buttonIcons/scadFiles/VolleyField.scad \
    resources/buttonIcons/scadFiles/VolleyPlayer.scad \
    resources/buttonIcons/sign_stop.png \
    resources/buttonIcons/video-display.png \
    resources/myLogo.ico \
    resources/myLogo.png

RESOURCES += \
    Resources.qrc

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
