pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3>=3.2)
pkg_check_modules(Libudev REQUIRED IMPORTED_TARGET libudev)

add_library(KBoardGamepad SHARED
    buttonsemantics.cpp
    buttonsemantics.h
    chordmatcher.cpp
    chordmatcher.h
    gamepad.cpp
    gamepad.h
    devicetracker.cpp
    devicetracker.h
    gamepadinput.cpp
    gamepadbackend.cpp
    gamepadbackend.h
    gamepadbuttons.cpp
    gamepadbuttons.h
    gamepaddevice.h
    descriptorscanner.cpp
    descriptorscanner.h
    hotplugmonitor.cpp
    hotplugmonitor.h
    pollset.cpp
    pollset.h
    navigationrepeat.cpp
    navigationrepeat.h
    steamdedupe.cpp
    steamdedupe.h
)
target_compile_options(KBoardGamepad PRIVATE -fvisibility=default)
target_include_directories(KBoardGamepad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(KBoardGamepad
    PUBLIC Qt::Core Qt::Qml Qt::Quick KBoardConfig
    PRIVATE PkgConfig::SDL3 PkgConfig::Libudev
)
set_target_properties(KBoardGamepad PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)

ecm_add_qml_module(KBoardGamepad URI "org.devl0rd.kboard.gamepad" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
ecm_finalize_qml_module(KBoardGamepad DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardGamepad ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
