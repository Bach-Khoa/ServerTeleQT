cmake_minimum_required(VERSION 3.16)
project(GDTTelemetryTLDK35 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql Charts)

set(SOURCES
    main.cpp
    core/PacketParser.cpp
    models/AppConfig.cpp
    network/TelemetryServer.cpp
    network/MulticastReceiver.cpp
    storage/DatabaseManager.cpp
    storage/CsvLogger.cpp
    ui/MainWindow.cpp
    ui/Panel5A42.cpp
    ui/Panel5U44.cpp
    ui/Panel5E15.cpp
    ui/Panel5I41.cpp
    ui/PanelTelemetry.cpp
    ui/ChartPanel.cpp
    ui/ErrorRatePanel.cpp
    ui/Detail5A42Dialog.cpp
    ui/SetupServerDialog.cpp
    ui/Setup5A42Dialog.cpp
    ui/LedIndicator.cpp
)

set(HEADERS
    core/Constants.h
    core/PacketParser.h
    models/TelemetryData.h
    models/AppConfig.h
    network/TelemetryServer.h
    network/MulticastReceiver.h
    storage/DatabaseManager.h
    storage/CsvLogger.h
    ui/MainWindow.h
    ui/Panel5A42.h
    ui/Panel5U44.h
    ui/Panel5E15.h
    ui/Panel5I41.h
    ui/PanelTelemetry.h
    ui/ChartPanel.h
    ui/ErrorRatePanel.h
    ui/Detail5A42Dialog.h
    ui/SetupServerDialog.h
    ui/Setup5A42Dialog.h
    ui/LedIndicator.h
)

add_executable(GDTTelemetryTLDK35
    ${SOURCES}
    ${HEADERS}
    resources/app.qrc
)

target_link_libraries(GDTTelemetryTLDK35 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Charts
)

target_include_directories(GDTTelemetryTLDK35 PRIVATE ${CMAKE_SOURCE_DIR})

if(WIN32)
    set_target_properties(GDTTelemetryTLDK35 PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
