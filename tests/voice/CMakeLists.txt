include(ECMAddTests)

set(voice_test_definitions KBOARD_VOICE_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data")

foreach(voice_test modelcatalogtest transcripttexttest speechdetectortest transcriptiontest)
    ecm_add_test(${voice_test}.cpp voicetestutils.h
        TEST_NAME kboard-voice-${voice_test}
        LINK_LIBRARIES Qt::Test Qt::Core KBoardVoice KBoardConfig
    )
    target_compile_definitions(kboard-voice-${voice_test} PRIVATE ${voice_test_definitions})
endforeach()

set_tests_properties(kboard-voice-transcriptiontest PROPERTIES TIMEOUT 300)
