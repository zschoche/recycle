cmake_minimum_required (VERSION 2.8)
project(test)

add_definitions("-std=c++1y")
add_definitions("-stdlib=libc++")
add_definitions("-Wall")
add_definitions("-O3")

add_definitions("-fexceptions")

#defines
#add_definitions(-DBOOST_ASIO_DYN_LINK)


#tests
add_executable(test test.cpp)
target_link_libraries(test c++ boost_unit_test_framework boost_thread boost_system)

