add_library(KBoardEmoji SHARED
    emojidata.cpp
    emojidata.h
    emojigridmodel.cpp
    emojigridmodel.h
    emojilistmodel.cpp
    emojilistmodel.h
    emojimatcher.cpp
    emojimatcher.h
    emojistore.cpp
    emojistore.h
    emojiuserstate.cpp
    emojiuserstate.h
    jsonfile.cpp
    jsonfile.h
)
target_compile_options(KBoardEmoji PRIVATE -fvisibility=default)
target_include_directories(KBoardEmoji PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(KBoardEmoji
    PUBLIC Qt::Core Qt::Gui Qt::Qml Qt::Quick
    PRIVATE KBoardConfig KBoardPlatform
)
set_target_properties(KBoardEmoji PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
ecm_add_qml_module(KBoardEmoji URI "org.devl0rd.kboard.emoji" GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick "org.devl0rd.kboard.config" "org.devl0rd.kboard.platform"
)
ecm_target_qml_sources(KBoardEmoji SOURCES
    qml/CellRow.qml
    qml/EmojiCell.qml
    qml/EmojiGrid.qml
    qml/EmojiPanel.qml
    qml/EmojiTabBar.qml
    qml/PanelButton.qml
    qml/PanelTopBar.qml
    qml/SearchPill.qml
    qml/SkinTonePopup.qml
)
ecm_finalize_qml_module(KBoardEmoji DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardEmoji ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
