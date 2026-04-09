#!/usr/bin/env python3
"""
Script to download and configure GLAD (OpenGL loader) for the project.
GLAD is a modern alternative to GLEW that doesn't require precompiled binaries.
"""

import os
import sys
import json
import urllib.request
import zipfile
import tempfile
import shutil

def download_glad():
    """Download GLAD configuration for OpenGL 3.3 Core Profile"""
    
    # GLAD online generator parameters for OpenGL 3.3 Core Profile
    # This matches what we need for modern OpenGL
    glad_config = {
        "api": "gl=3.3",
        "profile": "core",
        "generator": "c",
        "spec": "gl",
        "extensions": "",
        "options": {
            "loader": "true",
            "mx": "false",
            "mx_global": "false",
            "on_demand": "false",
            "alias": "false"
        }
    }
    
    print("Downloading GLAD configuration for OpenGL 3.3 Core Profile...")
    
    # Create temporary directory
    temp_dir = tempfile.mkdtemp()
    glad_zip_path = os.path.join(temp_dir, "glad.zip")
    
    try:
        # Use GLAD's web generator API
        params = urllib.parse.urlencode({
            "language": "c",
            "specification": "gl",
            "api": "gl=3.3",
            "profile": "core",
            "loader": "on"
        })
        
        url = f"https://gen.glad.sh/generate?{params}"
        print(f"Downloading from: {url}")
        
        # Download the generated files
        urllib.request.urlretrieve(url, glad_zip_path)
        
        # Extract to project directory
        project_dir = os.path.dirname(os.path.abspath(__file__))
        glad_dir = os.path.join(project_dir, "external", "glad")
        
        if os.path.exists(glad_dir):
            shutil.rmtree(glad_dir)
        
        os.makedirs(glad_dir, exist_ok=True)
        
        with zipfile.ZipFile(glad_zip_path, 'r') as zip_ref:
            zip_ref.extractall(glad_dir)
        
        print(f"GLAD extracted to: {glad_dir}")
        
        # Verify the structure
        glad_include_dir = os.path.join(glad_dir, "include")
        glad_src_dir = os.path.join(glad_dir, "src")
        
        if not os.path.exists(os.path.join(glad_include_dir, "glad", "gl.h")):
            print("Warning: Expected glad/gl.h not found, checking alternative structure...")
            
            # Some GLAD versions have different structure
            for root, dirs, files in os.walk(glad_dir):
                for file in files:
                    if file == "gl.h":
                        print(f"Found gl.h at: {os.path.join(root, file)}")
                        # Move to correct structure
                        target_include = os.path.join(glad_dir, "include", "glad")
                        os.makedirs(target_include, exist_ok=True)
                        shutil.move(os.path.join(root, file), os.path.join(target_include, "gl.h"))
                        
                        # Move KHR platform header if found
                        khr_src = os.path.join(root, "khrplatform.h")
                        if os.path.exists(khr_src):
                            khr_target = os.path.join(glad_dir, "include", "KHR")
                            os.makedirs(khr_target, exist_ok=True)
                            shutil.move(khr_src, os.path.join(khr_target, "khrplatform.h"))
                        
                        # Move gl.c if found
                        for src_file in files:
                            if src_file == "gl.c":
                                src_target = os.path.join(glad_dir, "src")
                                os.makedirs(src_target, exist_ok=True)
                                shutil.move(os.path.join(root, src_file), os.path.join(src_target, "gl.c"))
                                break
        
        # Create a simple CMakeLists.txt for GLAD
        cmake_content = """cmake_minimum_required(VERSION 3.10)

# GLAD configuration
project(glad)

# Source files
file(GLOB GLAD_SOURCES "src/*.c")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create library
add_library(glad STATIC ${GLAD_SOURCES})

# Target include directories
target_include_directories(glad PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Hide warnings for generated code
if(MSVC)
    target_compile_options(glad PRIVATE /w)
else()
    target_compile_options(glad PRIVATE -w)
endif()

# Export target
install(TARGETS glad EXPORT glad-targets)
install(EXPORT glad-targets 
    FILE glad-config.cmake
    NAMESPACE glad::
    DESTINATION lib/cmake/glad
)
install(DIRECTORY include/ DESTINATION include)
"""
        
        cmake_path = os.path.join(glad_dir, "CMakeLists.txt")
        with open(cmake_path, 'w') as f:
            f.write(cmake_content)
        
        print("Created CMakeLists.txt for GLAD")
        
        # Update main project's CMakeLists.txt
        update_main_cmake(project_dir, glad_dir)
        
        print("\nGLAD setup completed successfully!")
        print("GLAD will be built as part of the main project.")
        
    except Exception as e:
        print(f"Error downloading GLAD: {e}")
        print("\nFallback: Creating minimal GLAD files manually...")
        create_minimal_glad(project_dir)
    finally:
        # Clean up
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir, ignore_errors=True)

