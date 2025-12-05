set(WWNET_SRC
  BWBalance.cpp
  BWBalance.h
  connect.cpp
  connect.h
  fromaddress.h
  msgstat.cpp
  msgstat.h
  msgstatlist.cpp
  msgstatlist.h
  msgstatlistgroup.cpp
  msgstatlistgroup.h
  netstats.cpp
  netstats.h
  netutil.cpp
  netutil.h
  networkobject.cpp
  networkobject.h
  networkobjectfactory.cpp
  networkobjectfactory.h
  networkobjectfactorymgr.cpp
  networkobjectfactorymgr.h
  networkobjectmgr.cpp
  networkobjectmgr.h
  packetmgr.cpp
  packetmgr.h
  packettype.h
  rhost.cpp
  rhost.h
  singlepl.cpp
  singlepl.h
  wwpacket.cpp
  wwpacket.h
  network-typedefs.h
  socket_wrapper.h
)

if (WIN32)
  list(APPEND WWNET_SRC socket_wrapper_win32.cpp)
else()
  list(APPEND WWNET_SRC socket_wrapper_posix.cpp)
endif()

add_library(wwnet STATIC)
target_sources(wwnet PRIVATE ${WWNET_SRC})

target_link_libraries(wwnet PRIVATE wwcommon)

if (WIN32)
  target_link_libraries(wwnet PRIVATE ws2_32)
  target_compile_definitions(wwnet PRIVATE _WIN32_WINNT=0x0601 WINVER=0x0601)
endif()
