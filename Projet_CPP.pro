QT += widgets sql charts printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    connection.cpp \
    cours.cpp \
    salle.cpp \
    document.cpp

HEADERS += \
    mainwindow.h \
    connection.h \
    cours.h \
    salle.h \
    document.h

win32 {
    # Oracle Database 11.2 XE - dossier bin contenant oci.dll et sqora32.dll
    ORA_BIN = C:/Users/USER/OneDrive/Bureau/app/oracle/product/11.2.0/server/bin

    # Crée les dossiers de sortie
    QMAKE_POST_LINK += "cmd /c if not exist \"$$OUT_PWD\\debug\" mkdir \"$$OUT_PWD\\debug\""
    QMAKE_POST_LINK += "cmd /c if not exist \"$$OUT_PWD\\release\" mkdir \"$$OUT_PWD\\release\""

    # Copie oci.dll Oracle dans les dossiers de sortie (nécessaire pour QOCI si disponible)
    QMAKE_POST_LINK += "cmd /c copy /Y \"$${ORA_BIN}\\oci.dll\" \"$$OUT_PWD\\debug\\\""
    QMAKE_POST_LINK += "cmd /c copy /Y \"$${ORA_BIN}\\oci.dll\" \"$$OUT_PWD\\release\\\""
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target