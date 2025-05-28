@echo off
setlocal enabledelayedexpansion

for %%c in (Release Debug) do (
    echo Building %%c version...
    cmake --preset win-x64-%%c
    cmake --build --preset %%c
    if errorlevel 1 (
        echo Build failed for %%c
        exit /b 1
    )
)

echo All builds completed successfully