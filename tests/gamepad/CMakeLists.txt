include(ECMAddTests)

ecm_add_tests(
    gamepadbuttonstest.cpp
    chordmatchertest.cpp
    navigationrepeattest.cpp
    steamdedupetest.cpp
    gamepadlogictest.cpp
    gamepadbackendtest.cpp
    descriptorscannertest.cpp
    pollsettest.cpp
    NAME_PREFIX kboard-
    LINK_LIBRARIES Qt::Test KBoardGamepad KBoardConfig
)
set_tests_properties(kboard-gamepadbuttonstest kboard-chordmatchertest kboard-navigationrepeattest kboard-steamdedupetest kboard-gamepadlogictest kboard-gamepadbackendtest kboard-descriptorscannertest kboard-pollsettest PROPERTIES LABELS unit)

add_executable(kboard-gamepad-probe gamepadprobe.cpp)
target_link_libraries(kboard-gamepad-probe PRIVATE Qt::Core Qt::Gui KBoardGamepad KBoardConfig)
