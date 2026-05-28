@echo off
setlocal enabledelayedexpansion

:: 1. Настройка путей
set BUILD_DIR=build
set DIST_DIR=Dist
set ZIP_NAME=demo.zip

echo [1/4] Cleaning old files...
if exist %BUILD_DIR% rd /s /q %BUILD_DIR%
if exist %DIST_DIR% rd /s /q %DIST_DIR%
if exist %ZIP_NAME% del /q %ZIP_NAME%

echo [2/4] Configuring CMake (Release)...
cmake -S . -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo Configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

echo [3/4] Building project...
cmake --build %BUILD_DIR% --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo [4/4] Installing to %DIST_DIR%...
cmake --install %BUILD_DIR% --prefix "./%DIST_DIR%"

:: Архивация через PowerShell (так как в самом CMD нет встроенного zip)
echo [EXTRA] Creating ZIP archive...
powershell -Command "Compress-Archive -Path .\%DIST_DIR%\* -DestinationPath .\%ZIP_NAME% -Force"

echo ========================================
echo Done! Archive: %ZIP_NAME%
echo ========================================
pause