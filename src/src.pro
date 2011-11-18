include (../common.pri)
TEMPLATE = app
QT += dbus opengl declarative
CONFIG += qdbus mobility link_pkgconfig
PKGCONFIG += QtNetwork QtContacts libresourceqt1 libpulse-mainloop-glib ofono-qt
MOBILITY += contacts multimedia
MOC_DIR = .moc
OBJECTS_DIR = .obj
MGEN_OUTDIR = .gen
LIBS += -lseaside

DEFINES += CONFIG_DEFAULT_TARGET_UX=\\\"meego-ux-components\\\"

INCLUDEPATH += /var/tmp/build-root/usr/include

target.path = $$M_INSTALL_BIN

STYLE_HEADERS += peopleitemstyle.h \
    callitemstyle.h

MODEL_HEADERS += peopleitemmodel.h \
    callitemmodel.h

SOURCES += main.cpp \
    dialercontext.cpp \
    dialerapplication.cpp \
    managerproxy.cpp \
    historyproxy.cpp \
    modemproxy.cpp \
    networkproxy.cpp \
    peopleitem.cpp \
    callproxy.cpp \
    callmanager.cpp \
    callitem.cpp \
    callitemmodel.cpp \
    historytablemodel.cpp \
    dbustypes.cpp \
    resourceproxy.cpp \
    pacontrol.cpp \
    qmlmainwindow.cpp \
    qmldialer.cpp \
    qmlcallitem.cpp \
    dbusdialeradapter.cpp

HEADERS += \
    common.h \
    dialercontext.h \
    dialerapplication.h \
    managerproxy.h \
    historyproxy.h \
    modemproxy.h \
    networkproxy.h \
    peopleitem.h \
    callproxy.h \
    callmanager.h \
    callitem.h \
    historytablemodel.h \
    dbustypes.h \
    resourceproxy.h \
    notificationdialog.h \
    pacontrol.h \
    $$MODEL_HEADERS \
    $$STYLE_HEADERS \
    $$DBUS_INTERFACE_HEADERS \
    $$DBUS_ADAPTOR_HEADERS \
    qmlmainwindow.h \
    qmldialer.h \
    qmlcallitem.h \
    dbusdialeradapter.h

#DBUS_ADAPTORS += dbus/com.meego.dialer.xml

DBUS_INTERFACES += \
    dbus/org.ofono.manager.xml \
    dbus/org.ofono.modem.xml \
    dbus/org.ofono.operator.xml \
    dbus/org.ofono.voicecall.xml \
    dbus/org.ofono.history.xml

    system(qdbusxml2cpp -i dbustypes.h -p manager_interface.h: dbus/org.ofono.manager.xml)
    system(qdbusxml2cpp -i dbustypes.h -p modem_interface.h: dbus/org.ofono.modem.xml)
    system(qdbusxml2cpp -i dbustypes.h -p history_interface.h: dbus/org.ofono.history.xml)

MAKE_CLEAN += $$OBJECTS_DIR/*.o
MAKE_DISTCLEAN += \
    $$MOC_DIR/* $$MOC_DIR \
    $$OBJECTS_DIR/* $$OBJECTS_DIR \
    $$MGEN_OUTDIR/* $$MGEN_OUTDIR \

# Install
INSTALLS += target
