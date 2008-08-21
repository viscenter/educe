; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=SCIRun
AppVerName=SCIRun 4.0
AppVersion=4.0
AppPublisher=SCI Institute
AppPublisherURL=http://software.sci.utah.edu
AppSupportURL=http://software.sci.utah.edu
AppUpdatesURL=http://software.sci.utah.edu
AppCopyright=Copyright (c) 2004 Scientific Computing and Imaging Institute, University of Utah.
DefaultDirName={pf}\SCIRun
DefaultGroupName=SCIRun
OutputBaseFilename=SCIRun_4.0
AllowNoIcons=yes
Compression=lzma
SolidCompression=yes
SourceDir=C:\Users\ayla\Documents\NCRR\cibc\SCIRun
LicenseFile=C:\Users\ayla\Documents\NCRR\cibc\SCIRun\src\LICENSE

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Dirs]
;Name: "{app}\bin"; Permissions: users-modify
;Name: "{app}\bin\on-the-fly-libs"; Permissions: users-modify
;Name: "{app}\lib"; Permissions: users-modify
;Name: "{app}\src"; Permissions: users-modify
Name: "{app}\bin";
Name: "{app}\bin\on-the-fly-libs";
Name: "{app}\lib";
Name: "{app}\src";
Name: "{userappdata}\SCIRun"; Flags: uninsalwaysuninstall

[Files]
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
; Visual C++ redistributable
Source: "C:\Users\ayla\Documents\vcredist_x86.exe"; DestDir: "{tmp}";

; SCIRun TCL, XML, net files etc. from source
; NOTE: including nets/Samples
Source: "src\Dataflow\GUI\*"; Excludes: ".svn"; DestDir: "{app}\src\Dataflow\GUI"; Flags: ignoreversion replacesameversion
Source: "src\Dataflow\XML\*"; Excludes: ".svn"; DestDir: "{app}\src\Dataflow\XML"; Flags: ignoreversion replacesameversion
Source: "src\scirunrc"; DestDir: "{app}\src"; Flags: ignoreversion replacesameversion
Source: "src\main\scisplash.ppm"; DestDir: "{app}\src\main"; Flags: ignoreversion replacesameversion
Source: "src\nets\*"; Excludes: ".svn"; DestDir: "{app}\src\nets"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "src\Subnets\*"; Excludes: ".svn"; DestDir: "{app}\src\Subnets"; Flags: ignoreversion replacesameversion
Source: "src\pixmaps\*"; Excludes: ".svn"; DestDir: "{app}\src\pixmaps"; Flags: ignoreversion replacesameversion

Source: "src\Packages\BioPSE\Dataflow\GUI\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\BioPSE\Dataflow\GUI"; Flags: ignoreversion replacesameversion
Source: "src\Packages\BioPSE\Dataflow\XML\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\BioPSE\Dataflow\XML"; Flags: ignoreversion replacesameversion
Source: "src\Packages\BioPSE\nets\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\BioPSE\nets"; Flags: ignoreversion replacesameversion recursesubdirs

;Source: "src\Packages\Insight\Dataflow\GUI\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\Insight\Dataflow\GUI"; Flags: ignoreversion replacesameversion
;Source: "src\Packages\Insight\Dataflow\XML\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\Insight\Dataflow\XML"; Flags: ignoreversion replacesameversion
;Source: "src\Packages\Insight\nets\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\Insight\nets"; Flags: ignoreversion replacesameversion recursesubdirs

Source: "src\Packages\MatlabInterface\Dataflow\GUI\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\MatlabInterface\Dataflow\GUI"; Flags: ignoreversion replacesameversion
Source: "src\Packages\MatlabInterface\Dataflow\XML\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\MatlabInterface\Dataflow\XML"; Flags: ignoreversion replacesameversion
Source: "src\Packages\MatlabInterface\nets\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\MatlabInterface\nets"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "src\Packages\MatlabInterface\Services\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\MatlabInterface\Services"; Flags: ignoreversion replacesameversion

