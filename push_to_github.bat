@echo off
echo ========================================================
echo        Pushing Operating Systems Assignment to GitHub
echo ========================================================

REM Check if git is installed
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: Git is not installed or not in PATH!
    echo Please install Git from https://git-scm.com/
    pause
    exit /b 1
)

REM Initialize git repository if not already initialized
if not exist ".git" (
    echo Initializing Git repository...
    git init
    git branch -M main
)

REM Stage all files
echo Staging files...
git add .

REM Commit changes
echo Committing changes...
git commit -m "feat: complete xv6 assignment 7 (Threads, Semaphores, Deadlock, Priority Scheduling)"

REM Prompt for GitHub remote URL if not set
git remote get-url origin >nul 2>nul
if %errorlevel% neq 0 (
    echo.
    echo No GitHub remote 'origin' found.
    set /p REPO_URL="Enter your GitHub Repository URL (e.g. https://github.com/username/repo-name.git): "
    if not "%REPO_URL%"=="" (
        git remote add origin %REPO_URL%
    ) else (
        echo No repository URL provided. Aborting remote push.
        pause
        exit /b 1
    )
)

REM Push to GitHub
echo Pushing to GitHub (main branch)...
git push -u origin main

if %errorlevel% equ 0 (
    echo.
    echo ========================================================
    echo  Successfully pushed to GitHub!
    echo ========================================================
) else (
    echo.
    echo Note: If your remote repository already has files (like a README or LICENSE),
    echo you may need to run: git pull origin main --rebase
    echo and then run this script again.
)

pause
