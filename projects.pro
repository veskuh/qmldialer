include (common.pri)
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = src qml translations

#OTHER_FILES += dialer.service
OTHER_FILES += *.service *.desktop *.sh

# Keepalive script
#keepalive_script.files = dialer-keepalive.sh
#keepalive_script.path = $$M_INSTALL_BIN
#keepalive_script.CONFIG += no_check_exist

# XDG Autostart
#autostart_entry.files = dialer.desktop
autostart_entry.files = dialer-prestart.desktop
autostart_entry.path = $$M_XDG_DIR/autostart
autostart_entry.CONFIG += no_check_exist

# Desktop
desktop_entry.files = dialer.desktop
desktop_entry.path = $$M_INSTALL_DATA/applications
desktop_entry.CONFIG += no_check_exist

application_icon.files = themes/base/icons/icons-Applications-dialer.svg
application_icon.path = $$M_INSTALL_DATA/pixmaps
application_icon.CONFIG += no_check_exist

# DBus service
dbus_service.files = dialer.service
dbus_service.path = $$M_DBUS_SERVICES_DIR

# Tools directory
tools_directory.files = tools
tools_directory.path  = $$INSTALL_ROOT/usr/share/meego-handset-dialer/

# Documentation
documentation.files = AUTHORS ChangeLog LICENSE README TODO
documentation.path  = $$INSTALL_ROOT/usr/share/meego-handset-dialer/

INSTALLS += \
    autostart_entry \
    desktop_entry \
    application_icon \
    dbus_service \
    tools_directory \
    documentation
