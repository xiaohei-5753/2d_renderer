@echo off
REM Build GLEW for MinGW from source
REM Based on instructions from https://github.com/nigels-com/glew/issues/223

echo Building GLEW for MinGW...

REM Check for required tools
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: gcc not found in PATH. Make sure MinGW is installed and in PATH.
    pause
    exit /b 1
)

where ar >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: ar (archive utility) not found. Make sure MinGW bin directory is in PATH.
    pause
    exit /b 1
)

REM Set paths
set GLEW_SOURCE_DIR=E:\programming\libraries\glew-2.1.0
set BUILD_DIR=%GLEW_SOURCE_DIR%\build-mingw
set OUTPUT_DIR=%GLEW_SOURCE_DIR%\lib-mingw

REM Check if source exists
if not exist "%GLEW_SOURCE_DIR%\src\glew.c" (
    echo Error: GLEW source not found at %GLEW_SOURCE_DIR%\src\glew.c
    echo Downloading GLEW source from GitHub...
    
    REM Download GLEW 2.1.0 source
    curl -L -o glew-2.1.0.zip https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.zip
    if %errorlevel% neq 0 (
        echo Error: Failed to download GLEW source.
        pause
        exit /b 1
    )
    
    REM Extract source
    powershell -Command "Expand-Archive -Path glew-2.1.0.zip -DestinationPath %GLEW_SOURCE_DIR% -Force"
    if %errorlevel% neq 0 (
        echo Error: Failed to extract GLEW source.
        pause
        exit /b 1
    )
    
    REM Clean up zip file
    del glew-2.1.0.zip
)

REM Create build directories
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
if not exist "%OUTPUT_DIR%\lib" mkdir "%OUTPUT_DIR%\lib"
if not exist "%OUTPUT_DIR%\bin" mkdir "%OUTPUT_DIR%\bin"

echo Building GLEW library...

REM Compile glew.c to object file
cd "%GLEW_SOURCE_DIR%"
gcc -DGLEW_NO_GLU -O2 -Wall -W -Iinclude -DGLEW_BUILD -o src/glew.o -c src/glew.c
if %errorlevel% neq 0 (
    echo Error: Failed to compile glew.c
    pause
    exit /b 1
)

REM Create shared library (DLL) and import library
gcc -nostdlib -shared -Wl,-soname,libglew32.dll -Wl,--out-implib,%OUTPUT_DIR%\lib\libglew32.dll.a -o %OUTPUT_DIR%\bin\glew32.dll src/glew.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
if %errorlevel% neq 0 (
    echo Warning: Failed to create shared library, trying without -nostdlib...
    gcc -shared -Wl,-soname,libglew32.dll -Wl,--out-implib,%OUTPUT_DIR%\lib\libglew32.dll.a -o %OUTPUT_DIR%\bin\glew32.dll src/glew.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
    if %errorlevel% neq 0 (
        echo Error: Failed to create shared library
        pause
        exit /b 1
    )
)

REM Create static library
ar cr %OUTPUT_DIR%\lib\libglew32.a src/glew.o
if %errorlevel% neq 0 (
    echo Error: Failed to create static library
    pause
    exit /b 1
)

echo Building GLEW MX variant (multi-context)...

REM Compglew MX variant
gcc -DGLEW_NO_GLU -DGLEW_MX -O2 -Wall -W -Iinclude -DGLEW_BUILD -o src/glew.mx.o -c src/glew.c
if %errorlevel% neq 0 (
    echo Warning: Failed to compile MX variant, skipping...
) else (
    ar cr %OUTPUT_DIR%\lib\libglew32mx.a src/glew.mx.o
    if %errorlevel% neq 0 (
        echo Warning: Failed to create MX static library
    )
)

echo Copying header files...
xcopy "%GLEW_SOURCE_DIR%\include\GL" "%OUTPUT_DIR%\include\GL" /E /I /Y
if %errorlevel% neq 0 (
    echo Warning: Failed to copy header files
)

echo.
echo =============================================
echo GLEW for MinGW build completed successfully!
echo =============================================
echo.
echo Libraries built:
echo   - %OUTPUT_DIR%\lib\libglew32.a (static)
echo   - %OUTPUT_DIR%\lib\libglew32.dll.a (import library for DLL)
echo   - %OUTPUT_DIR%\bin\glew32.dll (dynamic library)
echo.
echo Header files copied to: %OUTPUT_DIR%\include\GL
echo.
echo To use with CMake, update GLEW_ROOT to: %OUTPUT_DIR%
echo.
pause