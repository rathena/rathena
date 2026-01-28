# Code Quality Tools

This directory contains scripts to run code quality checks locally, matching the GitHub Actions pipeline.

## Prerequisites

### Required Tools

1. **CMake** - For generating project files
   - Windows: Download from https://cmake.org/download/
   - macOS: `brew install cmake`
   - Linux: `sudo apt-get install cmake` or `sudo yum install cmake`

2. **LLVM/Clang Tools** - For clang-format and clang-tidy
   - **Windows**: Download from https://releases.llvm.org/ and add to PATH
   - **macOS**: 
     ```bash
     brew install llvm
     # Add to PATH (add to ~/.zshrc or ~/.bash_profile):
     export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
     # Or for Intel Macs:
     export PATH="/usr/local/opt/llvm/bin:$PATH"
     ```
   - **Linux**: 
     ```bash
     sudo apt-get install clang-format clang-tidy
     # or
     sudo yum install clang-tools-extra
     ```

### Optional Tools

- **Git** - Required for diff-based checks (usually already installed)

## Usage

### Windows (PowerShell)

```powershell
# Run all checks
.\tools\code-quality-check.ps1

# Fix formatting issues automatically
.\tools\code-quality-check.ps1 -FixFormat

# Run specific checks
.\tools\code-quality-check.ps1 -SkipCMake        # Skip CMake check
.\tools\code-quality-check.ps1 -SkipFormat       # Skip format check
.\tools\code-quality-check.ps1 -SkipTidy         # Skip clang-tidy check

# Compare against different branch
.\tools\code-quality-check.ps1 -BaseBranch develop
```

### Linux/macOS (Bash)

```bash
# Run all checks
./tools/code-quality-check.sh

# Fix formatting issues automatically
./tools/code-quality-check.sh --fix-format

# Run specific checks
./tools/code-quality-check.sh --skip-cmake       # Skip CMake check
./tools/code-quality-check.sh --skip-format      # Skip format check
./tools/code-quality-check.sh --skip-tidy        # Skip clang-tidy check

# Compare against different branch
./tools/code-quality-check.sh --base-branch develop

# Show help
./tools/code-quality-check.sh --help
```

## What Each Check Does

### 1. CMake Generation Check
- **Only runs for Visual Studio generators** (Windows platform or when explicitly specified)
- Runs CMake to generate Visual Studio project files (.vcxproj, .sln)
- Fails if any project files are modified
- Ensures project files are up-to-date
- **Automatically skipped** on non-Windows platforms or when using non-Visual Studio generators (like Unix Makefiles)

### 2. Code Formatting Check (clang-format)
- Runs clang-format on **all source files in `src/` directory only**
- 3rdparty code and other directories are excluded
- Fails if any files need formatting
- Configuration: `.clang-format` in project root

### 3. Static Analysis Check (clang-tidy)
- Runs clang-tidy on **files changed in `src/` directory only** compared to base branch
- 3rdparty code and other directories are excluded
- Fails if new warnings or errors are found
- **Passes gracefully** if there's nothing to compare against (e.g., base branch doesn't exist)
- Configuration: `.clang-tidy` in project root

## Fixing Issues

### CMake Changes
If CMake generates changes to Visual Studio project files, commit them:
```bash
git add *.vcxproj *.sln
git commit -m "Update Visual Studio project files"
```

**Note:** The CMake check only runs for Visual Studio generators. On Linux/macOS, this check is automatically skipped since Unix Makefiles generate files not tracked in git.

### Formatting Issues

**Option 1: Use the built-in fix command (Recommended)**
```bash
# Windows
.\tools\code-quality-check.ps1 -FixFormat

# Linux/macOS
./tools/code-quality-check.sh --fix-format
```

This will:
- Automatically format all files in `src/` directory
- Show which files were changed
- Leave the changes staged for you to review and commit

**Option 2: Manual formatting**
```bash
# Windows
Get-ChildItem -Path src -Include *.cpp,*.hpp,*.c,*.h -Recurse | ForEach-Object { clang-format -i $_.FullName }

# Linux/macOS
find src -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" \) -exec clang-format -i {} \;
```

**Note:** Only files in the `src/` directory are checked and formatted. Third-party libraries in `3rdparty/` and other directories are excluded.

### clang-tidy Issues
Review the warnings/errors reported by clang-tidy and fix them manually. Common fixes:
- Add missing includes
- Fix implicit conversions
- Add const correctness
- Use modern C++ features
- Fix potential bugs

## Continuous Integration

These same checks run automatically on pull requests via GitHub Actions. The workflow file is located at `.github/workflows/code_quality.yml`.

**The CI pipeline uses the same scripts** (`code-quality-check.ps1` for Windows runners) to ensure consistency between local and CI checks. This means any changes to the check logic only need to be made in one place.

### Making CI Required

To require these checks to pass before merging:
1. Go to your repository settings on GitHub
2. Navigate to Branches → Branch protection rules
3. Add a rule for your main branch
4. Check "Require status checks to pass before merging"
5. Select "Code Quality Pipeline" from the list

## Troubleshooting

### "clang-format not found" or "clang-tidy not found"

**macOS (Homebrew):**
The script will automatically detect Homebrew's LLVM installation. If it still doesn't work, add LLVM to your PATH permanently:

```bash
# For Apple Silicon (M1/M2/M3):
echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc

# For Intel Macs:
echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

Verify installation:
```bash
which clang-format
which clang-tidy
```

**Windows:**
Add LLVM bin directory to PATH:
1. Search for "Environment Variables" in Windows
2. Edit "Path" system variable
3. Add LLVM bin directory (e.g., `C:\Program Files\LLVM\bin`)
4. Restart terminal/PowerShell

**Linux:**
Ensure the tools are installed:
```bash
# Ubuntu/Debian
sudo apt-get install clang-format clang-tidy

# RHEL/CentOS
sudo yum install clang-tools-extra
```

### "compile_commands.json not found"
- This file is generated automatically by the script
- If issues persist, manually generate: `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .`

### "Unable to determine changed files" or "git diff shows no base branch"
This is not a failure - the script will automatically skip clang-tidy and pass the check when:
- The base branch doesn't exist locally
- You haven't fetched from origin yet
- Running on a fresh clone

**To enable clang-tidy analysis:**
- Ensure you've fetched the latest changes: `git fetch origin`
- Specify correct base branch with `-BaseBranch` (PS) or `--base-branch` (Bash)
- Make sure the base branch exists: `git branch -r | grep origin/<branch-name>`

## Configuration Files

- `.clang-format` - Code formatting rules (LLVM-based with project customizations)
- `.clang-tidy` - Static analysis rules (diagnostics, performance, modernize, etc.)

You can customize these files to match your project's coding standards.
