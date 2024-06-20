
:: This file cleans up generated .exe files, compiles the tests, and also runs them
:: This file is also licensed under the terms of the Apache-2.0 license.


@echo off
IF EXIST tests.exe (
    DEL /F tffn-test.exe
)

gcc -o tests tests.c -I.

IF %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
    exit /b %ERRORLEVEL%
)

@echo on
tests.exe

@echo off
IF EXIST tests.exe (
    DEL /F tests.exe
)

