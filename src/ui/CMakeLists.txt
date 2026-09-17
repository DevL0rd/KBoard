add_library(KBoardUi SHARED
    glidetrail.cpp
    glidetrail.h
    keyboardsurface.cpp
    keyboardsurface.h
    keyentry.h
    trailsection.h
    keylayout.cpp
    keylayout.h
    keygestures.cpp
    keygestures.h
    keymodel.cpp
    keymodel.h
    keynames.cpp
    keynames.h
    layouts.cpp
    layouts.h
)
target_compile_options(KBoardUi PRIVATE -fvisibility=default)
target_include_directories(KBoardUi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(KBoardUi PUBLIC Qt::Quick PRIVATE KBoardConfig KBoardPlatform)
set_target_properties(KBoardUi PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

ecm_add_qml_module(KBoardUi URI "org.devl0rd.kboard.ui" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
ecm_target_qml_sources(KBoardUi SOURCES
    qml/AccentPopup.qml
    qml/AppRules.qml
    qml/ClipboardChip.qml
    qml/ControllerNavigation.qml
    qml/CorrectionFlash.qml
    qml/EditPanel.qml
    qml/FieldProfile.qml
    qml/FocusRing.qml
    qml/KeyBubble.qml
    qml/KeyCap.qml
    qml/KeyIcon.qml
    qml/KeyInput.qml
    qml/KeyModifiers.qml
    qml/KeyRipple.qml
    qml/Keyboard.qml
    qml/KeyboardBackdrop.qml
    qml/KeyboardView.qml
    qml/LongPressRing.qml
    qml/PanelHost.qml
    qml/PanelKey.qml
    qml/PanelSlot.qml
    qml/PreviewDemo.qml
    qml/SearchKeys.qml
    qml/SoftShadow.qml
    qml/StripButton.qml
    qml/SuggestionCell.qml
    qml/SuggestionStrip.qml
    qml/TextEditing.qml
    qml/Theme.qml
    qml/TypingController.qml
)
ecm_finalize_qml_module(KBoardUi DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardUi ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
