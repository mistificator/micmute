#define MyAppName "MicMute"
#define MyAppPublisher "Mist Poryvaev"
#define MyAppURL "https://sourceforge.net/projects/micmute"
#define MyAppExeName "mic_mute.exe"
#define MyAppVersion GetFileVersion(MyAppExeName)

#define MyAppExeNameRus "mic_mute_rus.exe"

[Setup]
AppId={{A1FDC62A-32EC-4AA3-BBB6-80A7977CCAE2}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename={#MyAppName}_{#MyAppVersion}_Setup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin
VersionInfoVersion={#MyAppVersion}

[Languages]
Name: english; MessagesFile: "compiler:Default.isl"
Name: russian; MessagesFile: "compiler:Languages\Russian.isl"

[Files]
Source: ".\key_hook.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion; Languages: english
Source: ".\{#MyAppExeNameRus}"; DestDir: "{app}"; Flags: ignoreversion; Languages: russian
Source: ".\beep300.wav"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\beep750.wav"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\mic_mute.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\mic_mute_red.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\mic_mute_green.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\mic_mute_gray.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Languages: english
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeNameRus}"; Languages: russian
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Tasks]
Name: autorun; Description: "{cm:AutoStartProgram,{#StringChange(MyAppName, '&', '&&')}}"; GroupDescription: "{cm:AutoStartProgramGroupDescription}"

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "MicMute"; Flags: deletevalue; MinVersion: 6.0
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "MicMute"; Check: not IsTaskSelected('autorun'); Flags: deletevalue; OnlyBelowVersion: 6.0
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "MicMute"; ValueData: "{app}\{#MyAppExeName}"; Check: IsTaskSelected('autorun'); Flags: deletevalue; OnlyBelowVersion: 6.0; Languages: english
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "MicMute"; ValueData: "{app}\{#MyAppExeNameRus}"; Check: IsTaskSelected('autorun'); Flags: deletevalue; OnlyBelowVersion: 6.0; Languages: russian

[INI]
Filename: "{localappdata}\{#MyAppName}\mic_mute.ini"; Section: "Mic_Mute"; Key: "Autorun"; String: "0"; Check: not IsTaskSelected('autorun')
Filename: "{localappdata}\{#MyAppName}\mic_mute.ini"; Section: "Mic_Mute"; Key: "Autorun"; String: "1"; Check: IsTaskSelected('autorun')

[Run]
Filename: "schtasks"; Description: "Scheduling autorun task"; Parameters: "/create /sc onlogon /tn MicMute /rl highest /tr '""{app}\{#MyAppExeName}""'"; Flags: runhidden; Tasks: autorun; MinVersion: 6.0; Languages: english
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent runascurrentuser; Languages: english

Filename: "schtasks"; Description: "Настройка автозапуска в планировщике задач"; Parameters: "/create /sc onlogon /tn MicMute /rl highest /tr '""{app}\{#MyAppExeNameRus}""'"; Flags: runhidden; Tasks: autorun; MinVersion: 6.0; Languages: russian
Filename: "{app}\{#MyAppExeNameRus}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent runascurrentuser; Languages: russian

[Code]
function InitializeSetup: Boolean;
var
  ResultCode: Integer;
  Version: TWindowsVersion;
begin
  GetWindowsVersionEx(Version);
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeName}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeNameRus}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  if (Version.Major >= 6) then
  begin
    Exec('schtasks', '/delete /tn MicMute /f', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end;
  result := true;
end;

function InitializeUninstall(): Boolean;
var
  ResultCode: Integer;
  Version: TWindowsVersion;
begin
  GetWindowsVersionEx(Version);
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeName}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeNameRus}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  if (Version.Major >= 6) then
  begin
    Exec('schtasks', '/delete /tn MicMute /f', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end;
  result := true;
end;
