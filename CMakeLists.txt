cmake_minimum_required(VERSION 3.5)
project(PluginLoader CXX)

set(USE_GOOGLE_TEST "Build with Google test options" CACHE BOOL FALSE)
set(USE_EXAMPLE "Build with example test options" CACHE BOOL FALSE)

SET(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

set(${PROJECT_NAME}_SRCS
    plugins/Console.cpp
    plugins/MetaObject.cpp
    plugins/MultiLibraryPluginLoader.cpp
    plugins/PluginLoader.cpp
    plugins/PluginLoaderCore.cpp
    plugins/SharedLibrary.cpp
)

set(${PROJECT_NAME}_HEADERS
    plugins/VisibilityControl.h    
    plugins/Console.h
    plugins/Exceptions.hpp
    plugins/MetaObject.hpp
    plugins/MultiLibraryPluginLoader.hpp
    plugins/PluginLoader.hpp
    plugins/PluginLoaderCore.hpp
    plugins/SharedLibrary.hpp
    plugins/PluginMacro.hpp
)

#
add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRCS} ${${PROJECT_NAME}_HEADERS})

# Causes the visibility macros to use dllexport rather than dllimport
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PRIVATE "PLUGIN_LOADER_BUILDING_DLL")

SET(PLUGIN_LOADER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# For install
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${${PROJECT_NAME}_HEADERS}")
target_include_directories(${PROJECT_NAME} PUBLIC 	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/plugins>
                                                    $<INSTALL_INTERFACE:include>)
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}targets
        PUBLIC_HEADER DESTINATION include/plugins
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

# foo-config.cmake 을 자동 생성하여 설치한다
install(EXPORT ${PROJECT_NAME}targets         # 6.a
    FILE ${PROJECT_NAME}-config.cmake         # 6.b 파일 이름을 지정한다
    DESTINATION lib/cmake/                    # 6.c 설치할 곳
    EXPORT_LINK_INTERFACE_LIBRARIES)          # 6.d 동시에 링크해야할 라이브러리를 export 한다

if(USE_GOOGLE_TEST)
    add_subdirectory(testGoogle)
endif()

if(USE_EXAMPLE)
    add_subdirectory(testSimple)
    add_subdirectory(testMathFunctions)
endif()

