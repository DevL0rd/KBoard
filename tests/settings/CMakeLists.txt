find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets)

include(ECMAddTests)

ecm_add_test(tst_settingsindex.cpp
    TEST_NAME tst_settingsindex
    LINK_LIBRARIES Qt::Test KBoardSettingsApp
)
target_compile_definitions(tst_settingsindex PRIVATE
    KBOARD_SETTINGS_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../src/settings"
    KBOARD_KCFG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../src/config/kboardsettings.kcfg"
)

ecm_add_test(tst_settingsstore.cpp
    TEST_NAME tst_settingsstore
    LINK_LIBRARIES Qt::Test KBoardSettingsApp KF6::ConfigCore
)

add_executable(settingsshot settingsshot.cpp)
target_link_libraries(settingsshot PRIVATE Qt::Widgets Qt::Quick Qt::QuickControls2 KBoardSettingsApp)
