set(FCITX_XKBDBUS_LINK_LIBS)
if(_ENABLE_XKBDBUS)
  include_directories(
    ${DBUS_INCLUDE_DIRS})
  set(FCITX_XKBDBUS_LINK_LIBS ${FCITX_XKBDBUS_LINK_LIBS}
    ${DBUS_LIBRARIES})
  link_directories(${DBUS_LIBRARY_DIRS})
  set(xkbdbus_noinstall)
else()
  set(xkbdbus_noinstall NO_INSTALL)
endif()

set(FCITX_XKBDBUS_SOURCES
  xkbdbus.c
  ../../im/keyboard/isocodes.c)
fcitx_add_addon_full(xkbdbus SCAN SCAN_PRIV ${xkbdbus_noinstall}
  SOURCES ${FCITX_XKBDBUS_SOURCES}
  LINK_LIBS ${FCITX_XKBDBUS_LINK_LIBS} PkgConfig::JsonC)
