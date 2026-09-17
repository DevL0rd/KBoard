include(ECMAddTests)

add_library(kboard-typing-testsupport STATIC typingtestsupport.cpp typingtestsupport.h)
target_link_libraries(kboard-typing-testsupport PUBLIC Qt::Test KBoardTyping KBoardConfig)

foreach(name correctiontest predictiontest contexttest glidetest performancetest)
    ecm_add_test(${name}.cpp
        TEST_NAME kboard-typing-${name}
        LINK_LIBRARIES kboard-typing-testsupport KBoardTyping KBoardConfig KBoardPlatform Qt::Test
    )
endforeach()
