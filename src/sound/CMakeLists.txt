add_library(KBoardSound SHARED
    audiooutput.cpp
    audiooutput.h
    haptics.cpp
    haptics.h
    keysound.cpp
    keysound.h
    soundmixer.cpp
    soundmixer.h
    soundpack.cpp
    soundpack.h
)
target_compile_options(KBoardSound PRIVATE -fvisibility=default)
target_include_directories(KBoardSound PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(KBoardSound
    PUBLIC Qt::Core Qt::Qml Qt::Quick
    PRIVATE Qt::Multimedia Qt::DBus KBoardConfig
)
set_target_properties(KBoardSound PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
ecm_add_qml_module(KBoardSound URI "org.devl0rd.kboard.sound" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
ecm_finalize_qml_module(KBoardSound DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardSound ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
