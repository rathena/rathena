#!/usr/bin/env pwsh
# Code Quality Check Script for rAthena
# This script runs the same checks as the GitHub Actions code quality pipeline

param(
    [switch]$SkipCMake,
    [switch]$SkipFormat,
    [switch]$SkipTidy,
    [switch]$FixFormat,
    [string]$BaseBranch = "master",
    [string]$BaseCommit = "",
    [string]$HeadCommit = "",
    [string]$Generator = ""
)

$ErrorActionPreference = "Stop"
$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root_dir = Split-Path -Parent $script_dir

Write-Host "==================================" -ForegroundColor Cyan
if ($FixFormat) {
    Write-Host "rAthena Code Formatter" -ForegroundColor Cyan
} else {
    Write-Host "rAthena Code Quality Check" -ForegroundColor Cyan
}
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Change to root directory
Set-Location $root_dir

$failed_checks = @()

# Step 1: CMake Generation Check (only for Visual Studio generators)
if (-not $SkipCMake) {
    Write-Host "[1/3] Checking CMake generation..." -ForegroundColor Yellow
    
    # Determine which generator to use
    $RunCMakeCheck = $true
    if (-not $Generator) {
        if ($IsWindows -or $env:OS -eq "Windows_NT") {
            $Generator = "Visual Studio 17 2022"
            $GeneratorArgs = @("-A", "x64")
            Write-Host "Generating Visual Studio 2022 project files..." -ForegroundColor Gray
        } else {
            # Non-Windows platforms: skip CMake check
            $RunCMakeCheck = $false
            Write-Host "Skipping CMake check (not using Visual Studio generator on this platform)." -ForegroundColor Gray
            Write-Host "✓ CMake check skipped" -ForegroundColor Green
        }
    } else {
        # Check if generator is Visual Studio
        if ($Generator -match "Visual Studio") {
            $GeneratorArgs = @("-A", "x64")
            Write-Host "Generating $Generator project files..." -ForegroundColor Gray
        } else {
            # Non-VS generator: skip CMake check
            $RunCMakeCheck = $false
            Write-Host "Skipping CMake check (generator '$Generator' is not Visual Studio)." -ForegroundColor Gray
            Write-Host "✓ CMake check skipped" -ForegroundColor Green
        }
    }
    
    if ($RunCMakeCheck) {
        try {
            # Run CMake
            $cmakeArgs = @("-G", $Generator) + $GeneratorArgs + @("-DALLOW_SAME_DIRECTORY=ON", ".")
            & cmake $cmakeArgs 2>&1 | Out-Null
            
            # Check for changes using git status (doesn't modify staging area)
            $status = git status --porcelain
            
            if ($status) {
                Write-Host "✗ CMake generation created or modified files:" -ForegroundColor Red
                $status | ForEach-Object { 
                    $file = $_.Substring(3)  # Remove status prefix (e.g., " M ", "??")
                    Write-Host "  - $file" -ForegroundColor Red
                }
                $failed_checks += "CMake generation"
            } else {
                Write-Host "✓ CMake generation check passed" -ForegroundColor Green
            }
        } catch {
            Write-Host "✗ CMake generation failed: $_" -ForegroundColor Red
            $failed_checks += "CMake generation"
        }
    }
    Write-Host ""
}

# Step 2: clang-format Check/Fix
if (-not $SkipFormat) {
    if ($FixFormat) {
        Write-Host "[2/3] Fixing code formatting..." -ForegroundColor Yellow
        Write-Host "Running clang-format on source files in src/ directory only..." -ForegroundColor Gray
    } else {
        Write-Host "[2/3] Checking code formatting..." -ForegroundColor Yellow
        Write-Host "Running clang-format on source files in src/ directory only..." -ForegroundColor Gray
    }
    
    try {
        # Check if clang-format is available
        $null = Get-Command clang-format -ErrorAction Stop
        
        # Run clang-format only on files in src/ directory
        $files = Get-ChildItem -Path src -Include *.cpp,*.hpp,*.c,*.h -Recurse
        $file_count = $files.Count
        Write-Host "Found $file_count source files in src/ to check..." -ForegroundColor Gray
        
        foreach ($file in $files) {
            & clang-format -i $file.FullName
        }
        
        # Check for changes using git status (doesn't modify staging area) - filter to only src/ directory
        $status = git status --porcelain | Where-Object { $_ -match '\ssrc/' }
        
        if ($status) {
            $diff = $status | ForEach-Object { $_.Substring(3) }  # Remove status prefix
            if ($FixFormat) {
                Write-Host "✓ Fixed formatting in $($diff.Count) file(s):" -ForegroundColor Green
                $diff | ForEach-Object { Write-Host "  - $_" -ForegroundColor Green }
                Write-Host ""
                Write-Host "Files have been formatted. Review and commit the changes." -ForegroundColor Cyan
            } else {
                Write-Host "✗ Code formatting issues detected in:" -ForegroundColor Red
                $diff | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
                Write-Host "Run './tools/code-quality-check.ps1 -FixFormat' to fix formatting." -ForegroundColor Yellow
                $failed_checks += "Code formatting"
            }
        } else {
            Write-Host "✓ Code formatting check passed" -ForegroundColor Green
        }
    } catch {
        Write-Host "✗ clang-format not found. Please install LLVM/Clang or add it to PATH." -ForegroundColor Red
        Write-Host "See tools/README_CODE_QUALITY.md for installation instructions." -ForegroundColor Yellow
        $failed_checks += "Code formatting (tool not found)"
    }
    Write-Host ""
}

