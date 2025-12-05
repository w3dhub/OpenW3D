# Set source files
set(WWUI_SRC
    buttonctrl.cpp
    checkboxctrl.cpp
    childdialog.cpp
    comboboxctrl.cpp
    dialogbase.cpp
    dialogcontrol.cpp
    dialogmgr.cpp
    dialogtext.cpp
    dialogtransition.cpp
    dropdownctrl.cpp
    editctrl.cpp
    healthbarctrl.cpp
    imagectrl.cpp
    IMECandidate.cpp
    IMECandidateCtrl.cpp
    IMEManager.cpp
    inputctrl.cpp
    listctrl.cpp
    listiconmgr.cpp
    mapctrl.cpp
    menubackdrop.cpp
    menudialog.cpp
    menuentryctrl.cpp
    merchandisectrl.cpp
    mousemgr.cpp
    multilinetextctrl.cpp
    popupdialog.cpp
    ProgressCtrl.cpp
    screencursor.cpp
    scrollbarctrl.cpp
    shortcutbarctrl.cpp
    sliderctrl.cpp
    stylemgr.cpp
    tabctrl.cpp
    textmarqueectrl.cpp
    tooltip.cpp
    tooltipmgr.cpp
    treectrl.cpp
    viewerctrl.cpp
    wwuiinput.cpp
    buttonctrl.h
    checkboxctrl.h
    childdialog.h
    comboboxctrl.h
    controladvisesink.h
    dialogbase.h
    dialogcontrol.h
    dialogfactory.h
    dialogmgr.h
    dialogspec.h
    dialogtext.h
    dialogtransition.h
    dropdownctrl.h
    editctrl.h
    healthbarctrl.h
    imagectrl.h
    IMECandidate.h
    IMECandidate.h
    IMECandidateCtrl.h
    IMEManager.h
    IMEManager.h
    inputctrl.h
    listctrl.h
    listiconmgr.h
    mapctrl.h
    menubackdrop.h
    menudialog.h
    menuentryctrl.h
    merchandisectrl.h
    mousemgr.h
    multilinetextctrl.h
    popupdialog.h
    ProgressCtrl.h
    screencursor.h
    scrollbarctrl.h
    shortcutbarctrl.h
    sliderctrl.h
    stylemgr.h
    tabctrl.h
    textmarqueectrl.h
    tooltip.h
    tooltipmgr.h
    treectrl.h
    viewerctrl.h
    wwuiinput.h
)

# Targets to build.
add_library(wwui STATIC)

# This is needed for this module but breaks the build across the whole project?
target_compile_definitions(wwui PRIVATE
    _MBCS
)

target_link_libraries(wwui PRIVATE
    wwcommon
    imm32
)

target_sources(wwui PRIVATE ${WWUI_SRC})
