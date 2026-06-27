@echo off

set prevcd=%cd%
cd %~dp0..\..\..\..\

set PROJECT=CCR\CCR.uproject
set TESTS=RecallVoxel
set OUTPUT=%cd%\CCR\Reports\RecallVoxel
set LOG=%OUTPUT%\UnitTests.log
set ERRORS=%OUTPUT%\UnitTests.err

rem Plugins unrelated to the RecallVoxel SDF test pipeline.
set DISABLE=-DisablePlugins=CommonLoadingScreen,CommonGame,CommonUser,Developer,EasyOnline,ExtendedCommonUI,UnrealEnginePatch,VariableCollection

if exist "%OUTPUT%" (
	echo Deleting "%OUTPUT%" ...
	rmdir /S /Q "%OUTPUT%"
)

mkdir "%OUTPUT%"

echo Building project...
call "Engine\Build\BatchFiles\Build.bat" CCREditor Win64 Development -Project="%cd%\%PROJECT%" -TargetType=Editor
if "%errorlevel%" neq "0" (
	echo Build failed. Aborting tests.
	cd /d "%prevcd%"
	exit /b 1
)

echo Running %TESTS% ...
call "Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -ExecCmds="Automation RunTests %TESTS%; Quit" -ReportExportPath="%OUTPUT%" -unattended -nopause -NullRHI -NoLoadingScreen -nosplash -STDOUT -TestExit="Automation Test Queue Empty" %DISABLE% > "%LOG%" 2>&1
set ec=%errorlevel%

type "%LOG%"
cd /d "%prevcd%"

rem Filter out known project-level startup warnings unrelated to our tests.
findstr /C:" Error:" "%LOG%" | findstr /V /C:"MassSpawner" /C:"GameFeatureData" /C:"Asset Manager settings" > "%ERRORS%" 2>&1
if "%errorlevel%" equ "0" (
	echo.
	echo =[ ERRORS DETECTED ]============================================================ >&2
	type "%ERRORS%" >&2
	if "%ec%" equ "0" set ec=1
)

exit /b %ec%
