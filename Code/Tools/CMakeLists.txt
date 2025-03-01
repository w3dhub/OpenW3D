# Top Level CMake for building SDK tools.
add_subdirectory(MakeMix)
add_subdirectory(RenRem)

# On newer toolsets there are issues with MFC and DX8 conflicting.
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "12.0.8804")
    add_subdirectory(MixViewer)
    add_subdirectory(W3DView)
    add_subdirectory(LevelEdit)
    add_subdirectory(WWConfig)
endif()
