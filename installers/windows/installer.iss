; MusicGenVST Installer — Inno Setup Script

#ifndef PluginName
  #define PluginName "MusicGenVST"
#endif

#ifndef MyAppVersion
  #define MyAppVersion "0.0.1"
#endif
#define MyAppPublisher "Thunderboom Records"
#define MyAppURL "https://github.com/Revess/MusicGenVST"

[Setup]
AppId={{musicgenvst-installer}
AppName={#PluginName} — Windows x64
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={commoncf}\VST3
DefaultGroupName={#PluginName}
OutputDir=..\..\output
OutputBaseFilename=MusicGenVST-{#MyAppVersion}-Windows
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin

[Files]
Source: "..\..\build-artifacts\build\Plugins\{#PluginName}\{#PluginName}_artefacts\Release\VST3\{#PluginName}.vst3\*"; DestDir: "{app}\{#PluginName}.vst3"; Flags: recursesubdirs
; ACE-Step binaries bundled inside the VST3 directory
Source: "..\..\build-artifacts\acestep.cpp\build\ace-qwen3.exe"; DestDir: "{app}\{#PluginName}.vst3\acestep"; Flags: ignoreversion
Source: "..\..\build-artifacts\acestep.cpp\build\dit-vae.exe"; DestDir: "{app}\{#PluginName}.vst3\acestep"; Flags: ignoreversion
; Model download script
Source: "download-models.ps1"; DestDir: "{app}\{#PluginName}.vst3\acestep"; Flags: ignoreversion

[Run]
Filename: "powershell.exe"; Parameters: "-ExecutionPolicy Bypass -File ""{app}\{#PluginName}.vst3\acestep\download-models.ps1"""; StatusMsg: "Downloading ACE-Step models..."; Flags: runhidden

[UninstallDelete]
Type: dirifempty; Name: "{app}\{#PluginName}.vst3"
