@echo off
setlocal enabledelayedexpansion

REM 配置变量
set QT_BIN="D:\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe"
set BUILD_DIR="..\build\x64\Release"
set QML_DIR="..\src\QtUI"
set NSIS_SCRIPT="GameOptimizerPro.nsi"
set ARCH="x64"

REM 创建安装程序输出目录
if not exist "%ARCH%" mkdir "%ARCH%"

REM 1. 检查 windeployqt 是否存在
if not exist %QT_BIN% (
  echo Error: windeployqt.exe not found at %QT_BIN%
  pause
  exit /b 1
)

REM 2. 检查目标 EXE 是否存在
if not exist "%BUILD_DIR%\GameOptimizerPro_%ARCH%.exe" (
  echo Error: GameOptimizerPro_%ARCH%.exe not found in %BUILD_DIR%
  pause
  exit /b 1
)

REM 3. 运行 windeployqt 添加 Qt 依赖
echo Running windeployqt to gather Qt dependencies...
%QT_BIN% --release --qmldir %QML_DIR% --compiler-runtime --no-translations "%BUILD_DIR%\GameOptimizerPro_%ARCH%.exe"

if %ERRORLEVEL% neq 0 (
  echo Error: windeployqt failed!
  pause
  exit /b 1
)

REM 4. 检查 NSIS 是否可用
where NSIS.exe >nul 2>&1
if %ERRORLEVEL% neq 0 (
  echo Error: NSIS - makensis no found in PATH!
  pause
  exit /b 1
)

REM 5. 运行 NSIS 构建安装包
echo Building installer with NSIS...

REM 清理旧安装包
del /q "%ARCH%\*" >nul 2>&1

REM 在运行 makensis 前计算哈希值
@REM for /f "tokens=*" %%i in ('certutil -hashfile "%BUILD_DIR%\GameOptimizerPro_%ARCH%.exe" SHA256 ^| find /v "hash" ^| find /v "CertUtil"') do (
@REM  set EXE_HASH=%%i
@REM )
@REM /DEXE_HASH="%EXE_HASH%"
makensis /DBUILD_ARCH=%ARCH% %NSIS_SCRIPT%

if %ERRORLEVEL% neq 0 (
  echo Error: NSIS compilation failed!
  pause
  exit /b 1
)

REM 6. 生成安装程序的哈希值文件
set INSTALLER_EXE="%ARCH%\GameOptimizerPro_%ARCH%_Setup.exe"
if exist %INSTALLER_EXE% (
  echo Generating SHA256 hash for installer...
  REM 生成哈希文件（仅保留哈希值）
  certutil -hashfile %INSTALLER_EXE% SHA256 | findstr /v "SHA256" | findstr /v "CertUtil" > "%ARCH%\GameOptimizerPro_%ARCH%_Setup.exe.sha256"
  echo Hash saved to %ARCH%\GameOptimizerPro_%ARCH%_Setup.exe.sha256
  ) else (
  echo Error: Installer not found at %INSTALLER_EXE%
)

echo Installer built successfully!
pause
