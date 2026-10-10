cmake_minimum_required(VERSION 3.16)
project(SmartLibrary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
    AdminDashboard.cpp
    AdminDashboard.h
    AdminPages.cpp
    AdminPages.h
    Alert.cpp
    Alert.h
    AuthWindow.cpp
    AuthWindow.h
    ILibraryBackend.h
    InMemoryBackend.cpp
    InMemoryBackend.h
    PostgresBackend.cpp
    PostgresBackend.h
    MemberDashboard.cpp
    MemberDashboard.h
    Style.h
    Types.h
)

qt_add_executable(SmartLibrary ${PROJECT_SOURCES}
    postgresbackend.h postgresbackend.cpp
    memberdashboard.h memberdashboard.cpp)
target_link_libraries(SmartLibrary PRIVATE Qt6::Widgets Qt6::Sql)