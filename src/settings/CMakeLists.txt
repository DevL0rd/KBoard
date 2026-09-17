find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Service)

add_library(KBoardSettingsApp SHARED
    store/edithistory.cpp
    store/edithistory.h
    store/settingsstore.cpp
    store/settingsstore.h
    catalog/settingscatalog.cpp
    catalog/settingscatalog.h
    system/appnavigation.cpp
    system/appnavigation.h
    system/aboutinfo.cpp
    system/aboutinfo.h
    system/applist.cpp
    system/applist.h
    system/focusdriver.cpp
    system/focusdriver.h
    system/systeminfo.cpp
    system/systeminfo.h
    system/systemstatus.cpp
    system/systemstatus.h
)
target_compile_options(KBoardSettingsApp PRIVATE -fvisibility=default)
target_include_directories(KBoardSettingsApp PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(KBoardSettingsApp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/store
    ${CMAKE_CURRENT_SOURCE_DIR}/catalog
    ${CMAKE_CURRENT_SOURCE_DIR}/system
)
target_link_libraries(KBoardSettingsApp
    PUBLIC Qt::Quick KBoardConfig
    PRIVATE Qt::DBus KF6::ConfigCore KF6::ConfigGui KF6::CoreAddons KF6::I18n KF6::Service
)
set_target_properties(KBoardSettingsApp PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
qt_add_resources(KBoardSettingsApp "kboard_settings_catalog" PREFIX "/kboard/settings" BASE catalog FILES catalog/index.json)

ecm_add_qml_module(KBoardSettingsApp URI "org.devl0rd.kboard.settings" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)

set(KBOARD_SETTINGS_QML_SINGLETONS
    qml/components/Motion.qml
    qml/bridges/Modules.qml
)
set_source_files_properties(${KBOARD_SETTINGS_QML_SINGLETONS} PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

file(GLOB KBOARD_SETTINGS_QML CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml qml/*/*.qml qml/*/*.js qml/*/*/*.qml qml/*/*/*.js)
ecm_target_qml_sources(KBoardSettingsApp SOURCES ${KBOARD_SETTINGS_QML})
ecm_finalize_qml_module(KBoardSettingsApp DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardSettingsApp ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

add_executable(kboard-settings app/main.cpp)
target_include_directories(kboard-settings PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(kboard-settings PRIVATE
    Qt::Widgets Qt::Quick Qt::QuickControls2
    KF6::CoreAddons KF6::I18n KF6::I18nQml KF6::Crash KF6::DBusAddons KF6::WindowSystem
    KBoardSettingsApp
)
install(TARGETS kboard-settings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.devl0rd.kboard.settings.desktop DESTINATION ${KDE_INSTALL_APPDIR})
