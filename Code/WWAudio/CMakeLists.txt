# Set source files
set(WWAUDIO_SRC
    AudibleSound.cpp
    AudioEvents.cpp
    AudioSaveLoad.cpp
    FilteredSound.cpp
    Listener.cpp
    LogicalListener.cpp
    LogicalSound.cpp
    Sound3D.cpp
    soundhandle.cpp
    SoundPseudo3D.cpp
    SoundScene.cpp
    SoundSceneObj.cpp
    Threads.cpp
    WWAudio.cpp
    AABTreeSoundCullClass.h
    AudibleSound.h
    AudioEvents.h
    AudioSaveLoad.h
    FilteredSound.h
    Listener.h
    LogicalListener.h
    LogicalSound.h
    PriorityVector.h
    Sound3D.h
    SoundBuffer.h
    SoundCullObj.h
    soundhandle.h
    SoundPseudo3D.h
    SoundScene.h
    SoundSceneObj.h
    Threads.h
    Utils.h
    WWAudio.h
)

if(W3D_BUILD_OPTION_MILES)
    list(APPEND WWAUDIO_SRC
        miles/MilesAudio.h
        miles/MilesAudio.cpp
        miles/MilesSoundBuffer.cpp
        miles/MilesSoundBuffer.h
        miles/MilesUtils.cpp
        miles/sound2dhandle.cpp
        miles/sound2dhandle.h
        miles/sound3dhandle.cpp
        miles/sound3dhandle.h
        miles/soundstreamhandle.cpp
        miles/soundstreamhandle.h
    )
else()
    list(APPEND WWAUDIO_SRC
        null/NullAudio.h
        null/NullAudio.cpp
        null/NullUtils.cpp
    )
endif()

# Targets to build.
add_library(wwaudio STATIC)

target_link_libraries(wwaudio PRIVATE
    wwcommon
)

target_sources(wwaudio PRIVATE ${WWAUDIO_SRC})

if(TARGET milesstub)
    target_link_libraries(wwaudio PRIVATE milesstub)
endif()

# This module needs building differently for the level editor
if (W3D_TOOLS)
    # Targets to build.
    add_library(wwaudioe STATIC)

    target_link_libraries(wwaudioe PRIVATE
        wwcommon
    )

    target_compile_definitions(wwaudioe PRIVATE PARAM_EDITING_ON)

    target_sources(wwaudioe PRIVATE ${WWAUDIO_SRC})

    if(W3D_BUILD_OPTION_MILES)
        target_compile_definitions(wwaudioe PRIVATE W3D_HAS_MILES)
    endif()

    if(TARGET milesstub)
        target_link_libraries(wwaudioe PRIVATE milesstub)
    endif()
endif()
