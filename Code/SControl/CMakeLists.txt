set(SCONTROL_SRC 
    servercontrol.cpp
    servercontrol.h
    servercontrolsocket.cpp
    servercontrolsocket.h
)

# Targets to build.
add_library(scontrol STATIC)

target_link_libraries(scontrol PRIVATE
    wwcommon
)

target_sources(scontrol PRIVATE ${SCONTROL_SRC})
