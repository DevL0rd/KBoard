include(ECMAddTests)

ecm_add_test(soundtest.cpp
    TEST_NAME kboard-soundtest
    LINK_LIBRARIES Qt::Test Qt::Qml KBoardSound KBoardConfig
)

add_executable(kboard-sound-probe soundprobe.cpp)
target_link_libraries(kboard-sound-probe PRIVATE Qt::Core Qt::Gui KBoardSound KBoardConfig)
