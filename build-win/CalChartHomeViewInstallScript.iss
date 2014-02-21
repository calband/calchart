; CalChart Installer script
; Tested with InnoSetup 5.4.3

[Setup]
AppName=CalChartHomeView
AppVersion=3.4.0
DefaultDirName={pf}\CalChartHomeView
DefaultGroupName=CalChartHomeView
UninstallDisplayIcon={app}\CalChartHomeView.exe
UninstallDisplayName=Uninstall CalChartHomeView
OutputDir=InstallerFiles

[Files]
Source: "CalChartHomeView\Release\CalChartHomeView.exe"; DestDir: "{app}"
Source: "..\docs\*"; DestDir: "{app}\docs"
Source: "..\resources\image\*"; DestDir: "{app}\image"
Source: "README.txt"; DestDir: "{app}"; Flags: isreadme
Source: "vcredist_x86.exe"; DestDir: "{app}\bin";

[Run]
Filename: "{app}\bin\vcredist_x86.exe"; Parameters: "/q:a/c:""VCREDI~3.EXE /q:a /c:""""msi exec /i vcredist.msi /qn"""" """; WorkingDir: "{app}\bin"; StatusMsg: "Installing C-RunTime.  Windows may ask you to install or repair the runtime files needed to run CalChartHomeView on your system."

[Icons]
Name: "{commonprograms}\CalChartHomeView"; Filename: "{app}\CalChartHomeView.exe"
Name: "{commondesktop}\CalChartHomeView"; Filename: "{app}\CalChartHomeView.exe"

