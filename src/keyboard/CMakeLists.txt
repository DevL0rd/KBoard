add_executable(kboard main.cpp activeapp.cpp activeapp.h keyboardservice.cpp keyboardservice.h)

ecm_add_qml_module(kboard URI "org.devl0rd.kboard" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
ecm_target_qml_sources(kboard SOURCES
    qml/Main.qml
)

target_link_libraries(kboard PRIVATE
    Qt::Gui Qt::Qml Qt::Quick Qt::QuickControls2 Qt::DBus
    KF6::CoreAddons KF6::I18n KF6::I18nQml KF6::Crash
    KBoardConfig KBoardPlatform
)
target_include_directories(kboard PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(kboard PRIVATE KBOARD_SETTINGS_BINARY="${KDE_INSTALL_FULL_BINDIR}/kboard-settings")
add_dependencies(kboard KBoardUi)

ecm_finalize_qml_module(kboard DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS kboard ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
configure_file(org.devl0rd.kboard.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/org.devl0rd.kboard.desktop @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.devl0rd.kboard.desktop DESTINATION ${KDE_INSTALL_APPDIR})
