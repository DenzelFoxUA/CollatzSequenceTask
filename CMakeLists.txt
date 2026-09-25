cmake_minimum_required(VERSION 3.21)

project(CollatzSequenceTask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(CollatzSequenceTask
    main.cpp
	UserInterface.cpp
    UserInterface.h
    CollatzSequenceProcessor.h
    CollatzSequenceProcessor.cpp
)

target_link_libraries(CollatzSequenceTask
    PRIVATE
    Qt6::Widgets
)