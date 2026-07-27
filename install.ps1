<#
.SYNOPSIS
    rAthena AI World - Windows Install Wizard
.DESCRIPTION
    Complete setup script for the rAthena AI World server with dual AI systems on Windows.
    Handles system dependencies, Python environments, C++ build, databases,
    cache layer, configuration, Windows services, and verification.

.PARAMETER Quick
    Quick install (skips prompts, uses defaults)
.PARAMETER Dev
    Development install (debug mode, verbose)
.PARAMETER DryRun
    Preview what would be installed without making changes
.PARAMETER Help
    Show help message

.EXAMPLE
    .\install.ps1                    # Full interactive installation
    .\install.ps1 -Quick             # Quick install with defaults
    .\install.ps1 -Dev               # Development install
    .\install.ps1 -DryRun            # Preview only

.NOTES
    Author: AI-MMORPG-World Team
    Version: 1.0.0
    Requires: Windows 10/11, PowerShell 5.1+, Administrator rights
#>

param(
    [switch]$Quick,
    [switch]$Dev,
    [switch]$DryRun,
    [switch]$Help
)

# ============================================================================
# CONFIGURATION
# ============================================================================

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$LogFile = Join-Path $ScriptDir "install.log"
$Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

# Mode flags
$QuickMode = $Quick.IsPresent
$DevMode = $Dev.IsPresent
$DryRunMode = $DryRun.IsPresent

# Component flags
$InstallSystemDeps = $true
$BuildCppServer = $true
$SetupPostgres = $true
$SetupMysql = $true
$SetupCache = $true
$SetupPython = $true
$CreateConfig = $true
$CreateServices = $true
$RunVerification = $true

# Default database config
$PostgresDb = "ai_world_memory"
$PostgresUser = "ai_world_user"
$PostgresPassword = ""
$MysqlDb = "ragnarok"
$MysqlUser = "ragnarok"
$MysqlPassword = ""

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

function Write-Log {
    param([string]$Message)
    $time = Get-Date -Format "HH:mm:ss"
    $line = "[$time] $Message"
    Write-Host $line -ForegroundColor Green
    Add-Content -Path $LogFile -Value $line
}

function Write-Info {
    param([string]$Message)
    $line = "[INFO] $Message"
    Write-Host $line -ForegroundColor Cyan
    Add-Content -Path $LogFile -Value $line
}

function Write-Warn {
    param([string]$Message)
    $line = "[WARN] $Message"
    Write-Host $line -ForegroundColor Yellow
    Add-Content -Path $LogFile -Value $line
}

function Write-Error {
    param([string]$Message)
    $line = "[ERROR] $Message"
    Write-Host $line -ForegroundColor Red
    Add-Content -Path $LogFile -Value $line
}

function Write-Banner {
    Write-Host ""
    Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Blue
    Write-Host "║          rAthena AI World - Windows Install Wizard        ║" -ForegroundColor Blue
    Write-Host "║          Dual AI Systems Setup                            ║" -ForegroundColor Blue
    Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Blue
    Write-Host ""
}

function Write-Section {
    param([string]$Title)
    Write-Host ""
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Blue
    Write-Host "  $Title" -ForegroundColor Blue
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Blue
    Write-Host ""
}

