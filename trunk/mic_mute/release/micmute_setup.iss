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
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Languages: english
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeNameRus}"; Languages: russian
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run\{#MyAppExeName}"; Flags: deletekey
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run\{#MyAppExeNameRus}"; Flags: deletekey

[Tasks]
;Name: autorun; Description: "{cm:AutoStartProgram,{#StringChange(MyAppName, '&', '&&')}}"; GroupDescription: "{cm:AutoStartProgramGroupDescription}"

[Run]
;Filename: "schtasks"; Description: "Scheduling autorun task"; Parameters: "/create /sc onlogon /tn MicMute /rl highest /delay 0000:00 /tr ""{app}\{#MyAppExeName}"""; Flags: runhidden; Tasks: autorun; Languages: english
Filename: "schtasks"; Description: "Scheduling autorun task"; Parameters: "/create /sc onlogon /tn MicMute /rl highest /delay 0000:00 /tr ""{app}\{#MyAppExeName}"""; Flags: runhidden; Languages: english
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent; Languages: english

;Filename: "schtasks"; Description: "Настройка автозапуска в планировщике задач"; Parameters: "/create /sc onlogon /tn MicMute /rl highest /delay 0000:00 /tr ""{app}\{#MyAppExeNameRus}"""; Flags: runhidden; Tasks: autorun; Languages: russian
Filename: "schtasks"; Description: "Настройка автозапуска в планировщике задач"; Parameters: "/create /sc onlogon /tn MicMute /rl highest /delay 0000:00 /tr ""{app}\{#MyAppExeNameRus}"""; Flags: runhidden; Languages: russian
Filename: "{app}\{#MyAppExeNameRus}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent; Languages: russian

[Code]
function InitializeSetup: Boolean;
var
  ResultCode: Integer;
begin
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeName}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeNameRus}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('schtasks', '/delete /tn MicMute /f', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  result := true;
end;

function InitializeUninstall(): Boolean;
var
  ResultCode: Integer;
begin
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeName}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('taskkill', '/t /f /im "' + ExpandConstant('{#MyAppExeNameRus}') + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Exec('schtasks', '/delete /tn MicMute /f', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  result := true;
end;
