set(KBOARD_KLIPY_API_KEY "" CACHE STRING "KLIPY production API key compiled into the GIF panel")

add_library(KBoardGif SHARED
    gifdiscovery.cpp
    gifdiscovery.h
    giffeed.cpp
    giffeed.h
    gifinserter.cpp
    gifinserter.h
    giflibrary.cpp
    giflibrary.h
    gifmodel.cpp
    gifmodel.h
    gifpaste.cpp
    gifpaste.h
    gifstore.cpp
    gifstore.h
    gifstorefixtures.cpp
    klipyapi.h
    klipyclient.cpp
    klipyclient.h
    klipyjson.cpp
    klipyurls.cpp
)
target_compile_options(KBoardGif PRIVATE -fvisibility=default)
target_include_directories(KBoardGif PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties(gifstore.cpp PROPERTIES COMPILE_DEFINITIONS "KBOARD_KLIPY_API_KEY=\"${KBOARD_KLIPY_API_KEY}\"")
target_link_libraries(KBoardGif
    PUBLIC Qt::Core Qt::Gui Qt::Qml Qt::Quick Qt::Network
    PRIVATE KBoardConfig KBoardPlatform KF6::GuiAddons
)
set_target_properties(KBoardGif PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)

ecm_add_qml_module(KBoardGif URI "org.devl0rd.kboard.gif" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick org.devl0rd.kboard.config org.devl0rd.kboard.platform)
ecm_target_qml_sources(KBoardGif SOURCES
    qml/GifChip.qml
    qml/GifChipsRow.qml
    qml/GifGrid.qml
    qml/GifMediaToggle.qml
    qml/GifPanel.qml
    qml/GifRoundButton.qml
    qml/GifSearchPill.qml
    qml/GifSkeleton.qml
    qml/GifStatus.qml
    qml/GifTile.qml
    qml/GifToast.qml
)
ecm_finalize_qml_module(KBoardGif DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardGif ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