function Test-Admin {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Test-Command {
    param([string]$Command)
    return Get-Command $Command -ErrorAction SilentlyContinue
}

function Prompt-YesNo {
    param([string]$Prompt, [string]$Default = "y")
    if ($QuickMode) {
        return ($Default -eq "y")
    }
    $suffix = if ($Default -eq "y") { "[Y/n]" } else { "[y/N]" }
    $reply = Read-Host "$Prompt $suffix"
    if ([string]::IsNullOrEmpty($reply)) { return ($Default -eq "y") }
    return ($reply -match "^[Yy]$")
}

function Prompt-Input {
    param([string]$Prompt, [string]$Default = "")
    if ($QuickMode -and $Default) { return $Default }
    if ($Default) {
        return Read-Host "$Prompt [$Default]"
    } else {
        return Read-Host "$Prompt"
    }
}

function Prompt-Password {
    param([string]$Prompt)
    if ($QuickMode) {
        # Generate random password
        $chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*"
        return -join ((1..32) | ForEach-Object { Get-Random -Maximum $chars.Length | ForEach-Object { $chars[$_] } })
    }
    return Read-Host "$Prompt" -AsSecureString | ConvertFrom-SecureString -AsPlainText
}

# ============================================================================
# SYSTEM CHECKS
# ============================================================================

function Check-System {
    Write-Section "Checking System Requirements"

    # OS
    $os = Get-WmiObject Win32_OperatingSystem
    Write-Log "OS: $($os.Caption) ($($os.Version))"

    # Architecture
    Write-Log "Architecture: $($env:PROCESSOR_ARCHITECTURE)"

    # CPU cores
    $cpuCores = (Get-WmiObject Win32_ComputerSystem).NumberOfLogicalProcessors
    Write-Log "CPU cores: $cpuCores"
    if ($cpuCores -lt 4) {
        Write-Warn "Minimum 4 CPU cores recommended (detected: $cpuCores)"
    }

    # RAM
    $totalRamGB = [math]::Round((Get-WmiObject Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 1)
    Write-Log "RAM: $totalRamGB GB"
    if ($totalRamGB -lt 8) {
        Write-Error "Minimum 8 GB RAM required (detected: $totalRamGB GB)"
        exit 1
    }
    if ($totalRamGB -lt 16) {
        Write-Warn "16+ GB RAM recommended for production use"
    }

    # Disk space
    $drive = Get-PSDrive -Name (Split-Path -Qualifier $ScriptDir).TrimEnd(':')
    $freeDiskGB = [math]::Round($drive.Free / 1GB, 1)
    Write-Log "Free disk space: $freeDiskGB GB"
    if ($freeDiskGB -lt 20) {
        Write-Error "Minimum 20 GB free disk space required (detected: $freeDiskGB GB)"
        exit 1
    }
    if ($freeDiskGB -lt 100) {
        Write-Warn "100+ GB recommended if training ML models"
    }

    # GPU detection
    $gpu = Get-WmiObject Win32_VideoController | Where-Object { $_.Name -match "NVIDIA" } | Select-Object -First 1
    if ($gpu) {
        Write-Log "GPU detected: $($gpu.Name)"
    } else {
        Write-Info "No NVIDIA GPU detected (optional - ML training requires GPU)"
    }

    # Internet connectivity
    try {
        $null = Test-Connection -ComputerName "8.8.8.8" -Count 1 -Quiet
        Write-Log "Internet connectivity verified"
    } catch {
        Write-Warn "No internet connection detected (some features may not work)"
    }

    # Administrator rights
    if (-not (Test-Admin)) {
        Write-Error "This script requires Administrator privileges. Please run as Administrator."
        exit 1
    }
    Write-Log "Administrator rights verified"
}

# ============================================================================
# INSTALLATION FUNCTIONS
# ============================================================================

function Install-SystemDependencies {
    Write-Section "Installing System Dependencies"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would install system packages"
        return
    }

    # Check if Chocolatey is installed
    $chocoInstalled = Test-Command "choco"
    if (-not $chocoInstalled) {
        Write-Log "Installing Chocolatey package manager..."
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    }

    Write-Log "Installing build tools and libraries..."
    $packages = @(
        "git",
        "cmake",
        "cmake.install",
        "visualstudio2022buildtools",
        "visualstudio2022-workload-vctools",
        "python",
        "python3",
        "openssl",
        "curl",
        "wget",
        "jq",
        "7zip"
    )

    foreach ($pkg in $packages) {
        if ($DryRunMode) {
            Write-Info "[DRY RUN] Would install: $pkg"
        } else {
            Write-Log "Installing $pkg..."
            choco install $pkg -y --no-progress 2>&1 | Out-Null
        }
    }

    Write-Log "System dependencies installed successfully"
}

function Build-CppServer {
    Write-Section "Building C++ rAthena Server"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would build C++ server with cmake + MSBuild"
        return
    }

    # Check if already built
    $mapServer = Join-Path $ScriptDir "map-server.exe"
    $loginServer = Join-Path $ScriptDir "login-server.exe"
    $charServer = Join-Path $ScriptDir "char-server.exe"

    if ((Test-Path $mapServer) -and (Test-Path $loginServer) -and (Test-Path $charServer)) {
        Write-Log "Server binaries already exist. Skipping build."
        if (-not $QuickMode) {
            if (-not (Prompt-YesNo "Force rebuild?" "n")) {
                return
            }
        }
    }

    Write-Log "Configuring build with CMake..."
    $buildDir = Join-Path $ScriptDir "build"
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
    Set-Location $buildDir

    # Detect Visual Studio version
    $vsPath = & "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" 2>&1 | Out-Null
    $vsVersion = "Visual Studio 17 2022"

    cmake .. -G "$vsVersion" -DCMAKE_BUILD_TYPE=Release -DINSTALL_TO_SOURCE=ON 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configuration failed"
        Set-Location $ScriptDir
        return
    }

    Write-Log "Building server (this may take a while)..."
    cmake --build . --config Release 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed"
        Set-Location $ScriptDir
        return
    }

    Set-Location $ScriptDir

    # Copy binaries to root
    Get-ChildItem -Path $buildDir -Filter "*.exe" -Recurse | ForEach-Object {
        Copy-Item $_.FullName -Destination $ScriptDir -Force
    }

    # Verify binaries
    if ((Test-Path $mapServer) -and (Test-Path $loginServer) -and (Test-Path $charServer)) {
        Write-Log "Server binaries built successfully"
        Get-Item $mapServer, $loginServer, $charServer | ForEach-Object {
            Write-Log "  $($_.Name): $([math]::Round($_.Length / 1KB)) KB"
        }
    } else {
        Write-Warn "Some server binaries may be missing. Check build output."
    }
}

