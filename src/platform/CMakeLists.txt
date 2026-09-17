add_library(KBoardPlatform SHARED)

ecm_add_qtwayland_client_protocol(KBoardPlatform PROTOCOL ${WaylandProtocols_DATADIR}/unstable/input-method/input-method-unstable-v1.xml BASENAME input-method-unstable-v1)
ecm_add_qtwayland_client_protocol(KBoardPlatform PROTOCOL "${Wayland_DATADIR}/wayland.xml" BASENAME wayland)

target_sources(KBoardPlatform PRIVATE
    inputcontext.cpp
    inputcontext.h
    keycodes.cpp
    keycodes.h
    inputmethod.cpp
    inputmethod_p.h
    inputpanelintegration.cpp
    inputpanelintegration.h
    inputpanelrole.h
    panelwindow.cpp
    panelwindow.h
    qwaylandinputpanelshellintegration.cpp
    qwaylandinputpanelshellintegration_p.h
    qwaylandinputpanelsurface.cpp
    qwaylandinputpanelsurface_p.h
)

target_include_directories(KBoardPlatform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(KBoardPlatform
    PUBLIC Qt::Core Qt::Gui Qt::Qml Qt::Quick
    PRIVATE Qt::GuiPrivate Qt::WaylandClient Qt::WaylandClientPrivate Wayland::Client PkgConfig::XKBCommon KF6::WindowSystem
)
set_target_properties(KBoardPlatform PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
target_compile_options(KBoardPlatform PRIVATE -fvisibility=default)

ecm_add_qml_module(KBoardPlatform URI "org.devl0rd.kboard.platform" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
ecm_finalize_qml_module(KBoardPlatform DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardPlatform ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
