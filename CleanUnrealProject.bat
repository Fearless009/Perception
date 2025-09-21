@echo off
setlocal

:: Define paths
set PROJECT_DIR=%~dp0
set CONFIG_DIR=%PROJECT_DIR%Saved\Config\WindowsEditor
set TEMP_DIR=%PROJECT_DIR%TempBackup

:: Ensure the TempBackup folder exists
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"

:: Backup EditorPerProjectUserSettings.ini
if exist "%CONFIG_DIR%\EditorPerProjectUserSettings.ini" (
    echo Backing up EditorPerProjectUserSettings.ini...
    move "%CONFIG_DIR%\EditorPerProjectUserSettings.ini" "%TEMP_DIR%"
)

:: Delete Binaries, Intermediate, and Saved folders
echo Deleting Binaries, Intermediate, and Saved folders...
rmdir /s /q "%PROJECT_DIR%Binaries"
rmdir /s /q "%PROJECT_DIR%Intermediate"
rmdir /s /q "%PROJECT_DIR%Saved"

:: Restore the Editor Preferences file
if exist "%TEMP_DIR%\EditorPerProjectUserSettings.ini" (
    echo Restoring EditorPerProjectUserSettings.ini...
    mkdir "%CONFIG_DIR%"
    move "%TEMP_DIR%\EditorPerProjectUserSettings.ini" "%CONFIG_DIR%"
)

:: Clean up temporary folder
rmdir /s /q "%TEMP_DIR%"

echo Done!
pause