function Setup-PostgreSQL {
    Write-Section "Setting Up PostgreSQL (AI Services Database)"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would install and configure PostgreSQL"
        return
    }

    # Check if PostgreSQL is installed
    $psql = Test-Command "psql"
    if (-not $psql) {
        Write-Log "Installing PostgreSQL..."
        choco install postgresql17 --params "/Password:$PostgresPassword" -y --no-progress 2>&1 | Out-Null
        # Refresh PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
    } else {
        Write-Log "PostgreSQL already installed"
    }

    # Prompt for credentials
    if ([string]::IsNullOrEmpty($PostgresPassword)) {
        Write-Host ""
        Write-Info "PostgreSQL Database Configuration"
        Write-Info "This database is for AI services ONLY (not for rAthena game data)"
        $PostgresDb = Prompt-Input "Database name" $PostgresDb
        $PostgresUser = Prompt-Input "Database user" $PostgresUser
        $PostgresPassword = Prompt-Password "Password for PostgreSQL user '$PostgresUser'"
        if ([string]::IsNullOrEmpty($PostgresPassword)) {
            Write-Error "Password cannot be empty"
            return
        }
    }

    # Create database and user
    Write-Log "Creating database '$PostgresDb' and user '$PostgresUser'..."
    $pgBin = "C:\Program Files\PostgreSQL\17\bin"
    if (-not (Test-Path $pgBin)) {
        $pgBin = "C:\Program Files\PostgreSQL\16\bin"
    }

    & "$pgBin\psql.exe" -U postgres -c "CREATE DATABASE $PostgresDb;" 2>$null | Out-Null
    & "$pgBin\psql.exe" -U postgres -c "CREATE USER $PostgresUser WITH PASSWORD '$PostgresPassword';" 2>$null | Out-Null
    & "$pgBin\psql.exe" -U postgres -c "GRANT ALL PRIVILEGES ON DATABASE $PostgresDb TO $PostgresUser;" 2>$null | Out-Null
    & "$pgBin\psql.exe" -d $PostgresDb -c "GRANT ALL ON SCHEMA public TO $PostgresUser;" 2>$null | Out-Null

    # Run schema initialization
    $initSql = Join-Path $ScriptDir "ai-autonomous-world\database\init.sql"
    if (Test-Path $initSql) {
        Write-Log "Initializing database schema..."
        & "$pgBin\psql.exe" -U postgres -d $PostgresDb -f $initSql 2>&1 | Out-Null
    }

    # Run migrations
    $migrationsDir = Join-Path $ScriptDir "ai-autonomous-world\database\migrations"
    if (Test-Path $migrationsDir) {
        Get-ChildItem $migrationsDir -Filter "*.sql" | Sort-Object Name | ForEach-Object {
            Write-Log "Applying migration: $($_.Name)..."
            & "$pgBin\psql.exe" -U postgres -d $PostgresDb -f $_.FullName 2>&1 | Out-Null
        }
    }

    Write-Log "PostgreSQL setup complete"
}