Source: "src\Packages\Teem\Dataflow\GUI\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\Teem\Dataflow\GUI"; Flags: ignoreversion replacesameversion
Source: "src\Packages\Teem\Dataflow\XML\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\Teem\Dataflow\XML"; Flags: ignoreversion replacesameversion
Source: "src\Packages\Teem\nets\*"; Excludes: ".svn"; DestDir: "{app}\src\Packages\Teem\nets"; Flags: ignoreversion replacesameversion recursesubdirs

; SCIRun build
Source: "bin\Release\Release\*.exe"; DestDir: "{app}\bin"; Flags: ignoreversion replacesameversion
Source: "bin\Release\Release\*.bat"; DestDir: "{app}\bin"; Flags: ignoreversion replacesameversion
Source: "bin\Release\lib\Release\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion replacesameversion

; SCIRun Thirdparty build
Source: "C:\Users\ayla\Documents\3P-compile\Release08\bin\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion replacesameversion
Source: "C:\Users\ayla\Documents\3P-compile\Release08\lib\tcl\*"; DestDir: "{app}\lib\tcl"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "C:\Users\ayla\Documents\3P-compile\Release08\lib\tcl8.4\*"; DestDir: "{app}\lib\tcl8.4"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "C:\Users\ayla\Documents\3P-compile\Release08\lib\tk8.4\*"; DestDir: "{app}\lib\tk8.4"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "C:\Users\ayla\Documents\3P-compile\Release08\lib\itcl\*"; DestDir: "{app}\lib\itcl"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "C:\Users\ayla\Documents\3P-compile\Release08\lib\itcl3.2\*"; DestDir: "{app}\lib\itcl3.2"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "C:\Users\ayla\Documents\3P-compile\Release08\lib\itk3.2\*"; DestDir: "{app}\lib\itk3.2"; Flags: ignoreversion replacesameversion recursesubdirs
Source: "C:\Users\ayla\Documents\3P-compile\Release08\lib\iwidgets\*"; DestDir: "{app}\lib\iwidgets"; Flags: ignoreversion replacesameversion recursesubdirs

[Icons]
Name: "{group}\SCIRun"; Filename: "{app}\bin\scirun.exe"; WorkingDir: "{app}"
Name: "{commondesktop}\SCIRun"; Filename: "{app}\bin\scirun.exe"; WorkingDir: "{app}"; Tasks: desktopicon
Name: "{group}\{cm:ProgramOnTheWeb,SCIRun}"; Filename: "http://software.sci.utah.edu"
Name: "{group}\{cm:UninstallProgram,SCIRun}"; Filename: "{uninstallexe}"

[Registry]
Root: HKLM; Subkey: "Software\SCI Institute\SCIRun"; ValueType: string; ValueName: "InstallPath"; ValueData: {app}; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\SCI Institute\SCIRun\Packages\BioPSE"; ValueType: string; ValueName: "version"; ValueData: "4.0"; Flags: uninsdeletevalue uninsdeletekeyifempty
;Root: HKLM; Subkey: "Software\SCI Institute\SCIRun\Packages\Insight"; ValueType: string; ValueName: "version"; ValueData: "4.0"; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\SCI Institute\SCIRun\Packages\MatlabInterface"; ValueType: string; ValueName: "version"; ValueData: "4.0"; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\SCI Institute\SCIRun\Packages\SCIRun"; ValueType: string; ValueName: "version"; ValueData: "4.0"; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\SCI Institute\SCIRun\Packages\Teem"; ValueType: string; ValueName: "version"; ValueData: "4.0"; Flags: uninsdeletevalue uninsdeletekeyifempty

[Run]
Filename: "{tmp}\vcredist_x86.exe"; Flags: nowait
Filename: "{app}\bin\scirun.exe"; Description: "{cm:LaunchProgram,SCIRun}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; cleanup user app data
Type: filesandordirs; Name: "{userappdata}\SCIRun\*"
;Type: files; Name: "{userappdata}\SCIRun\.scirunrc"
