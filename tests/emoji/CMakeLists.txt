include(ECMAddTests)

ecm_add_test(emojitest.cpp
    TEST_NAME kboard-emojitest
    LINK_LIBRARIES Qt::Test Qt::Gui KBoardEmoji KBoardConfig
)
set_tests_properties(kboard-emojitest PROPERTIES ENVIRONMENT "KBOARD_USE_BUILD_TREE=1;QT_QPA_PLATFORM=offscreen")