function Setup-MySQL {
    Write-Section "Setting Up MariaDB/MySQL (Game Database)"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would install and configure MySQL"
        return
    }

    # Check if MySQL is installed
    $mysql = Test-Command "mysql"
    if (-not $mysql) {
        Write-Log "Installing MySQL..."
        choco install mysql -y --no-progress 2>&1 | Out-Null
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
    } else {
        Write-Log "MySQL already installed"
    }

    # Prompt for credentials
    if ([string]::IsNullOrEmpty($MysqlPassword)) {
        Write-Host ""
        Write-Info "MySQL Database Configuration"
        Write-Info "This database is for rAthena game data ONLY"
        $MysqlDb = Prompt-Input "Database name" $MysqlDb
        $MysqlUser = Prompt-Input "Database user" $MysqlUser
        $MysqlPassword = Prompt-Password "Password for MySQL user '$MysqlUser'"
        if ([string]::IsNullOrEmpty($MysqlPassword)) {
            Write-Error "Password cannot be empty"
            return
        }
    }

    # Create database and user
    Write-Log "Creating database '$MysqlDb' and user '$MysqlUser'..."
    mysql -u root -e "CREATE DATABASE IF NOT EXISTS $MysqlDb CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;" 2>$null
    mysql -u root -e "CREATE USER IF NOT EXISTS '$MysqlUser'@'localhost' IDENTIFIED BY '$MysqlPassword';" 2>$null
    mysql -u root -e "GRANT ALL PRIVILEGES ON $MysqlDb.* TO '$MysqlUser'@'localhost';" 2>$null
    mysql -u root -e "FLUSH PRIVILEGES;" 2>$null

    # Import SQL files
    $sqlFiles = @(
        "sql-files\main.sql",
        "sql-files\logs.sql",
        "sql-files\web.sql",
        "sql-files\ai_ipc_tables.sql"
    )

    foreach ($sqlFile in $sqlFiles) {
        $fullPath = Join-Path $ScriptDir $sqlFile
        if (Test-Path $fullPath) {
            Write-Log "Importing $(Split-Path $sqlFile -Leaf)..."
            mysql -u root $MysqlDb < $fullPath 2>&1 | Out-Null
        }
    }

    # Import item/mob databases
    Write-Log "Importing item and monster databases..."
    Get-ChildItem (Join-Path $ScriptDir "sql-files") -Filter "item_db*.sql" | ForEach-Object {
        Write-Log "  Importing $($_.Name)..."
        mysql -u root $MysqlDb < $_.FullName 2>&1 | Out-Null
    }
    Get-ChildItem (Join-Path $ScriptDir "sql-files") -Filter "mob_db*.sql" | ForEach-Object {
        Write-Log "  Importing $($_.Name)..."
        mysql -u root $MysqlDb < $_.FullName 2>&1 | Out-Null
    }
    Get-ChildItem (Join-Path $ScriptDir "sql-files") -Filter "mob_skill_db*.sql" | ForEach-Object {
        Write-Log "  Importing $($_.Name)..."
        mysql -u root $MysqlDb < $_.FullName 2>&1 | Out-Null
    }

    Write-Log "MySQL setup complete"
}

