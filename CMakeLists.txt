cmake_minimum_required(VERSION 2.6)
project(check_redis)

set (CHECK_REDIS_VERSION "1.3.1-20200913")
# set(CMAKE_BUILD_TYPE Debug)

include (FindPkgConfig)

pkg_search_module(HIREDIS REQUIRED hiredis)
include_directories(SYSTEM ${HIREDIS_INCLUDE_DIRS})
link_directories(${HIREDIS_LIBRARY_DIRS})

add_library(util util.cpp)
add_library(redis_info redis_info.cpp)
add_library(redis_slave_info redis_slave_info.cpp)

# header file for results
configure_file("${PROJECT_SOURCE_DIR}/check_redis_config.h.in" "${PROJECT_SOURCE_DIR}/check_redis_config.h")

add_executable(check_redis_slave check_redis_slave.cpp)
target_link_libraries(check_redis_slave redis_info)
target_link_libraries(check_redis_slave util)
target_link_libraries(check_redis_slave redis_slave_info)
target_link_libraries(check_redis_slave ${HIREDIS_LDFLAGS})

add_executable(check_redis_slave_replication check_redis_slave_replication.cpp)
target_link_libraries(check_redis_slave_replication redis_info)
target_link_libraries(check_redis_slave_replication util)
target_link_libraries(check_redis_slave_replication redis_slave_info)
target_link_libraries(check_redis_slave_replication ${HIREDIS_LDFLAGS})

add_executable(check_redis_master check_redis_master.cpp)
target_link_libraries(check_redis_master redis_info)
target_link_libraries(check_redis_master util)
target_link_libraries(check_redis_master redis_slave_info)
target_link_libraries(check_redis_master ${HIREDIS_LDFLAGS})

add_executable(check_redis_master_connected_slaves check_redis_master_connected_slaves.cpp)
target_link_libraries(check_redis_master_connected_slaves redis_info)
target_link_libraries(check_redis_master_connected_slaves util)
target_link_libraries(check_redis_master_connected_slaves redis_slave_info)
target_link_libraries(check_redis_master_connected_slaves ${HIREDIS_LDFLAGS})

install(TARGETS check_redis_slave DESTINATION lib/nagios/plugins)
install(TARGETS check_redis_slave_replication DESTINATION lib/nagios/plugins)
install(TARGETS check_redis_master DESTINATION lib/nagios/plugins)
install(TARGETS check_redis_master_connected_slaves DESTINATION lib/nagios/plugins)

