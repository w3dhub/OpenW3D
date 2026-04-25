# Set source files
set(WW3D2_SRC
    aabtree.cpp
    aabtreebuilder.cpp
    agg_def.cpp
    animatedsoundmgr.cpp
    animobj.cpp
    assetmgr.cpp
    assetstatus.cpp
    bitmaphandler.cpp
    bmp2d.cpp
    boxrobj.cpp
    bwrender.cpp
    camera.cpp
    collect.cpp
    coltest.cpp
    composite.cpp
    dazzle.cpp
    ddsfile.cpp
    decalmsh.cpp
    decalsys.cpp
    distlod.cpp
    dx8caps.cpp
    dx8fvf.cpp
    dx8indexbuffer.cpp
    dx8polygonrenderer.cpp
    dx8renderer.cpp
    dx8rendererdebugger.cpp
    dx8texman.cpp
    dx8vertexbuffer.cpp
    dx8wrapper.cpp
    dynamesh.cpp
    font3d.cpp
    formconv.cpp
    framgrab.cpp
    hanim.cpp
    hanimmgr.cpp
    hcanim.cpp
    hlod.cpp
    hmdldef.cpp
    hmorphanim.cpp
    hrawanim.cpp
    htree.cpp
    htreemgr.cpp
    intersec.cpp
    layer.cpp
    light.cpp
    lightenvironment.cpp
    line3d.cpp
    linegrp.cpp
    mapper.cpp
    matinfo.cpp
    matpass.cpp
    matrixmapper.cpp
    mesh.cpp
    meshbuild.cpp
    meshdam.cpp
    meshgeometry.cpp
    meshmatdesc.cpp
    meshmdl.cpp
    meshmdlio.cpp
    metalmap.cpp
    missingtexture.cpp
    motchan.cpp
    nullrobj.cpp
    part_buf.cpp
    part_emt.cpp
    part_ldr.cpp
    pivot.cpp
    pointgr.cpp
    polyinfo.cpp
    predlod.cpp
    prim_anim.cpp
    projector.cpp
    proto.cpp
    render2d.cpp
    render2dsentence.cpp
    renderobjectrecycler.cpp
    rendobj.cpp
    rinfo.cpp
    ringobj.cpp
    scene.cpp
    segline.cpp
    seglinerenderer.cpp
    shader.cpp
    shattersystem.cpp
    snappts.cpp
    sortingrenderer.cpp
    soundrobj.cpp
    sphereobj.cpp
    statistics.cpp
    stripoptimizer.cpp
    surfaceclass.cpp
    texfcach.cpp
    texproject.cpp
    texture.cpp
    textureloader.cpp
    texturethumbnail.cpp
    vertmaterial.cpp
    visrasterizer.cpp
    w3d_dep.cpp
    w3d_util.cpp
    ww3d.cpp
    ww3dformat.cpp
    aabtree.h
    aabtreebuilder.h
    agg_def.h
    animatedsoundmgr.h
    animobj.h
    assetmgr.h
    assetstatus.h
    bitmaphandler.h
    bmp2d.h
    boxrobj.h
    bwrender.h
    camera.h
    collect.h
    colorspace.h
    coltest.h
    coltype.h
    composite.h
    dazzle.h
    ddsfile.h
    decalmsh.h
    decalsys.h
    distlod.h
    dllist.h
    dx8caps.h
    dx8fvf.h
    dx8indexbuffer.h
    dx8list.h
    dx8polygonrenderer.h
    dx8renderer.h
    dx8rendererdebugger.h
    dx8texman.h
    dx8vertexbuffer.h
    dx8wrapper.h
    dynamesh.h
    font3d.h
    formconv.h
    framgrab.h
    hanim.h
    hanimmgr.h
    hcanim.h
    hlod.h
    hmdldef.h
    hmorphanim.h
    hrawanim.h
    htree.h
    htreemgr.h
    intersec.h
    intersec.inl
    inttest.h
    layer.h
    light.h
    lightenvironment.h
    line3d.h
    linegrp.h
    mapper.h
    matinfo.h
    matpass.h
    matrixmapper.h
    mesh.h
    meshbuild.h
    meshdam.h
    meshgeometry.h
    meshmatdesc.h
    meshmdl.h
    metalmap.h
    missingtexture.h
    motchan.h
    nullrobj.h
    part_buf.h
    part_emt.h
    part_ldr.h
    pivot.h
    pointgr.h
    polyinfo.h
    predlod.h
    prim_anim.h
    projector.h
    proto.h
    proxy.h
    rddesc.h
    render2d.h
    render2dsentence.h
    renderobjectrecycler.h
    rendobj.h
    rinfo.h
    ringobj.h
    robjlist.h
    scene.h
    segline.h
    seglinerenderer.h
    shader.h
    shattersystem.h
    snappts.h
    sortingrenderer.h
    soundrobj.h
    sphereobj.h
    statistics.h
    stripoptimizer.h
    surfaceclass.h
    texfcach.h
    texproject.h
    texture.h
    textureloader.h
    texturethumbnail.h
    vertmaterial.h
    visrasterizer.h
    w3d_dep.h
    w3d_file.h
    w3d_obsolete.h
    w3d_util.h
    w3derr.h
    ww3d.h
    ww3dformat.h
    ww3dids.h
    ww3dtrig.h
)

