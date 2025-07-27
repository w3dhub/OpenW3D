add_executable(renrem RenRem.cpp)

target_link_libraries(renrem PRIVATE wwcommon scontrol wwdebug wwlib winmm)
