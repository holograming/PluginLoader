cmake_minimum_required(VERSION 3.5)


include_directories(${PLUGIN_LOADER_INCLUDE_DIR})

add_library(${PROJECT_NAME}_TestSimplePlugins SHARED plugins.h plugins.cpp)
target_link_libraries(${PROJECT_NAME}_TestSimplePlugins ${PROJECT_NAME})

add_executable(${PROJECT_NAME}_TestSimple utest.cpp)
add_dependencies(${PROJECT_NAME}_TestSimple ${PROJECT_NAME} ${PROJECT_NAME}_TestSimplePlugins)
target_link_libraries(${PROJECT_NAME}_TestSimple ${PROJECT_NAME})