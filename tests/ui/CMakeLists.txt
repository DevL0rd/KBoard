include(ECMAddTests)

foreach(test testlayouts testkeylayout testkeygestures)
    ecm_add_test(${test}.cpp
        TEST_NAME ${test}
        LINK_LIBRARIES Qt::Test Qt::Quick KBoardUi KBoardConfig
    )
    target_compile_definitions(${test} PRIVATE KBOARD_LAYOUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../data/layouts")
endforeach()
