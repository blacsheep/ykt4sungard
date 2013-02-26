library datemangemod;

{ Important note about DLL memory management: ShareMem must be the
  first unit in your library's USES clause AND your project's (select
  Project-View Source) USES clause if your DLL exports any procedures or
  functions that pass strings as parameters or function results. This
  applies to all strings passed to and from your DLL--even those that
  are nested in records and classes. ShareMem is the interface unit to
  the BORLNDMM.DLL shared memory manager, which must be deployed along
  with your DLL. To avoid using BORLNDMM.DLL, pass string information
  using PChar or ShortString parameters. }

uses
  SysUtils,
  Classes,
  Windows,
  Forms,
  Controls,
  CommU in '..\Share\CommU.pas',
  datareplist in 'datareplist.pas' {datemngfrm},
  data2Excel in '..\Share\data2Excel.pas',
  AES in '..\Share\AES.pas',
  AES2010 in '..\Share\AES2010.pas';

{$R *.res}
function funcDllCall(AppHandle, ACtlHandle: integer; pConn: PDBConn; pUserInfo: PUser;PHdl:Pointer): integer; stdcall;
begin
  PtConn := pConn;
  PtUser := pUserInfo;
  Application.Handle := AppHandle;
  datemngfrm := Tdatemngfrm.Create(Application);
  PInteger(PHdl)^:= datemngfrm.Handle;
  with datemngfrm do
  begin
    ParentWindow := ACtlHandle;
    BorderStyle := bsNone;
    WindowState := wsMaximized;
    Show;
  end;
  result := 1;
end;

exports
  funcDllCall;
begin
end.
