cmake_minimum_required(VERSION 3.15)

project(shironeko)
set(CMAKE_CXX_STANDARD 17)

find_library(AVCODEC_LIBRARY avcodec HINTS lib)
find_library(AVFORMAT_LIBRARY avformat HINTS lib)
find_library(AVUTIL_LIBRARY avutil HINTS lib)
find_library(SWSCALE_LIBRARY swscale HINTS lib)

set(FFMPEG_LIBRARY ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})

set(COMMON 
    src/util/common.h 
    src/util/common.cpp
    src/util/frame_queue.cpp)

set(DEMUXING
    src/demuxing/demuxing.cpp
    src/demuxing/demuxing.h
    src/demuxing/input.cpp)

set(REMUXING
    src/remuxing/output.cpp
    src/remuxing/output.h
    src/remuxing/remuxing.cpp
    src/remuxing/remuxing.h
    src/remuxing/vp9.cpp)

add_executable(remuxing_test ${COMMON} ${DEMUXING} ${REMUXING} src/remuxing_test.cpp)
target_link_libraries(remuxing_test PRIVATE ${FFMPEG_LIBRARY})
target_include_directories(remuxing_test PRIVATE include)