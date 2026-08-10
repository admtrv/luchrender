# CMakeLists.txt

cmake_minimum_required(VERSION 3.22)
project(BulletRender)

set(CMAKE_CXX_STANDARD 20)

# address sanitizer
option(SANITIZE "enable AddressSanitizer with extended checks" OFF)

if(SANITIZE)
    message(STATUS "AddressSanitizer enabled")
    set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ASAN_FLAGS}")

    # embed runtime ASAN options into binary
    add_compile_definitions(ASAN_OPTIONS="detect_leaks=1:strict_string_checks=1:check_initialization_order=1:detect_stack_use_after_return=1:detect_container_overflow=1:abort_on_error=1")
endif()

# build library without build files and main.cpp
set(LIB_NAME ${PROJECT_NAME})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*/.*build.*/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/external/.*")

# editor interface belongs to standalone tool
file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/interface/*.cpp")
list(REMOVE_ITEM SOURCES ${EDITOR_SOURCES})

# render library
add_library(${LIB_NAME} STATIC ${SOURCES})
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ImGui library
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_link_libraries(${LIB_NAME} PUBLIC imgui)

# GLAD
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)
target_link_libraries(${LIB_NAME} PUBLIC glad)

# linking OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${LIB_NAME} PUBLIC OpenGL::GL)

# linking GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(${LIB_NAME} PUBLIC glfw)

# linking glm
find_package(glm REQUIRED)
target_link_libraries(${LIB_NAME} PUBLIC glm::glm)

# tinyobjloader library
add_library(tinyobjloader STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader/tiny_obj_loader.cc)
target_include_directories(tinyobjloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader)
target_link_libraries(${LIB_NAME} PUBLIC tinyobjloader)

# stb_image
add_library(stb_image STATIC ${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.cpp)
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)
target_link_libraries(${LIB_NAME} PUBLIC stb_image)

# if standalone app
if(DEFINED PROJECT_IS_TOP_LEVEL)
    set(_default_app ${PROJECT_IS_TOP_LEVEL})
else()
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
        set(_default_app ON)
    else()
        set(_default_app OFF)
    endif()
endif()

set(BIN_NAME Demo)
option(BULLET_RENDER_BUILD_APP "build standalone app with local main.cpp" ${_default_app})

if(BULLET_RENDER_BUILD_APP)
    add_executable(${BIN_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${EDITOR_SOURCES})
    target_link_libraries(${BIN_NAME} PRIVATE ${LIB_NAME})
    set_target_properties(${BIN_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

    # copy resources to binary after compilation
    add_custom_command(TARGET ${BIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${BIN_NAME}>/assets
    )
endif()
