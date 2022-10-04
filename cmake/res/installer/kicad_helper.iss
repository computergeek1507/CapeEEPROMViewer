#define MyTitleName "KicadHelper"
#define Version "0.0.1"

[Setup]
DisableDirPage=no   
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

AppName={#MyTitleName}
AppVersion={#Version}
DefaultDirName={commonpf64}\{#MyTitleName}
DefaultGroupName={#MyTitleName}
SetupIconFile=..\kicad.ico
ChangesAssociations = yes
UninstallDisplayIcon={app}\{#MyTitleName}.exe
Compression=lzma2
SolidCompression=yes
OutputDir=output
OutputBaseFilename={#MyTitleName}_{#Version}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "Do you want to create desktop icon?"; Flags: checkablealone

[Files]
Source: "..\..\cmake_vs\Release\{#MyTitleName}.exe"; DestDir: "{app}"
Source: "..\kicad.ico"; DestDir: "{app}"
Source: "..\..\cmake_vs\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\cmake_vs\Release\platforms\qwindows.dll"; DestDir: "{app}\platforms\"; Flags: ignoreversion  
Source: "..\..\cmake_vs\Release\imageformats\*.dll"; DestDir: "{app}\imageformats\"; Flags: ignoreversion
Source: "..\..\cmake_vs\Release\styles\*.dll"; DestDir: "{app}\styles\"; Flags: ignoreversion
Source: "..\..\cmake_vs\Release\translations\*.qm"; DestDir: "{app}\translations\"; Flags: ignoreversion

[Run]
Filename: "{app}\{#MyTitleName}.exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent 