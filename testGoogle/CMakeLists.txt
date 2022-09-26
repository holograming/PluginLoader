cmake_minimum_required(VERSION 3.5)

include_directories(${PLUGIN_LOADER_INCLUDE_DIR})

add_library(${PROJECT_NAME}_TestPlugins1 SHARED plugins1.cpp)
target_link_libraries(${PROJECT_NAME}_TestPlugins1 ${PROJECT_NAME})
add_dependencies(${PROJECT_NAME}_TestPlugins1 ${PROJECT_NAME})

add_library(${PROJECT_NAME}_TestPlugins2 SHARED plugins2.cpp)
target_link_libraries(${PROJECT_NAME}_TestPlugins2 ${PROJECT_NAME})
add_dependencies(${PROJECT_NAME}_TestPlugins2 ${PROJECT_NAME})

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
# build library shared
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

enable_testing()

add_executable(${PROJECT_NAME}_utest utest.cpp)
add_dependencies(${PROJECT_NAME}_utest ${PROJECT_NAME} ${PROJECT_NAME}_TestPlugins1 ${PROJECT_NAME}_TestPlugins2)
add_test(NAME ${PROJECT_NAME}_utest COMMAND ${PROJECT_NAME}_utest)
target_link_libraries(${PROJECT_NAME}_utest PUBLIC ${PROJECT_NAME} GTest::GTest)

add_executable(${PROJECT_NAME}_shared_ptr_test shared_ptr_test.cpp)
add_dependencies(${PROJECT_NAME}_shared_ptr_test ${PROJECT_NAME} ${PROJECT_NAME}_TestPlugins1 ${PROJECT_NAME}_TestPlugins2)
add_test(NAME ${PROJECT_NAME}_shared_ptr_test COMMAND ${PROJECT_NAME}_shared_ptr_test)
target_link_libraries(${PROJECT_NAME}_shared_ptr_test PUBLIC ${PROJECT_NAME} GTest::GTest)

add_executable(${PROJECT_NAME}_unique_ptr_test unique_ptr_test.cpp)
add_dependencies(${PROJECT_NAME}_unique_ptr_test ${PROJECT_NAME} ${PROJECT_NAME}_TestPlugins1 ${PROJECT_NAME}_TestPlugins2)
add_test(NAME ${PROJECT_NAME}_unique_ptr_test COMMAND ${PROJECT_NAME}_unique_ptr_test)
target_link_libraries(${PROJECT_NAME}_unique_ptr_test PUBLIC ${PROJECT_NAME} GTest::GTest)