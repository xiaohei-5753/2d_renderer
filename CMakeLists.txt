cmake_minimum_required(VERSION 3.10)
project(PixelRenderer2D VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler optimization flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # MinGW/GCC flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -ffast-math")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # MSVC flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ot /GL")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
endif()

# Common flags for all build types
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Detect compiler
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(USING_MSVC TRUE)
    message(STATUS "Using MSVC compiler")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(USING_MINGW TRUE)
    message(STATUS "Using MinGW compiler")
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Default library paths (update these if your libraries are in different locations)
set(DEFAULT_GLFW_ROOT "E:/programming/libraries/glfw-3.4.bin.WIN64")
set(DEFAULT_GLEW_ROOT "E:/programming/libraries/glew-2.1.0")
set(DEFAULT_GLM_ROOT "E:/programming/libraries/glm")

# Library paths (can be overridden with -D option)
set(GLFW_ROOT "${DEFAULT_GLFW_ROOT}" CACHE PATH "Path to GLFW installation")
set(GLEW_ROOT "${DEFAULT_GLEW_ROOT}" CACHE PATH "Path to GLEW installation")
set(GLM_ROOT "${DEFAULT_GLM_ROOT}" CACHE PATH "Path to GLM installation")

# Find OpenGL (system library)
find_package(OpenGL REQUIRED)

# GLFW configuration
if(EXISTS ${GLFW_ROOT})
    message(STATUS "Using GLFW from: ${GLFW_ROOT}")
    set(GLFW_INCLUDE_DIR "${GLFW_ROOT}/include")
    
    # Select library based on compiler
    if(USING_MSVC)
        if(EXISTS "${GLFW_ROOT}/lib-vc2022/glfw3.lib")
            set(GLFW_LIBRARY "${GLFW_ROOT}/lib-vc2022/glfw3.lib")
            message(STATUS "GLFW library: ${GLFW_LIBRARY}")
        else()
            message(FATAL_ERROR "GLFW MSVC library not found at ${GLFW_ROOT}/lib-vc2022/glfw3.lib")
        endif()
    elseif(USING_MINGW)
        if(EXISTS "${GLFW_ROOT}/lib-mingw-w64/libglfw3.a")
            set(GLFW_LIBRARY "${GLFW_ROOT}/lib-mingw-w64/libglfw3.a")
            message(STATUS "GLFW library: ${GLFW_LIBRARY}")
        else()
            message(FATAL_ERROR "GLFW MinGW library not found at ${GLFW_ROOT}/lib-mingw-w64/libglfw3.a")
        endif()
    else()
        message(FATAL_ERROR "Unsupported compiler for GLFW")
    endif()
else()
    message(FATAL_ERROR "GLFW not found at ${GLFW_ROOT}. Please install GLFW or set GLFW_ROOT.")
endif()

# GLEW configuration
if(EXISTS ${GLEW_ROOT})
    message(STATUS "Using GLEW from: ${GLEW_ROOT}")
    set(GLEW_INCLUDE_DIR "${GLEW_ROOT}/include")
    
    # Select library based on compiler
    if(USING_MSVC)
        if(EXISTS "${GLEW_ROOT}/lib/Release/x64/glew32.lib")
            set(GLEW_LIBRARY "${GLEW_ROOT}/lib/Release/x64/glew32.lib")
            set(GLEW_DLL "${GLEW_ROOT}/bin/Release/x64/glew32.dll")
            message(STATUS "GLEW library: ${GLEW_LIBRARY}")
        else()
            message(FATAL_ERROR "GLEW MSVC library not found at ${GLEW_ROOT}/lib/Release/x64/glew32.lib")
        endif()
    elseif(USING_MINGW)
        # Try multiple possible locations for MinGW GLEW library
        set(GLEW_FOUND FALSE)
        
        # Check for libglew32.a in various locations
        if(EXISTS "${GLEW_ROOT}/lib/libglew32.a")
            set(GLEW_LIBRARY "${GLEW_ROOT}/lib/libglew32.a")
            set(GLEW_FOUND TRUE)
            message(STATUS "GLEW library: ${GLEW_LIBRARY}")
        elseif(EXISTS "${GLEW_ROOT}/lib/Release/x64/libglew32.a")
            set(GLEW_LIBRARY "${GLEW_ROOT}/lib/Release/x64/libglew32.a")
            set(GLEW_FOUND TRUE)
            message(STATUS "GLEW library: ${GLEW_LIBRARY}")
        elseif(EXISTS "${GLEW_ROOT}/lib/Release/Win32/libglew32.a")
            set(GLEW_LIBRARY "${GLEW_ROOT}/lib/Release/Win32/libglew32.a")
            set(GLEW_FOUND TRUE)
            message(STATUS "GLEW library: ${GLEW_LIBRARY}")
        endif()
        
        # Also check for dynamic library import lib
        if(NOT GLEW_FOUND AND EXISTS "${GLEW_ROOT}/lib/Release/x64/libglew32.dll.a")
            set(GLEW_LIBRARY "${GLEW_ROOT}/lib/Release/x64/libglew32.dll.a")
            set(GLEW_FOUND TRUE)
            message(STATUS "GLEW library (dynamic): ${GLEW_LIBRARY}")
        endif()
        
        # Check for MinGW-built libraries in subdirectory
        if(NOT GLEW_FOUND AND EXISTS "${GLEW_ROOT}/lib-mingw/lib/libglew32.a")
            set(GLEW_LIBRARY "${GLEW_ROOT}/lib-mingw/lib/libglew32.a")
            set(GLEW_FOUND TRUE)
            message(STATUS "GLEW library (MinGW build): ${GLEW_LIBRARY}")
        endif()
        
        # If still not found, provide helpful error message
        if(NOT GLEW_FOUND)
            message(WARNING "GLEW MinGW library not found at standard locations.")
            message(WARNING "You have several options:")
            message(WARNING "1. Build GLEW from source using build_glew_for_mingw.bat script")
            message(WARNING "2. Download pre-built MinGW GLEW libraries")
            message(WARNING "3. Use MSVC compiler instead of MinGW")
            message(WARNING "4. Use GLAD (https://glad.dav1d.de/) as a modern alternative")
            
            # Check if we have a build script
            if(EXISTS "${PROJECT_SOURCE_DIR}/build_glew_for_mingw.bat")
                message(STATUS "Build script available: ${PROJECT_SOURCE_DIR}/build_glew_for_mingw.bat")
                message(STATUS "Run this script to build GLEW for MinGW")
            endif()
            
            # Try to use MSVC library as fallback (with strong warning)
            if(EXISTS "${GLEW_ROOT}/lib/Release/x64/glew32.lib")
                message(WARNING "FALLBACK: Using MSVC GLEW library with MinGW (likely incompatible)")
                set(GLEW_LIBRARY "${GLEW_ROOT}/lib/Release/x64/glew32.lib")
                set(GLEW_FOUND TRUE)
            else()
                message(FATAL_ERROR "GLEW library not found. Please build GLEW for MinGW or use MSVC compiler.")
            endif()
        endif()
    else()
        message(FATAL_ERROR "Unsupported compiler for GLEW")
    endif()
else()
    message(FATAL_ERROR "GLEW not found at ${GLEW_ROOT}. Please install GLEW or set GLEW_ROOT.")
endif()

# GLAD configuration disabled - using GLEW instead
set(GLAD_INCLUDE_DIR "")
set(GLAD_LIBRARY "")

# GLM configuration (header-only)
if(EXISTS ${GLM_ROOT})
    message(STATUS "Using GLM from: ${GLM_ROOT}")
    set(GLM_INCLUDE_DIR "${GLM_ROOT}")
else()
    message(FATAL_ERROR "GLM not found at ${GLM_ROOT}. Please install GLM or set GLM_ROOT.")
endif()

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
)

