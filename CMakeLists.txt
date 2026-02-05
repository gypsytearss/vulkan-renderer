cmake_minimum_required(VERSION 3.20...3.31)
project(vulkan-renderer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()


add_compile_options(-Wall)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(APPLE)
    add_definitions(-DPLATFORM_MAC)
elseif(UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif()

find_package(Vulkan REQUIRED)
find_program(GLSLC glslc REQUIRED)

# Shader compilation
set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/shaders)
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)

file(GLOB SHADER_SOURCES
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME}"
    )
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_OUTPUTS})

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(cgltf
    GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(glfw glm cgltf)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} shaders)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SHADER_DIR="${SHADER_BINARY_DIR}"
    ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${cgltf_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm)

# Testing
enable_testing()
add_test(NAME build_check
    COMMAND ${PROJECT_NAME} --build-check
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
