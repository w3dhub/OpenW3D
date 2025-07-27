set(SCRIPTS_SRC
    always.h
    bool.h
    combatsound.h
    CustomEvents.h
    DLLmain.cpp
    DPrint.cpp
    DPrint.h
    DrMobius.cpp
    gameobjobserver.h
    scriptevents.h
    ScriptFactory.cpp
    ScriptFactory.h
    ScriptNode.h
    ScriptRegistrant.h
    ScriptRegistrar.cpp
    ScriptRegistrar.h
    slist.h
    strtrim.cpp
    strtrim.h
    vector.h
    vector3.h
    visualc.h
    wwmath.h
    Test_BMG.cpp
    Test_Cinematic.cpp
    Test_DAK.cpp
    Test_DAY.cpp
    Test_DLS.cpp
    Test_DME.cpp
    Test_GTH.cpp
    Test_JDG_EVA.cpp
    Test_PDS.cpp
    Test_RAD.cpp
    Test_RMV.cpp
    Test_RMV_Toolkit.cpp
    Toolkit.cpp
    Toolkit.h
    Toolkit_Actions.cpp
    Toolkit_Animations.cpp
    Toolkit_Broadcaster.cpp
    Toolkit_Explosions.cpp
    Toolkit_Objectives.cpp
    Toolkit_Objects.cpp
    Toolkit_Powerup.cpp
    Toolkit_Siege.cpp
    Toolkit_Sounds.cpp
    Toolkit_Spawners.cpp
    Toolkit_Triggers.cpp
    Mission00.cpp
    Mission00.h
    Mission01.cpp
    Mission02.cpp
    Mission03.cpp
    Mission04.cpp
    Mission05.cpp
    Mission06.cpp
    Mission07.cpp
    mission08.cpp
    Mission09.cpp
    Mission1.h
    Mission10.cpp
    Mission10.h
    Mission11.cpp
    Mission11.h
    Mission2.h
    Mission3.h
    Mission4.h
    Mission5.h
    Mission6.h
    Mission7.h
    Mission8.h
    Mission9.h
    MissionDemo.cpp
    MissionX0.cpp
    MissionX0.h
    actionparams.h
    scriptcommands.h
    scripts.cpp
    scripts.h
    string_ids.h
)

# Targets to build.
add_library(scripts SHARED)

target_compile_definitions(scripts PRIVATE BUILDING_DLL)

target_link_libraries(scripts PRIVATE
    wwcommon
)

set_target_properties(scripts PROPERTIES PREFIX "")

target_sources(scripts PRIVATE ${SCRIPTS_SRC})
