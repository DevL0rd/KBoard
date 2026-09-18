include(ECMAddTests)

ecm_add_test(clipboardhistorytest.cpp
    TEST_NAME clipboardhistorytest
    LINK_LIBRARIES Qt::Test Qt::Gui KBoardClipboard KBoardConfig
)
set_tests_properties(clipboardhistorytest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(clipboardpreview clipboardpreview.cpp)
target_link_libraries(clipboardpreview PRIVATE Qt::Gui Qt::Quick Qt::QuickControls2 KF6::I18n KF6::I18nQml KBoardClipboard KBoardConfig)
target_compile_definitions(clipboardpreview PRIVATE KBOARD_CLIPBOARD_PREVIEW_QML="${CMAKE_CURRENT_SOURCE_DIR}/preview/ClipboardPreview.qml")
