@echo off
REM ============================================
REM Generate project_info.h with project name
REM ============================================

REM Get current folder name (project name)
for %%I in (.) do set "PROJECT_NAME=%%~nxI"

REM Ensure output folder exists
if not exist "Core\Inc" (
    echo [ERROR] Core\Inc folder not found!
    exit /b 1
)

REM Write header file
(
    echo #ifndef PROJECT_INFO_H
    echo #define PROJECT_INFO_H
    echo #define PROJECT_NAME "%PROJECT_NAME%"
    echo #endif
) > "Core\Inc\project_info.h"

echo [INFO] Generated Core\Inc\project_info.h with PROJECT_NAME="%PROJECT_NAME%"
exit /b 0