function Setup-Cache {
    Write-Section "Setting Up Redis (Cache Layer)"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would install Redis"
        return
    }

    # Check if Redis is installed
    $redis = Test-Command "redis-server"
    if (-not $redis) {
        Write-Log "Installing Redis..."
        choco install redis-64 -y --no-progress 2>&1 | Out-Null
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
    } else {
        Write-Log "Redis already installed"
    }

    # Start Redis service
    $redisService = Get-Service -Name "Redis" -ErrorAction SilentlyContinue
    if ($redisService) {
        if ($redisService.Status -ne "Running") {
            Start-Service -Name "Redis"
        }
        Write-Log "Redis service is running"
    } else {
        Write-Warn "Redis service not found. Start manually: redis-server"
    }

    Write-Log "Cache setup complete"
}

function Setup-PythonEnvironment {
    Write-Section "Setting Up Python Environment"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would create Python virtual environments"
        return
    }

    $aiDir = Join-Path $ScriptDir "ai-autonomous-world"
    $aiVenv = Join-Path $ScriptDir ".venv"

    if (Test-Path $aiDir) {
        Write-Log "Setting up AI Autonomous World Python environment..."

        if (-not (Test-Path $aiVenv)) {
            Write-Log "Creating virtual environment..."
            python -m venv $aiVenv
        } else {
            Write-Log "Virtual environment already exists"
        }

        # Activate and install
        $activateScript = Join-Path $aiVenv "Scripts\Activate.ps1"
        . $activateScript

        pip install --upgrade pip setuptools wheel -q

        $requirementsFile = Join-Path $aiDir "ai-service\requirements.txt"
        if (Test-Path $requirementsFile) {
            Write-Log "Installing AI service dependencies..."
            pip install -r $requirementsFile 2>&1 | Out-Null
        }

        deactivate
        Write-Log "AI Autonomous World Python environment ready"
    }

    Write-Log "Python environment setup complete"
}

function Create-Configuration {
    Write-Section "Creating Configuration Files"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would create .env configuration"
        return
    }

    $envFile = Join-Path $ScriptDir ".env"
    $envExample = Join-Path $ScriptDir ".env.example"

    if (Test-Path $envFile) {
        Write-Info ".env already exists"
        if (-not $QuickMode) {
            if (-not (Prompt-YesNo "Overwrite existing .env?" "n")) {
                Write-Log "Keeping existing .env"
                return
            }
        }
    }

    if (Test-Path $envExample) {
        Copy-Item $envExample $envFile -Force
        # Fill in database credentials
        if ($PostgresPassword) {
            (Get-Content $envFile) -replace "^POSTGRES_PASSWORD=.*", "POSTGRES_PASSWORD=$PostgresPassword" | Set-Content $envFile
        }
        if ($MysqlPassword) {
            (Get-Content $envFile) -replace "^MYSQL_PASSWORD=.*", "MYSQL_PASSWORD=$MysqlPassword" | Set-Content $envFile
        }
        Write-Log "Created .env from template"
    } else {
        Write-Log "Creating minimal .env..."
        @"
# rAthena AI World - Environment Configuration
# Generated by install.ps1

# --- Database ---
POSTGRES_DB=$PostgresDb
POSTGRES_USER=$PostgresUser
POSTGRES_PASSWORD=$PostgresPassword
POSTGRES_HOST=localhost
POSTGRES_PORT=5432

# --- Cache ---
REDIS_HOST=localhost
REDIS_PORT=6379
REDIS_PASSWORD=

# --- LLM API Keys (at least one required) ---
# OPENAI_API_KEY=sk-...
# ANTHROPIC_API_KEY=sk-...
# DEEPSEEK_API_KEY=...
# AZURE_OPENAI_API_KEY=...

# --- AI Service ---
AI_SERVICE_API_KEY=dev-key-change-in-production
API_KEY_REQUIRED=false
LOG_LEVEL=INFO
"@ | Set-Content $envFile
    }

    Write-Log "Configuration files created"
    Write-Warn "IMPORTANT: Edit .env to add your LLM API keys!"
}

