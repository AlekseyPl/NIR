cmake_minimum_required(VERSION 3.10)

project(Lte)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "TI6600")
	add_definitions(-DCHIP_6614)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "TI64x+")
	add_definitions(-DCHIP_6455)
endif()


add_subdirectory(Core)

add_library(Lte INTERFACE)
target_link_libraries(Lte INTERFACE Lte_Core)

