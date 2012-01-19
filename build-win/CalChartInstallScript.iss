; CalChart Installer script
; Tested with InnoSetup 5.4.3

[Setup]
AppName=CalChart
AppVersion=3.2.2
DefaultDirName={pf}\CalChart
DefaultGroupName=CalChart
UninstallDisplayIcon={app}\CalChart.exe
UninstallDisplayName=Uninstall CalChart
OutputDir=InstallerFiles

[Files]
Source: "CalChart\Release\CalChart.exe"; DestDir: "{app}"
Source: "..\docs\*"; DestDir: "{app}\docs"
Source: "README.txt"; DestDir: "{app}"; Flags: isreadme