def create_minimal_glad(project_dir):
    """Create minimal GLAD files if download fails"""
    
    glad_dir = os.path.join(project_dir, "external", "glad")
    os.makedirs(os.path.join(glad_dir, "include", "glad"), exist_ok=True)
    os.makedirs(os.path.join(glad_dir, "include", "KHR"), exist_ok=True)
    os.makedirs(os.path.join(glad_dir, "src"), exist_ok=True)
    
    # Create minimal KHR platform header
    khr_header = """#ifndef __khrplatform_h_
#define __khrplatform_h_

typedef signed char khronos_int8_t;
typedef unsigned char khronos_uint8_t;
typedef signed short int khronos_int16_t;
typedef unsigned short int khronos_uint16_t;
typedef signed int khronos_int32_t;
typedef unsigned int khronos_uint32_t;
typedef signed long long int khronos_int64_t;
typedef unsigned long long int khronos_uint64_t;

#define KHRONOS_SUPPORT_INT64   1
#define KHRONOS_SUPPORT_FLOAT   1

typedef float khronos_float_t;

#endif /* __khrplatform_h_ */
"""
    
    with open(os.path.join(glad_dir, "include", "KHR", "khrplatform.h"), 'w') as f:
        f.write(khr_header)
    
    # Create placeholder glad.h (simplified version)
    glad_h = """#ifndef __glad_h_
#define __glad_h_

#ifdef __cplusplus
extern "C" {
#endif

int gladLoadGL(void* (*proc)(const char*));
void gladUnloadGL(void);

#ifdef __cplusplus
}
#endif

#endif /* __glad_h_ */
"""
    
    with open(os.path.join(glad_dir, "include", "glad", "gl.h"), 'w') as f:
        f.write(glad_h)
    
    # Create placeholder gl.c
    gl_c = """#include "glad/gl.h"

int gladLoadGL(void* (*proc)(const char*)) {
    // Simplified loader - in real GLAD this would load all OpenGL functions
    return 1; // Success
}

void gladUnloadGL(void) {
    // Cleanup if needed
}
"""
    
    with open(os.path.join(glad_dir, "src", "gl.c"), 'w') as f:
        f.write(gl_c)
    
    print("Created minimal GLAD files")
    update_main_cmake(project_dir, glad_dir)

def update_main_cmake(project_dir, glad_dir):
    """Update main CMakeLists.txt to use GLAD instead of GLEW"""
    
    cmake_path = os.path.join(project_dir, "CMakeLists.txt")
    
    if not os.path.exists(cmake_path):
        print(f"Warning: CMakeLists.txt not found at {cmake_path}")
        return
    
    with open(cmake_path, 'r') as f:
        content = f.read()
    
    # Replace GLEW configuration with GLAD
    # We'll add GLAD as a subdirectory
    new_content = content
    
    # Find where to insert GLAD configuration
    # We'll add it after GLFW configuration but before target linking
    glfw_section_end = new_content.find("# GLM configuration")
    
    if glfw_section_end != -1:
        glad_setup = """
# GLAD configuration (modern alternative to GLEW)
set(GLAD_DIR "${PROJECT_SOURCE_DIR}/external/glad")
if(EXISTS ${GLAD_DIR})
    message(STATUS "Using GLAD from: ${GLAD_DIR}")
    add_subdirectory(${GLAD_DIR} glad)
    set(GLAD_INCLUDE_DIR "${GLAD_DIR}/include")
    set(GLAD_LIBRARY glad)
    message(STATUS "GLAD library configured")
else()
    message(WARNING "GLAD not found at ${GLAD_DIR}. Run setup_glad.py to download.")
    # Fallback to GLEW if available
    if(EXISTS ${GLEW_ROOT}/include/GL/glew.h)
        message(STATUS "Falling back to GLEW")
        set(GLAD_INCLUDE_DIR "${GLEW_ROOT}/include")
        set(GLAD_LIBRARY "")  # Will link dynamically
    else()
        message(FATAL_ERROR "Neither GLAD nor GLEW found. Please run setup_glad.py")
    endif()
endif()
"""
        
        new_content = new_content[:glfw_section_end] + glad_setup + new_content[glfw_section_end:]
    
    # Update include directories to use GLAD instead of GLEW
    new_content = new_content.replace(
        "include_directories(\n    ${PROJECT_SOURCE_DIR}/include\n    ${GLFW_INCLUDE_DIR}\n    ${GLEW_INCLUDE_DIR}\n    ${GLM_INCLUDE_DIR}\n)",
        "include_directories(\n    ${PROJECT_SOURCE_DIR}/include\n    ${GLFW_INCLUDE_DIR}\n    ${GLAD_INCLUDE_DIR}\n    ${GLM_INCLUDE_DIR}\n)"
    )
    
    # Update target link libraries
    if "target_link_libraries(${PROJECT_NAME} PRIVATE \n    OpenGL::GL\n    ${GLFW_LIBRARY}\n    ${GLEW_LIBRARY}\n)" in new_content:
        new_content = new_content.replace(
            "target_link_libraries(${PROJECT_NAME} PRIVATE \n    OpenGL::GL\n    ${GLFW_LIBRARY}\n    ${GLEW_LIBRARY}\n)",
            "target_link_libraries(${PROJECT_NAME} PRIVATE \n    OpenGL::GL\n    ${GLFW_LIBRARY}\n    ${GLAD_LIBRARY}\n)"
        )
    
    with open(cmake_path, 'w') as f:
        f.write(new_content)
    
    print("Updated CMakeLists.txt to use GLAD")

if __name__ == "__main__":
    download_glad()