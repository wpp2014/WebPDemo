# cmake 最低版本需求
cmake_minimum_required(VERSION 3.10)

# 工程名称
project(webp_demo)

if (UNIX AND NOT APPLE)
  SET(LINUX TRUE)
endif()

# 判断有没有指定编译模式
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("-- Debug mode")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message("-- Release mode")
else()
  message(FATAL_ERROR "Please specify Debug Or Release")
endif()

# 设置
set(CMAKE_CXX_STANDARD 11)  # C++标准

# 源码
list(APPEND SOURCES
  src/main.cc
  src/file_util.cc
  src/file_util.h
  src/webp.cc
  src/webp.h
)

# 头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 依赖库
# add_library(lm STATIC IMPORTED)
# set_target_properties(lm PROPERTIES IMPORTED_LOCATION libm.a)
list(APPEND EXTRA_LIBS
  libjpeg.a
  libpng.a
  libwebp.a
  libm.a
  libz.a
  -lpthread
)

# 生成目标文件名和编译源码
add_executable(webp_demo ${SOURCES})
# 链接库
target_link_libraries(webp_demo ${EXTRA_LIBS})
