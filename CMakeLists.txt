cmake_minimum_required(VERSION 2.6)
project(check_redis)

set (CHECK_REDIS_VERSION "1.0.0")
# set(CMAKE_BUILD_TYPE Debug)

include (FindPkgConfig)

pkg_search_module(HIREDIS REQUIRED hiredis)
include_directories(SYSTEM ${HIREDIS_INCLUDE_DIRS})
link_directories(${HIREDIS_LIBRARY_DIRS})

add_library(util util.cpp)
add_library(redis_info redis_info.cpp)

# header file for results
configure_file("${PROJECT_SOURCE_DIR}/check_redis_config.h.in" "${PROJECT_SOURCE_DIR}/check_redis_config.h")

add_executable(check_redis_slave check_redis_slave.cpp)

target_link_libraries(check_redis_slave redis_info)
target_link_libraries(check_redis_slave util)
target_link_libraries(check_redis_slave ${HIREDIS_LDFLAGS})

install(TARGETS check_redis_slave DESTINATION bin)