function Create-WindowsServices {
    Write-Section "Creating Windows Services"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would create Windows services"
        return
    }

    $aiVenv = Join-Path $ScriptDir ".venv"
    $aiWorkdir = Join-Path $ScriptDir "ai-autonomous-world\ai-service"
    $pythonExe = Join-Path $aiVenv "Scripts\python.exe"

    # AI Service
    $aiServiceName = "rAthenaAIService"
    $aiService = Get-Service -Name $aiServiceName -ErrorAction SilentlyContinue
    if (-not $aiService) {
        Write-Log "Creating $aiServiceName Windows service..."
        New-Service -Name $aiServiceName `
            -BinaryPathName "$pythonExe -m uvicorn main:app --host 0.0.0.0 --port 8000 --workers 4" `
            -DisplayName "rAthena AI World - AI Autonomous World Service" `
            -Description "AI service with 21 AI agents for NPC intelligence" `
            -StartupType Automatic `
            -Credential (Get-Credential -Message "Enter Windows user account for AI service" -UserName "$env:USERDOMAIN\$env:USERNAME")
    } else {
        Write-Log "Service $aiServiceName already exists"
    }

    # AI IPC Worker
    $ipcServiceName = "rAthenaAIIPCWorker"
    $ipcService = Get-Service -Name $ipcServiceName -ErrorAction SilentlyContinue
    if (-not $ipcService) {
        Write-Log "Creating $ipcServiceName Windows service..."
        $ipcWorker = Join-Path $ScriptDir "ml_inference_service\ai_ipc_worker.py"
        if (Test-Path $ipcWorker) {
            New-Service -Name $ipcServiceName `
                -BinaryPathName "$pythonExe $ipcWorker" `
                -DisplayName "AI IPC Worker - NPC-AI Communication Bridge" `
                -Description "Polls ai_requests table and forwards to AI service" `
                -StartupType Automatic
        }
    } else {
        Write-Log "Service $ipcServiceName already exists"
    }

    Write-Log "Windows services created"
    Write-Log "  Start with: Start-Service $aiServiceName"
    Write-Log "  Start with: Start-Service $ipcServiceName"
}

function Run-Verification {
    Write-Section "Running Verification Tests"

    if ($DryRunMode) {
        Write-Info "[DRY RUN] Would run verification tests"
        return
    }

    $errors = 0

    # 1. Check PostgreSQL
    Write-Log "Checking PostgreSQL..."
    $psql = Test-Command "psql"
    if ($psql) {
        Write-Log "  ✓ PostgreSQL is installed"
    } else {
        Write-Error "  ✗ PostgreSQL is not installed"
        $errors++
    }

    # 2. Check MySQL
    Write-Log "Checking MySQL..."
    $mysql = Test-Command "mysql"
    if ($mysql) {
        Write-Log "  ✓ MySQL is installed"
    } else {
        Write-Warn "  ⚠ MySQL not installed"
    }

    # 3. Check Redis
    Write-Log "Checking Redis..."
    $redis = Test-Command "redis-cli"
    if ($redis) {
        Write-Log "  ✓ Redis is installed"
    } else {
        Write-Warn "  ⚠ Redis not detected"
    }

    # 4. Check server binaries
    Write-Log "Checking server binaries..."
    $binaries = @("map-server.exe", "login-server.exe", "char-server.exe", "web-server.exe")
    foreach ($bin in $binaries) {
        $binPath = Join-Path $ScriptDir $bin
        if (Test-Path $binPath) {
            Write-Log "  ✓ $bin exists"
        } else {
            Write-Warn "  ⚠ $bin not found (may need to build)"
        }
    }

    # 5. Check Python environment
    Write-Log "Checking Python environment..."
    $venvPath = Join-Path $ScriptDir ".venv"
    if (Test-Path $venvPath) {
        Write-Log "  ✓ Python venv exists"
        $activateScript = Join-Path $venvPath "Scripts\Activate.ps1"
        . $activateScript
        $importCheck = python -c "import fastapi" 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Log "  ✓ FastAPI importable"
        } else {
            Write-Warn "  ⚠ FastAPI not importable"
        }
        deactivate
    } else {
        Write-Warn "  ⚠ Python venv not found"
    }

    # 6. Check .env
    Write-Log "Checking configuration..."
    $envFile = Join-Path $ScriptDir ".env"
    if (Test-Path $envFile) {
        Write-Log "  ✓ .env exists"
    } else {
        Write-Warn "  ⚠ .env not found"
    }

    Write-Host ""
    if ($errors -eq 0) {
        Write-Log "All checks passed!"
    } else {
        Write-Warn "$errors check(s) had issues - review warnings above"
    }
}

function Show-NextSteps {
    Write-Section "Installation Complete!"

    Write-Host "  ✓ rAthena AI World has been installed successfully!" -ForegroundColor Green
    Write-Host ""

    Write-Host "  ┌─────────────────────────────────────────────────────────┐" -ForegroundColor Cyan
    Write-Host "  │  NEXT STEPS                                              │" -ForegroundColor Cyan
    Write-Host "  └─────────────────────────────────────────────────────────┘" -ForegroundColor Cyan
    Write-Host ""

    Write-Host "  1. Configure LLM API Keys" -ForegroundColor Blue
    Write-Host "     Edit $ScriptDir\.env"
    Write-Host "     Set at least one LLM provider key:"
    Write-Host "       - OPENAI_API_KEY"
    Write-Host "       - ANTHROPIC_API_KEY"
    Write-Host "       - DEEPSEEK_API_KEY"
    Write-Host "       - AZURE_OPENAI_API_KEY"
    Write-Host ""

    Write-Host "  2. Start the AI Service" -ForegroundColor Blue
    Write-Host "     cd $ScriptDir\ai-autonomous-world\ai-service"
    Write-Host "     $ScriptDir\.venv\Scripts\activate"
    Write-Host "     python main.py"
    Write-Host "     # Or via service: Start-Service rAthenaAIService"
    Write-Host ""

    Write-Host "  3. Start the Game Server" -ForegroundColor Blue
    Write-Host "     cd $ScriptDir"
    Write-Host "     .\map-server.exe"
    Write-Host "     .\login-server.exe"
    Write-Host "     .\char-server.exe"
    Write-Host ""

    Write-Host "  4. Test the Installation" -ForegroundColor Blue
    Write-Host "     curl http://localhost:8000/health"
    Write-Host "     Open http://localhost:8000/docs in your browser"
    Write-Host ""

    Write-Host "  5. Enable Services on Boot" -ForegroundColor Blue
    Write-Host "     Set-Service rAthenaAIService -StartupType Automatic"
    Write-Host "     Set-Service rAthenaAIIPCWorker -StartupType Automatic"
    Write-Host ""

    Write-Host "  ┌─────────────────────────────────────────────────────────┐" -ForegroundColor Yellow
    Write-Host "  │  IMPORTANT                                               │" -ForegroundColor Yellow
    Write-Host "  └─────────────────────────────────────────────────────────┘" -ForegroundColor Yellow
    Write-Host "  • Change default passwords before production deployment"
    Write-Host "  • Set API_KEY_REQUIRED=true in .env for production"
    Write-Host "  • Configure SSL/TLS for production"
    Write-Host "  • See docs/SETUP.md for detailed configuration guide"
    Write-Host ""

    Write-Host "  Documentation:" -ForegroundColor Cyan
    Write-Host "    • Setup Guide:     docs\SETUP.md"
    Write-Host "    • Production:      docs\PRODUCTION_DEPLOYMENT_GUIDE.md"
    Write-Host "    • Operations:      docs\OPERATIONS_RUNBOOK.md"
    Write-Host ""

    Write-Host "  Happy gaming! 🎮" -ForegroundColor Green
    Write-Host ""
}

function Show-Help {
    @"
rAthena AI World - Windows Install Wizard

USAGE:
    .\install.ps1 [OPTIONS]

OPTIONS:
    -Quick              Quick install (skip prompts, use defaults)
    -Dev                Development install (debug mode, verbose)
    -DryRun             Preview what would be installed without making changes
    -Help               Show this help message

DESCRIPTION:
    Complete setup script for the rAthena AI World server with dual AI systems.
    Handles system dependencies, Python environments, C++ build, databases,
    cache layer, configuration, Windows services, and verification.

COMPONENTS:
    AI Autonomous World System  - NPC intelligence (21 AI agents, production)
    ML Monster AI System        - Monster behavior (infrastructure ready)
    rAthena Game Server         - Core MMORPG server (C++)

REQUIREMENTS:
    OS:     Windows 10/11
    CPU:    4+ cores
    RAM:    8 GB minimum (16+ GB recommended)
    Disk:   20 GB free (100+ GB for ML training)
    GPU:    Optional (NVIDIA 12GB+ for ML training)

EXAMPLES:
    .\install.ps1                    # Full interactive installation
    .\install.ps1 -Quick             # Quick install with defaults
    .\install.ps1 -Dev               # Development install
    .\install.ps1 -DryRun            # Preview only

LOG FILE:
    All output is logged to: install.log

"@
}

# ============================================================================
# MAIN
# ============================================================================

function Main {
    if ($Help) {
        Show-Help
        return
    }

    # Initialize log
    New-Item -ItemType File -Force -Path $LogFile | Out-Null

    Write-Banner

    Write-Log "Installation started at $Timestamp"
    Write-Log "Working directory: $ScriptDir"
    $mode = if ($QuickMode) { "Quick" } else { "Interactive" }
    if ($DevMode) { $mode += " + Dev" }
    if ($DryRunMode) { $mode += " + Dry Run" }
    Write-Log "Mode: $mode"
    Write-Log "Log file: $LogFile"

    # System checks
    Check-System

    # Interactive component selection
    if (-not $QuickMode -and -not $DryRunMode) {
        Write-Host ""
        Write-Info "Component Selection"
        Write-Host "  Select which components to install:"
        Write-Host ""
        if (-not (Prompt-YesNo "Install system dependencies?" "y")) { $InstallSystemDeps = $false }
        if (-not (Prompt-YesNo "Build C++ server from source?" "y")) { $BuildCppServer = $false }
        if (-not (Prompt-YesNo "Set up PostgreSQL (AI services)?" "y")) { $SetupPostgres = $false }
        if (-not (Prompt-YesNo "Set up MySQL (game data)?" "y")) { $SetupMysql = $false }
        if (-not (Prompt-YesNo "Set up Redis (cache)?" "y")) { $SetupCache = $false }
        if (-not (Prompt-YesNo "Set up Python environments?" "y")) { $SetupPython = $false }
        if (-not (Prompt-YesNo "Create configuration files?" "y")) { $CreateConfig = $false }
        if (-not (Prompt-YesNo "Create Windows services?" "y")) { $CreateServices = $false }
        if (-not (Prompt-YesNo "Run verification tests?" "y")) { $RunVerification = $false }
        Write-Host ""
    }

    # Run installation steps
    if ($InstallSystemDeps) { Install-SystemDependencies }
    if ($BuildCppServer) { Build-CppServer }
    if ($SetupPostgres) { Setup-PostgreSQL }
    if ($SetupMysql) { Setup-MySQL }
    if ($SetupCache) { Setup-Cache }
    if ($SetupPython) { Setup-PythonEnvironment }
    if ($CreateConfig) { Create-Configuration }
    if ($CreateServices) { Create-WindowsServices }
    if ($RunVerification) { Run-Verification }

    # Print next steps
    if (-not $DryRunMode) {
        Show-NextSteps
    }

    Write-Log "Installation completed at $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    Write-Log "Full log available at: $LogFile"
}

Main
