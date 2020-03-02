cmake_minimum_required(VERSION 3.10)

project(LibTMS)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "TI6600")
	add_definitions(-DCHIP_6614)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "TI64x+")
	add_definitions(-DCHIP_6455)
endif()



add_definitions(-DTMS320C66X -DLITTLE_ENDIAN_HOST -DLITTLE_ENDIAN -D_LITTLE_ENDIAN -DNOASSUME)


add_subdirectory(Math)
add_subdirectory(System)

add_library(LibTMS INTERFACE)

target_link_libraries(LibTMS INTERFACE math system)