# Step 3: clang-tidy Check (only for src/ directory)
if (-not $SkipTidy -and -not $FixFormat) {
    Write-Host "[3/3] Running clang-tidy analysis on src/ directory only..." -ForegroundColor Yellow
    
    try {
        # Check if clang-tidy is available
        $null = Get-Command clang-tidy -ErrorAction Stop
        
        # Get changed files - ONLY from src/ directory
        $changed = @()
        $git_failed = $false
        
        if ($BaseCommit -and $HeadCommit) {
            # CI mode: use explicit commits
            Write-Host "Finding changed files in src/ between $BaseCommit and $HeadCommit..." -ForegroundColor Gray
            try {
                $changed = git diff --name-only --diff-filter=d "$BaseCommit...$HeadCommit" 2>&1 | Where-Object { $_ -match '\.(cpp|hpp|c|h)$' -and $_ -match '^src/' }
                if ($LASTEXITCODE -ne 0) { $git_failed = $true }
            } catch {
                $git_failed = $true
            }
        } else {
            # Local mode: compare with base branch
            Write-Host "Finding changed files in src/ compared to origin/$BaseBranch..." -ForegroundColor Gray
            try {
                $changed = git diff --name-only --diff-filter=d "origin/$BaseBranch...HEAD" 2>&1 | Where-Object { $_ -match '\.(cpp|hpp|c|h)$' -and $_ -match '^src/' }
                if ($LASTEXITCODE -ne 0) { $git_failed = $true }
            } catch {
                $git_failed = $true
            }
        }
        
        if ($git_failed) {
            Write-Host "⚠ Unable to determine changed files (base branch may not exist). Skipping clang-tidy." -ForegroundColor Yellow
            Write-Host "✓ clang-tidy check skipped (nothing to compare against)" -ForegroundColor Green
        } elseif (-not $changed) {
            Write-Host "No C/C++ source files changed in src/ directory. Skipping clang-tidy." -ForegroundColor Gray
            Write-Host "✓ clang-tidy check passed (no files to check)" -ForegroundColor Green
        } else {
            Write-Host "Analyzing $($changed.Count) changed file(s)..." -ForegroundColor Gray
            
            # Generate compile_commands.json if it doesn't exist
            if (-not (Test-Path "compile_commands.json")) {
                Write-Host "Generating compile_commands.json..." -ForegroundColor Gray
                cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DALLOW_SAME_DIRECTORY=ON . 2>&1 | Out-Null
            }
            
            $errors = @()
            foreach ($file in $changed) {
                $full_path = Join-Path $root_dir $file
                if (Test-Path $full_path) {
                    Write-Host "  Checking: $file" -ForegroundColor Gray
                    $output = & clang-tidy -p . $full_path 2>&1
                    $exit_code = $LASTEXITCODE
                    
                    if ($exit_code -ne 0) {
                        Write-Host "    Issues found!" -ForegroundColor Red
                        Write-Host $output
                        $errors += $file
                    }
                }
            }
            
            if ($errors.Count -gt 0) {
                Write-Host "✗ clang-tidy found issues in:" -ForegroundColor Red
                $errors | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
                $failed_checks += "clang-tidy analysis"
            } else {
                Write-Host "✓ clang-tidy analysis passed" -ForegroundColor Green
            }
        }
    } catch {
        Write-Host "✗ clang-tidy not found. Please install LLVM/Clang or add it to PATH." -ForegroundColor Red
        Write-Host "See tools/README_CODE_QUALITY.md for installation instructions." -ForegroundColor Yellow
        $failed_checks += "clang-tidy (tool not found)"
    }
    Write-Host ""
}

# Summary
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan

if ($failed_checks.Count -eq 0) {
    Write-Host "✓ All code quality checks passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "✗ Failed checks:" -ForegroundColor Red
    $failed_checks | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
    exit 1
}
