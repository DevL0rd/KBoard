add_library(KBoardClipboard SHARED
    clipboarddetect.cpp
    clipboarddetect.h
    clipboardentry.cpp
    clipboardentry.h
    clipboardexpiry.cpp
    clipboardguard.cpp
    clipboardguard.h
    clipboardexpiry.h
    clipboardhistory.cpp
    clipboardhistory.h
    clipboardhistoryfile.cpp
    clipboardhistoryinstance.cpp
    clipboardhistoryfile.h
    clipboardimageprovider.cpp
    clipboardimageprovider.h
    clipboardimagestore.cpp
    clipboardimagestore.h
    clipboardlatest.cpp
    clipboardpaster.cpp
    clipboardpaster.h
    clipboardlatest.h
    clipboardroles.cpp
    clipboardroles.h
    klipperbridge.cpp
    klipperbridge.h
    systemclipboard.cpp
    systemclipboard.h
)
target_compile_options(KBoardClipboard PRIVATE -fvisibility=default)
target_compile_definitions(KBoardClipboard PRIVATE TRANSLATION_DOMAIN="kboard")
target_include_directories(KBoardClipboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(KBoardClipboard
    PUBLIC Qt::Quick KBoardConfig
    PRIVATE Qt::DBus Qt::Concurrent KF6::GuiAddons KF6::I18n KBoardPlatform
)
set_target_properties(KBoardClipboard PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)

ecm_add_qml_module(KBoardClipboard URI "org.devl0rd.kboard.clipboard" GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)
set_source_files_properties(qml/ClipboardStyle.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
ecm_target_qml_sources(KBoardClipboard SOURCES
    qml/CardImageContent.qml
    qml/CardLinkContent.qml
    qml/CardOtpContent.qml
    qml/CardTextContent.qml
    qml/ClearButton.qml
    qml/ClipboardCard.qml
    qml/ClipboardEmptyState.qml
    qml/ClipboardHeader.qml
    qml/ClipboardPanel.qml
    qml/ClipboardStyle.qml
    qml/FilterChip.qml
    qml/HeaderButton.qml
    qml/MasonryGrid.qml
    qml/PausedBanner.qml
    qml/PinBadge.qml
)
ecm_finalize_qml_module(KBoardClipboard DESTINATION ${KDE_INSTALL_QMLDIR})
install(TARGETS KBoardClipboard ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
