cmake_minimum_required(VERSION 3.8)
project(PeIterator LANGUAGES CXX)

option(_BUILD_EXAMPLE "Build example app" OFF)

add_library(${PROJECT_NAME} INTERFACE
        Include/PeIterator/PeTypes.h
        Include/PeIterator/PeImage.h
        Include/PeIterator/PeSection.h
        Include/PeIterator/PeRelocation.h
        Include/PeIterator/PeHeader.h
        Include/PeIterator/PeImport.h
        Include/PeIterator/PeExport.h
        Include/PeIterator/PeException.h
        Include/PeIterator/PeTls.h)
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/Include/PeIterator)

if (${_BUILD_EXAMPLE})
    add_subdirectory(Example)
endif ()