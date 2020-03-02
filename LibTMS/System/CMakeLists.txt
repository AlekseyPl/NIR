cmake_minimum_required(VERSION 3.10)
project(system)

set(SOURCES 
./DebugInfo.cpp
)

set(HEADERS
./DebugInfo.h
)


add_library(system STATIC ${SOURCES} ${HEADERS})

target_include_directories(system
	PUBLIC ../)
