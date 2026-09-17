include(ECMAddTests)

foreach(test klipyapitest gifstoretest)
    ecm_add_test(${test}.cpp fixtures.h
        TEST_NAME gif-${test}
        LINK_LIBRARIES Qt::Test KBoardGif
    )
    target_compile_definitions(gif-${test} PRIVATE KBOARD_GIF_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
endforeach()
