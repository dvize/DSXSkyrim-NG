@echo off

cmake -S . --preset=FLATRIM --check-stamp-file "build\CMakeFiles\generate.stamp"
if %ERRORLEVEL% NEQ 0 exit 1
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 exit 1

pause