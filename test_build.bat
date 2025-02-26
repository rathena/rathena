@echo off
setlocal enabledelayedexpansion

:: Colors for output
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "NC=[0m"

echo %YELLOW%Starting build and test verification...%NC%

:: Create build directory
if not exist build mkdir build
cd build || exit /b 1

:: Configure with tests enabled
echo.
echo %YELLOW%Configuring CMake...%NC%
cmake -DBUILD_TESTING=ON ^
      -DMYSQL_INCLUDE_DIR=C:/MySQL/include ^
      -DMYSQL_LIBRARY=C:/MySQL/lib/libmysql.lib ^
      .. || exit /b 1

:: Build the project
echo.
echo %YELLOW%Building project...%NC%
cmake --build . --config Release || exit /b 1

:: Setup test database
echo.
echo %YELLOW%Setting up test database...%NC%
mysql -u test_user -ptest_pass -e "source src/test/setup_test_db.sql"
if %ERRORLEVEL% EQU 0 (
    echo %GREEN%Test database setup complete%NC%
) else (
    echo %RED%Failed to setup test database%NC%
    exit /b 1
)

:: Run tests
echo.
echo %YELLOW%Running tests...%NC%

:: Run sync tests
echo.
echo %YELLOW%Running synchronization tests...%NC%
Release\network_tests.exe --gtest_filter=SyncTest.* || exit /b 1

:: Run network monitor tests
echo.
echo %YELLOW%Running network monitor tests...%NC%
Release\network_tests.exe --gtest_filter=NetworkMonitorTest.* || exit /b 1

:: Run config parser tests
echo.
echo %YELLOW%Running config parser tests...%NC%
Release\network_tests.exe --gtest_filter=P2PConfigParserTest.* || exit /b 1

:: Memory leak check with Dr. Memory if available
where DrMemory >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo.
    echo %YELLOW%Running memory leak checks with Dr. Memory...%NC%
    drmemory.exe -light ^
                 -logfile drmemory-out.txt ^
                 -- Release\network_tests.exe
    
    findstr "NO ERRORS FOUND" drmemory-out.txt >nul
    if %ERRORLEVEL% EQU 0 (
        echo %GREEN%No memory leaks detected%NC%
    ) else (
        echo %RED%Memory leaks detected! Check drmemory-out.txt for details%NC%
        exit /b 1
    )
)

:: Run performance tests if enabled
if "%1"=="--performance" (
    echo.
    echo %YELLOW%Running performance tests...%NC%
    Release\network_tests.exe --gtest_filter=*Performance*
)

:: Final cleanup
echo.
echo %YELLOW%Cleaning up test database...%NC%
mysql -u test_user -ptest_pass -e "DROP DATABASE IF EXISTS test_rathena"

echo.
echo %GREEN%All tests completed successfully!%NC%
exit /b 0

:error
echo %RED%Build or test process failed!%NC%
exit /b 1