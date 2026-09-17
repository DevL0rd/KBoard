pkg_check_modules(Hunspell REQUIRED IMPORTED_TARGET hunspell)

add_library(KBoardTyping SHARED
    autocorrectpolicy.cpp
    autocorrectpolicy.h
    commitanalysis.cpp
    commitanalysis.h
    corrector.cpp
    corrector.h
    glidedecoder.cpp
    glidedecoder.h
    glidegeometry.cpp
    glidegeometry.h
    glideservice.cpp
    glideservice.h
    inputpolicy.cpp
    inputpolicy.h
    keygeometry.cpp
    keygeometry.h
    languagedata.cpp
    languagedata.h
    languageloader.cpp
    languageloader.h
    lexicon.cpp
    lexicon.h
    localeresolver.cpp
    localeresolver.h
    predictor.cpp
    predictor.h
    suggestion.h
    suggestionslots.cpp
    suggestionslots.h
    textanalysis.cpp
    textanalysis.h
    textcasing.cpp
    textcharacters.cpp
    textexpansions.cpp
    textexpansions.h
    typingengine.cpp
    typingengine.h
    typingengineglide.cpp
    typingenginelearning.cpp
    typingenginequeries.cpp
    usermodel.cpp
    usermodel.h
    userstore.cpp
    userstore.h
    wordranker.cpp
    wordranker.h
    wordscorer.cpp
    wordscorer.h
)
target_compile_options(KBoardTyping PRIVATE -fvisibility=default)
target_include_directories(KBoardTyping PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(KBoardTyping
    PUBLIC Qt::Core Qt::Qml Qt::Quick KBoardConfig KBoardPlatform
    PRIVATE Qt::Concurrent PkgConfig::Hunspell
)
set_target_properties(KBoardTyping PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
ecm_add_qml_module(KBoardTyping URI "org.devl0rd.kboard.typing" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
ecm_finalize_qml_module(KBoardTyping DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardTyping ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
