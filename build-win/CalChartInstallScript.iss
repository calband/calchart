; CalChart Installer script
; Tested with InnoSetup 5.4.3

[Setup]
AppName=CalChart
AppVersion=3.4.2
DefaultDirName={pf}\CalChart
DefaultGroupName=CalChart
UninstallDisplayIcon={app}\CalChart.exe
UninstallDisplayName=Uninstall CalChart
OutputDir=InstallerFiles

[Files]
Source: "CalChart\Release\CalChart.exe"; DestDir: "{app}"
Source: "..\docs\*"; DestDir: "{app}\docs"
Source: "..\resources\image\*"; DestDir: "{app}\image"
Source: "README.txt"; DestDir: "{app}"; Flags: isreadme
Source: "vcredist_x86.exe"; DestDir: "{app}\bin";

[Run]
Filename: "{app}\bin\vcredist_x86.exe"; WorkingDir: "{app}\bin"; StatusMsg: "Installing C-RunTime.  Windows may ask you to install or repair the runtime files needed to run CalChart on your system."

[Icons]
Name: "{commonprograms}\CalChart"; Filename: "{app}\CalChart.exe"
Name: "{commondesktop}\CalChart"; Filename: "{app}\CalChart.exe"

