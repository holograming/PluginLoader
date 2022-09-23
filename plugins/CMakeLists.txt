cmake_minimum_required(VERSION 3.5)
project(PluginLoader CXX)

# 1. INTERFACE_INCLUDE_DIRECTORIES 에 소스 디렉토리와 빌드 디렉토리를 자동으로 추가한다.
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

# xxxd.lib
# xxxd.dll
set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Adds a postfix for debug-built libraries.")

add_compile_options(-fPIC)

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

#add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRCS} ${${PROJECT_NAME}_HEADERS})
add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRCS} ${${PROJECT_NAME}_HEADERS})

# 3. 설치 할 헤더 파일을 지정해 둔다.
# 지정한 파일에는 INSTALL(TARGETS) 의 PUBLIC_HEADER 로 지정한 디렉토리에 설치된다.
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${${PROJECT_NAME}_HEADERS}")

# Causes the visibility macros to use dllexport rather than dllimport
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PRIVATE "PLUGIN_LOADER_BUILDING_DLL")

#target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#https://medium.com/@yjo/cmake-%EB%B9%8C%EB%93%9C-%EC%8B%9C%EC%8A%A4%ED%85%9C-%EB%A7%8C%EB%93%A4%EA%B8%B0-9ec3e2d66cf0

target_include_directories(${PROJECT_NAME} PUBLIC 	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/plugins>
                                                    $<INSTALL_INTERFACE:include>)
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}targets
        PUBLIC_HEADER DESTINATION include/plugins
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

# 6. foo-config.cmake 을 자동 생성하여 설치한다
install(EXPORT ${PROJECT_NAME}targets                     # 6.a
    FILE ${PROJECT_NAME}-config.cmake                     # 6.b 파일 이름을 지정한다
    DESTINATION lib/cmake/              # 6.c 설치할 곳
    EXPORT_LINK_INTERFACE_LIBRARIES)          # 6.d 동시에 링크해야할 라이브러리를 export 한다

#add_subdirectory(example)
