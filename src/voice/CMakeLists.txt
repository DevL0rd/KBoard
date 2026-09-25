add_library(KBoardVoice SHARED
    audiocapture.cpp
    audiocapture.h
    engineclient.cpp
    engineclient.h
    modelcatalog.cpp
    modelcatalog.h
    modeldownloader.cpp
    modeldownloader.h
    silerovad.cpp
    silerovad.h
    speechdetector.cpp
    speechdetector.h
    speechengine.cpp
    speechengine.h
    speechsegmenter.cpp
    speechsegmenter.h
    transcripttext.cpp
    transcripttext.h
    transcriptwriter.cpp
    transcriptwriter.h
    voicebackends.cpp
    voicebackends.h
    voicelogging.cpp
    voicelogging.h
    voicemodels.cpp
    voicemodels.h
    voicestates.h
    voicetyping.cpp
    voicetyping.h
    voicetypingmodels.cpp
    voicetypingproperties.cpp
)
target_include_directories(KBoardVoice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(KBoardVoice PRIVATE -fvisibility=default)
target_link_libraries(KBoardVoice
    PUBLIC Qt::Core Qt::Quick
    PRIVATE Qt::Network Qt::Multimedia KBoardConfig KBoardPlatform
)
if(KBOARD_BUNDLED_WHISPER)
    target_link_libraries(KBoardVoice PRIVATE whisper parakeet ggml)
    target_link_options(KBoardVoice PRIVATE "LINKER:--exclude-libs,ALL")
else()
    pkg_check_modules(Whisper REQUIRED IMPORTED_TARGET "whisper >= ${KBOARD_SYSTEM_WHISPER_MIN_VERSION}")
    pkg_check_modules(Parakeet REQUIRED IMPORTED_TARGET "parakeet >= ${KBOARD_SYSTEM_WHISPER_MIN_VERSION}")
    target_link_libraries(KBoardVoice PRIVATE PkgConfig::Whisper PkgConfig::Parakeet)
    target_compile_definitions(KBoardVoice PRIVATE KBOARD_LOAD_GGML_BACKENDS)
endif()
set_target_properties(KBoardVoice PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)

ecm_add_qml_module(KBoardVoice URI "org.devl0rd.kboard.voice" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
set_source_files_properties(qml/VoiceStyle.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
ecm_target_qml_sources(KBoardVoice SOURCES
    qml/VoiceButton.qml
    qml/VoiceChip.qml
    qml/VoiceMessage.qml
    qml/VoiceOrb.qml
    qml/VoicePanel.qml
    qml/VoicePanelView.qml
    qml/VoiceStatusLine.qml
    qml/VoiceStyle.qml
    qml/VoiceTranscript.qml
    qml/VoiceWaveform.qml
)
ecm_finalize_qml_module(KBoardVoice DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardVoice ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

add_executable(kboard-voice-model modeltool.cpp)
target_link_libraries(kboard-voice-model PRIVATE Qt::Core Qt::Network KBoardVoice)
install(TARGETS kboard-voice-model ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
