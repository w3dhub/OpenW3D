set(WWBITPACK_SRC
    BitPacker.cpp
    BitPacker.h
    bitpackids.h
    bitstream.cpp
    bitstream.h
    encoderlist.cpp
    encoderlist.h
    encodertypeentry.cpp
    encodertypeentry.h
)

# Targets to build.
add_library(wwbitpack STATIC)

target_link_libraries(wwbitpack PRIVATE
    wwcommon
)

target_sources(wwbitpack PRIVATE ${WWBITPACK_SRC})
