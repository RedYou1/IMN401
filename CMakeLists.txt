cmake_minimum_required(VERSION 3.14)
include(FetchContent)

project(Infographie1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# ============================================================================
# Libraries
# ============================================================================

# GLM
add_subdirectory(${LIBS_DIR}/glm)

# GLAD
set(GLAD_SRC ${LIBS_DIR}/glad/src/glad.c)
add_library(glad STATIC ${GLAD_SRC})
target_include_directories(glad PUBLIC ${LIBS_DIR}/glad/include)
set_target_properties(glad PROPERTIES FOLDER Libs)

# ImGui
add_library(imgui STATIC
    ${LIBS_DIR}/imgui/imgui.cpp
    ${LIBS_DIR}/imgui/imgui_demo.cpp
    ${LIBS_DIR}/imgui/imgui_draw.cpp
    ${LIBS_DIR}/imgui/imgui_impl_glfw_gl3.cpp
)
target_include_directories(imgui PUBLIC 
    ${LIBS_DIR}/imgui 
    ${LIBS_DIR}/glad/include 
    ${GLFW_INCLUDE_DIR}          # ← This was missing / incorrect before
)
target_link_libraries(imgui PUBLIC glfw)
set_target_properties(imgui PROPERTIES FOLDER Libs)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${LIBS_DIR}/stb)

# GLFW (imported)
set(GLFW_INCLUDE_DIR ${LIBS_DIR}/GLFW/include)
set(GLFW_LIBRARY_DIR ${LIBS_DIR}/GLFW/lib)

add_library(glfw STATIC IMPORTED)
if(WIN32)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION ${GLFW_LIBRARY_DIR}/glfw3.lib
        INTERFACE_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR}
    )
else()
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION ${GLFW_LIBRARY_DIR}/libglfw3.a
        INTERFACE_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR}
    )
endif()
set_target_properties(glfw PROPERTIES FOLDER Libs/GLFW3)

# Logger
add_library(Logger STATIC ${LIBS_DIR}/Logger/ImGUILogger.cpp)
target_include_directories(Logger PUBLIC ${LIBS_DIR}/Logger ${LIBS_DIR})
set_target_properties(Logger PROPERTIES FOLDER Libs)

# image_DXT
add_library(image_DXT STATIC ${LIBS_DIR}/image_DXT.cpp)
target_include_directories(image_DXT PUBLIC ${LIBS_DIR})
set_target_properties(image_DXT PROPERTIES FOLDER Libs)

# ============================================================================
# Main Executable
# ============================================================================

file(GLOB_RECURSE SOURCE_FILES 
    ${SOURCE_DIR}/*.cpp 
    ${SOURCE_DIR}/*.h 
    ${SOURCE_DIR}/*.hpp
)

add_executable(${PROJECT_NAME} 
    ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SOURCE_DIR}
    ${LIBS_DIR}/glad/include
    ${GLFW_INCLUDE_DIR}           # ← Important for glfw3native.h
    ${LIBS_DIR}/Logger
    ${LIBS_DIR}/imgui
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    imgui 
    glm 
    stb 
    glad 
    glfw 
    Logger 
    image_DXT
)

if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
# ============================================================================
# Unit tests (doctest)
# ============================================================================
include(CTest)
enable_testing()

FetchContent_Declare(
    doctest
    SOURCE_DIR ${LIBS_DIR}/doctest
)
FetchContent_MakeAvailable(doctest)

# Make each doctest TEST_CASE show up as an individual CTest test.
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

add_executable(Infographie1_tests
    tests/Main_tests.cpp
    src/Frame.cpp
    src/Camera.cpp
    src/EngineGL.cpp
    src/EffectGL.cpp
    src/Texture2D.cpp
    src/FrameBufferObject.cpp
    src/Effects/Display/Display.cpp
    src/GeometricModel.cpp
    src/GeometricModelLoader/OBJLoader.cpp
    src/GLProgram.cpp
    src/GLProgramPipeline.cpp
    src/MaterialGL.cpp
    src/ModelGL.cpp
    src/Node.cpp
    src/Scene.cpp
    src/Materials/BaseMaterial/BaseMaterial.cpp
    src/Plane.cpp
    src/Sphere.cpp
    src/Ray.cpp
    src/VectorPrimitive.cpp)

target_include_directories(Infographie1_tests PRIVATE
    ${SOURCE_DIR}
    ${LIBS_DIR}/glad/include
    ${GLFW_INCLUDE_DIR}
    ${LIBS_DIR}
    ${LIBS_DIR}/Logger
    ${LIBS_DIR}/imgui
)
target_link_libraries(Infographie1_tests PRIVATE doctest::doctest glm glad glfw imgui Logger image_DXT)

if(UNIX AND NOT APPLE)
    if(INF1_TESTS_USE_EGL)
        target_compile_definitions(Infographie1_tests PRIVATE INF1_TESTS_USE_EGL=1)
    else()
        target_link_libraries(Infographie1_tests PRIVATE pthread dl X11)
    endif()
endif()
if(MSVC)
    target_link_options(Infographie1_tests PRIVATE /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCMTD)
endif()

doctest_discover_tests(Infographie1_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_target_properties(Infographie1_tests PROPERTIES FOLDER Tests)