# Targets to build.
add_library(ww3d2 STATIC)

target_link_libraries(ww3d2 PRIVATE
    wwcommon
    wwlib
    vfw32
    winmm
)

target_sources(ww3d2 PRIVATE ${WW3D2_SRC})

# Additional backend options (DX8 is always available on Windows via ENABLE_DX9_BACKEND)
# Vulkan: cross-platform, requires Vulkan SDK headers
option(ENABLE_VULKAN_BACKEND "Enable Vulkan backend" OFF)
add_feature_info(Vulkan_Backend ENABLE_VULKAN_BACKEND "Vulkan render backend (cross-platform)")

# DX12: Windows-only, requires Windows 10 SDK
option(ENABLE_DX12_BACKEND "Enable DirectX 12 backend" OFF)
add_feature_info(DX12_Backend ENABLE_DX12_BACKEND "DirectX 12 render backend (Windows only)")

# Vulkan backend sources
if (ENABLE_VULKAN_BACKEND)
    if (NOT WIN32)
        find_package(Vulkan QUIET)
        if (NOT Vulkan_FOUND)
            message(STATUS "Vulkan not found; Vulkan backend will not be built")
        endif()
    else()
        # On Windows, Vulkan is typically available via the GPU driver
        find_package(Vulkan QUIET)
    endif()
    if (Vulkan_FOUND)
        target_sources(ww3d2 PRIVATE
            backends/vulkan/vulkanbackend.cpp
        )
        target_include_directories(ww3d2 PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/backends/vulkan
        )
        target_link_libraries(ww3d2 PRIVATE Vulkan::Vulkan)
    else()
        message(STATUS "Vulkan backend enabled but Vulkan not available — backend stub will be excluded")
        # Still update cache so the option appears "on" even if unmet
    endif()
endif()

# DX12 backend sources (Windows only)
if (ENABLE_DX12_BACKEND)
    if (WIN32)
        target_sources(ww3d2 PRIVATE
            backends/dx12/dx12backend.cpp
        )
        target_include_directories(ww3d2 PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/backends/dx12
        )
    else()
        message(WARNING "ENABLE_DX12_BACKEND is Windows-only; ignoring on ${CMAKE_SYSTEM_NAME}")
    endif()
endif()

# This module needs building differently for the level editor
if (W3D_TOOLS)
    # Targets to build.
    add_library(ww3d2e STATIC)

    target_link_libraries(ww3d2e PRIVATE
        wwcommon
        vfw32
        winmm
    )

    target_compile_definitions(ww3d2e PRIVATE PARAM_EDITING_ON)

    target_sources(ww3d2e PRIVATE ${WW3D2_SRC})
endif()
