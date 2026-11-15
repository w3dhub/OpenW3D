cmake_minimum_required(VERSION 3.25)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(OpenW3DServerConsoleTests LANGUAGES CXX)
    include(CTest)
    add_subdirectory(../../../Code/Commando/console server_console)
    if(BUILD_TESTING)
        include(../../cmake/TestHelpers.cmake)
        add_custom_target(w3d_tests)
    endif()
endif()

if(NOT BUILD_TESTING)
    return()
endif()

w3d_add_test(server_console_tests
    SOURCES ConsoleTests.cpp
    LIBRARIES server_console)

find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_executable(server_console_harness ConsoleHarness.cpp)
    target_link_libraries(server_console_harness PRIVATE server_console)
    add_dependencies(w3d_tests server_console_harness)
    set(windows_test)
    if(WIN32)
        add_executable(server_console_windows_tests WindowsConsoleTests.cpp)
        target_compile_definitions(server_console_windows_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
        target_link_libraries(server_console_windows_tests PRIVATE server_console)
        add_dependencies(w3d_tests server_console_windows_tests)
        set(windows_test $<TARGET_FILE:server_console_windows_tests>)
    endif()
    add_test(NAME server_console_subprocess_tests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/subprocess_tests.py
                $<TARGET_FILE:server_console_harness> ${windows_test})
    set_tests_properties(server_console_subprocess_tests PROPERTIES TIMEOUT 45)
else()
    message(STATUS "Python3 interpreter not found; skipping server console subprocess tests")
endif()
