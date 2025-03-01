# Set source files
set(WWAUDIO_SRC
    AudibleSound.cpp
    AudioEvents.cpp
    AudioSaveLoad.cpp
    FilteredSound.cpp
    Listener.cpp
    listenerhandle.cpp
    LogicalListener.cpp
    LogicalSound.cpp
    sound2dhandle.cpp
    Sound3D.cpp
    sound3dhandle.cpp
    SoundBuffer.cpp
    soundhandle.cpp
    SoundPseudo3D.cpp
    SoundScene.cpp
    SoundSceneObj.cpp
    soundstreamhandle.cpp
    Threads.cpp
    Utils.cpp
    WWAudio.cpp
    AABTreeSoundCullClass.h
    AudibleSound.h
    AudioEvents.h
    AudioSaveLoad.h
    FilteredSound.h
    Listener.h
    listenerhandle.h
    LogicalListener.h
    LogicalSound.h
    PriorityVector.h
    sound2dhandle.h
    Sound3D.h
    sound3dhandle.h
    SoundBuffer.h
    SoundCullObj.h
    soundhandle.h
    SoundPseudo3D.h
    SoundScene.h
    SoundSceneObj.h
    soundstreamhandle.h
    Threads.h
    Utils.h
    WWAudio.h
)

# Targets to build.
add_library(wwaudio STATIC)

target_link_libraries(wwaudio PRIVATE
    wwcommon
)

target_sources(wwaudio PRIVATE ${WWAUDIO_SRC})

# This module needs building differently for the level editor
if (RENEGADE_TOOLS)
    # Targets to build.
    add_library(wwaudioe STATIC)

    target_link_libraries(wwaudioe PRIVATE
        wwcommon
    )

    target_compile_definitions(wwaudioe PRIVATE PARAM_EDITING_ON)

    target_sources(wwaudioe PRIVATE ${WWAUDIO_SRC})
endif()