# Source files
file(GLOB_RECURSE SRC_FILES 
    "src/*.cpp"
)

# Ensure expression.cpp is included (in case GLOB misses it)
list(APPEND SRC_FILES "src/expression.cpp")

# Shader files
file(GLOB SHADER_FILES
    "shaders/*.vert"
    "shaders/*.frag"
    "shaders/*.comp"
)

# Add executable
add_executable(${PROJECT_NAME} ${SRC_FILES} ${SHADER_FILES})

# Target includes
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
    ${GLEW_INCLUDE_DIR}
)

# Target libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenGL::GL
    ${GLFW_LIBRARY}
    ${GLEW_LIBRARY}
)

# GLAD library disabled - using GLEW instead
# if(GLAD_LIBRARY)
#     target_link_libraries(${PROJECT_NAME} PRIVATE ${GLAD_LIBRARY})
# endif()

# Windows-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        gdi32
        opengl32
    )
    
    # For MSVC, copy DLLs to output directory
    if(USING_MSVC AND GLEW_DLL AND EXISTS ${GLEW_DLL})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${GLEW_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glew32.dll"
            COMMENT "Copying GLEW DLL to output directory"
        )
    endif()
    
    # Copy GLFW DLL if needed (GLFW might be statically linked)
    if(USING_MSVC AND EXISTS "${GLFW_ROOT}/lib-vc2022/glfw3.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${GLFW_ROOT}/lib-vc2022/glfw3.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll"
            COMMENT "Copying GLFW DLL to output directory"
        )
    endif()
endif()

# Copy shaders to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/shaders/raytrace.vert"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/raytrace.vert"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/shaders/raytrace.frag"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/raytrace.frag"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/shaders/raytrace.comp"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/raytrace.comp"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/shaders/display.vert"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/display.vert"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/shaders/display.frag"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/display.frag"
    COMMENT "Copying shaders to output directory"
)

# Print configuration summary
message(STATUS "=========================================")
message(STATUS "PixelRenderer2D Configuration Summary")
message(STATUS "=========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "GLFW: ${GLFW_ROOT}")
message(STATUS "GLEW: ${GLEW_ROOT}")
message(STATUS "GLM: ${GLM_ROOT}")
message(STATUS "Source files count: ${SRC_FILES}")
message(STATUS "=========================================")

# Create a simple test to verify OpenGL setup
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling, skipping OpenGL test")
else()
    # We could add a test here to verify OpenGL works
    message(STATUS "OpenGL libraries configured successfully")
endif()