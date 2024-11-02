cmake_minimum_required(VERSION 3.5)
project(ws_player C CXX)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11  -Wno-deprecated-declarations")

# 添加包含目录
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_linux/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

# 添加链接库路径
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_linux/lib)

# 添加源文件
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 添加可执行文件
add_executable(ws-player ${SOURCE_FILES})

# 配置emscripten
# -s USE_PTHREADS=1：启用对 pthreads 的支持，允许在 Web Worker 中使用多线程，首次拉工程编译需将该行注释或者值设置为0。
# -s PTHREAD_POOL_SIZE=8：指定 pthread 线程池的大小为 8。
# -s EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']：导出 Emscripten 运行时的方法，包括 ccall 和 cwrap。
# -s ALLOW_MEMORY_GROWTH=1：允许内存动态增长。
# -s EXPORTED_FUNCTIONS=['_free']：导出名为 _free 的函数。
# -s MODULARIZE=1：生成模块化的输出，允许在 JavaScript 中进行导入和使用。
# -s EXPORT_ES6=1：以 ES6 模块的形式导出输出。
# -s USE_ES6_IMPORT_META=0：禁用使用 ES6 import.meta 的特性。
# -s ENVIRONMENT='web,worker'：指定编译后的模块在 Web Worker 环境中运行。
# -s TOTAL_STACK=5242880：指定总栈大小为 5MB。
# -s TOTAL_MEMORY=134217728: 指定分配的堆大小为128MB
# -s INITIAL_MEMORY=33554432: 指定初始内存为32MB
# -s USE_SDL=2：启用对 SDL（Simple DirectMedia Layer）的支持。
# -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0：启用对已弃用的 find event target 行为的支持。
# -s USE_WEBGPU=1: 启用对WebGPU的支持。
set(EMSCRIPTEN_FLAGS
    "-s USE_PTHREADS=1 \
    -s PTHREAD_POOL_SIZE=4 \
    -s \"EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']\" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s \"EXPORTED_FUNCTIONS=['_malloc', '_free']\" \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s USE_ES6_IMPORT_META=0 \
    -s ENVIRONMENT='web,worker' \
    -s TOTAL_STACK=5242880 \
    -s TOTAL_MEMORY=134217728 \
    -s INITIAL_MEMORY=33554432 \
    -s USE_SDL=2 \
    -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 \
    -s USE_WEBGPU=1"
)

# 将 EMSCRIPTEN_FLAGS 传递给目标属性
set_target_properties(ws-player PROPERTIES
    COMPILE_FLAGS "${EMSCRIPTEN_FLAGS}"
    LINK_FLAGS "${EMSCRIPTEN_FLAGS}"
)

# 链接库
target_link_libraries(ws-player avformat avcodec avutil swscale swresample)