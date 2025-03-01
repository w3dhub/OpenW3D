add_executable(makemix MakeMix.cpp)

target_link_libraries(makemix PRIVATE wwcommon wwdebug wwlib winmm)