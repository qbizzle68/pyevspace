@echo off
setlocal

set "REPO_ROOT=%~dp0"
rem %~dp0 includes a trailing backslash, remove it
set "REPO_ROOT=%REPO_ROOT:~0,-1%"

set UPDATE_SUBMODULES=

:parse_args
if "%~1"=="" goto run
if "%~1"=="--help" goto print_help
if "%~1"=="-h" goto print_help
if "%~1"=="--update-submodules" (
    set UPDATE_SUBMODULES=1
    shift
    goto parse_args
)
echo Error: unrecognized option '%~1'>&2
call :print_help >&2
exit /b 1

:print_help
echo Syntax: %~nx0 [--update-submodules] [--help^|-h]
echo.
echo     Setup script for the pyevspace git repo. This script initializes the environment by
echo     creating symlinks for git hooks. If present, the '--update-submodules' option will
echo     also update the git submodules for this repo.
goto :eof

:run
if defined UPDATE_SUBMODULES (
    echo Updating submodules
    git submodule update --init --recursive
    if errorlevel 1 exit /b 1
)

echo Symlinking hooks
if exist "%REPO_ROOT%\.git\hooks\pre-push" del "%REPO_ROOT%\.git\hooks\pre-push"
mklink "%REPO_ROOT%\.git\hooks\pre-push" "%REPO_ROOT%\scripts\hooks\pre-push"
if errorlevel 1 (
    echo Error: failed to create symlink. Run as Administrator or enable Developer Mode in Windows Settings.>&2
    exit /b 1
)

endlocal