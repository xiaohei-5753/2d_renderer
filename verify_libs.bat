@echo off
echo ========================================
echo OpenGL Library Verification Script
echo ========================================
echo.

set LIBRARY_PATH=E:\programming\libraries

echo Checking library directories...
echo.

:: Check GLFW
echo [1/3] Checking GLFW...
if exist "%LIBRARY_PATH%\glfw-3.4.bin.WIN64" (
  echo   GLFW directory: OK
  if exist "%LIBRARY_PATH%\glfw-3.4.bin.WIN64\include\GLFW\glfw3.h" (
    echo   GLFW headers: OK
  ) else (
    echo   GLFW headers: MISSING
  )
  if exist "%LIBRARY_PATH%\glfw-3.4.bin.WIN64\lib-vc2022\glfw3.lib" (
    echo   GLFW MSVC library: OK
  ) else (
    echo   GLFW MSVC library: MISSING
  )
  if exist "%LIBRARY_PATH%\glfw-3.4.bin.WIN64\lib-mingw-w64\libglfw3.a" (
    echo   GLFW MinGW library: OK
  ) else (
    echo   GLFW MinGW library: MISSING
  )
) else (
  echo   GLFW directory: NOT FOUND
)

echo.

:: Check GLEW
echo [2/3] Checking GLEW...
if exist "%LIBRARY_PATH%\glew-2.1.0" (
  echo   GLEW directory: OK
  if exist "%LIBRARY_PATH%\glew-2.1.0\include\GL\glew.h" (
    echo   GLEW headers: OK
  ) else (
    echo   GLEW headers: MISSING
  )
  if exist "%LIBRARY_PATH%\glew-2.1.0\lib\Release\x64\glew32.lib" (
    echo   GLEW MSVC library: OK
  ) else (
    echo   GLEW MSVC library: MISSING
  )
  if exist "%LIBRARY_PATH%\glew-2.1.0\bin\Release\x64\glew32.dll" (
    echo   GLEW DLL: OK
  ) else (
    echo   GLEW DLL: MISSING
  )
) else (
  echo   GLEW directory: NOT FOUND
)

echo.

:: Check GLM
echo [3/3] Checking GLM...
if exist "%LIBRARY_PATH%\glm" (
  echo   GLM directory: OK
  if exist "%LIBRARY_PATH%\glm\glm.hpp" (
    echo   GLM headers: OK
  ) else (
    echo   GLM headers: MISSING
  )
) else (
  echo   GLM directory: NOT FOUND
)

echo.
echo ========================================
echo Compiler Check
echo ========================================
echo.

:: Check for MSVC
where cl >nul 2>nul
if %errorlevel% equ 0 (
  echo MSVC compiler (cl.exe): FOUND
) else (
  echo MSVC compiler (cl.exe): NOT FOUND
)

:: Check for MinGW
where g++ >nul 2>nul
if %errorlevel% equ 0 (
  echo MinGW compiler (g++): FOUND
) else (
  echo MinGW compiler (g++): NOT FOUND
)

echo.
echo ========================================
echo Project Configuration
echo ========================================
echo.

echo Project path contains Chinese characters: "渲染器"
echo This may cause issues with CMake and build tools.
echo.
echo Recommendation: Move project to path without special characters.
echo Example: E:\programming\cpp\2d_renderer\optimized\AI-generate
echo.
echo To test compilation manually:
echo 1. Open Developer Command Prompt for VS (for MSVC)
echo 2. Or use MinGW shell (for g++)
echo 3. Try: cmake -B build -G "Visual Studio 17 2022" -A x64
echo.
echo Press any key to exit...
pause >nul