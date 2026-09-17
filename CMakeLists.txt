cmake_minimum_required(VERSION 3.25)

project(kboard VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "6.11.0")
set(KF_MIN_VERSION "6.26.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQmlModule)
include(ECMSetupVersion)
include(FeatureSummary)

set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 DBus Network Multimedia WaylandClient Test Concurrent)
find_package(Qt6GuiPrivate ${QT_MIN_VERSION} REQUIRED NO_MODULE)
find_package(Qt6WaylandClientPrivate ${QT_MIN_VERSION} REQUIRED NO_MODULE)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Config CoreAddons I18n GuiAddons WindowSystem DBusAddons Kirigami Crash)
find_package(Wayland 1.2 REQUIRED COMPONENTS Client)
find_package(QtWaylandScanner REQUIRED ${QT_MIN_VERSION})
find_package(WaylandProtocols 1.19 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XKBCommon REQUIRED IMPORTED_TARGET xkbcommon)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KBOARD VERSION_HEADER ${CMAKE_BINARY_DIR}/kboard_version.h)

add_compile_definitions(
    -DKBOARD_QML_BUILD_DIR="${CMAKE_BINARY_DIR}/qml"
    -DKBOARD_QML_INSTALL_DIR="${KDE_INSTALL_FULL_QMLDIR}"
    -DKBOARD_DATA_BUILD_DIR="${CMAKE_SOURCE_DIR}/data"
    -DKBOARD_DATA_INSTALL_DIR="${KDE_INSTALL_FULL_DATADIR}/kboard"
)

option(KBOARD_WERROR "Treat warnings as errors" ON)
if(KBOARD_WERROR)
    add_compile_options(-Werror)
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

install(DIRECTORY data/ DESTINATION ${KDE_INSTALL_DATADIR}/kboard)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
