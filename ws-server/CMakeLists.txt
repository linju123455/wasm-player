cmake_minimum_required(VERSION 3.5)
project(ws-server C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -w -pthread -lrt --std=c++11")

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ffmpeg_linux)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/ffmpeg_linux)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_libraries(${OPENSSL_LIBRARIES})
endif()

find_package(X11 REQUIRED)
if (X11_FOUND)
    include_directories(${X11_INCLUDE_DIR})
    link_libraries(${X11_LIBRARIES})
endif()

# 添加源文件
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 添加可执行文件
add_executable(ws-server ${SOURCE_FILES})

target_link_libraries(ws-server avformat avcodec avutil ssl crypto ${X11_LIBRARIES})