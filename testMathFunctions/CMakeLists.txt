cmake_minimum_required(VERSION 3.5)


include_directories(${PLUGIN_LOADER_INCLUDE_DIR})

add_library(${PROJECT_NAME}_TestPluginsMathFunctions SHARED plugins.cpp)
target_link_libraries(${PROJECT_NAME}_TestPluginsMathFunctions ${PROJECT_NAME})


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

add_executable(${PROJECT_NAME}_TestMathFunction utest.cpp)
add_dependencies(${PROJECT_NAME}_TestMathFunction ${PROJECT_NAME} ${PROJECT_NAME}_TestPluginsMathFunctions)
target_link_libraries(${PROJECT_NAME}_TestMathFunction ${PROJECT_NAME} GTest::GTest)