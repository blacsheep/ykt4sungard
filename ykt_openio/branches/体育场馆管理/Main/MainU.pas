unit MainU;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, ComCtrls, ToolWin, ImgList, StdCtrls, Menus, ShellAPI, CommU;

type
  TRunDLLFun = function(AppHandle, ACtlHandle: integer; pConn: PDBConn; pUserInfo: PUser; AFrmHdl: Pointer): integer; stdcall;

  TMainFrm = class(TForm)
    tvMenuList: TTreeView;
    StatusBar1: TStatusBar;
    Splitter1: TSplitter;
    ImageList1: TImageList;
    MainMenu1: TMainMenu;
    N11111: TMenuItem;
    N1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    N7: TMenuItem;
    ToolBar1: TToolBar;
    tbtnExit: TToolButton;
    plControl: TPanel;
    Timer1: TTimer;
    N8: TMenuItem;
    procedure tbtnExitClick(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure tvMenuListDblClick(Sender: TObject);
    procedure N3Click(Sender: TObject);
    procedure N7Click(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure N8Click(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure FormResize(Sender: TObject);
  private
    { Private declarations }
    CurrentHandle, CurrentFrmHandle: HWND;
    //列举模块
    function funcListModule(AList: array of TMenu): boolean;
    //调用DLL
    function funcCallDll(ADll: string): boolean;
    //响应关闭DLL窗口
    procedure ProcWM_RELEASEDLL(var Msg: TMessage); message WM_RELEASEDLL;
    procedure CreateParams(var Params: TCreateParams); override; //
  public
    { Public declarations }
    usename: string;
  end;

var
  MainFrm: TMainFrm;

implementation

uses AboutU, LoginU, SetupU;

{$R *.dfm}
{$WARNINGS OFF}
 procedure TMainFrm.CreateParams(var Params: TCreateParams);
  begin
    inherited;
    Params.ExStyle := 33554432; //0x 02 00 00 00
  end;


function TMainFrm.funcListModule(AList: array of TMenu): boolean;
var
  i, j: integer;
  Top, ParentNode, ChildNode: TTreeNode;
begin
  Top := tvMenuList.TopItem;
  Top.ImageIndex := 0;
  Top.DeleteChildren;
  for i := 0 to length(AList) - 1 do
    if AList[i].isValid and (not AList[i].isWeb) then
    begin
      ParentNode := tvMenuList.Items.AddChild(Top, AList[i].Caption);
      ParentNode.ImageIndex := 1;
      ParentNode.SelectedIndex := 1;
      ParentNode.Data := Addr(AList[i].ID);
      for j := 0 to length(AList[i].SubMenu) - 1 do
        if AList[i].SubMenu[j].isVisble and
          (not AList[i].SubMenu[j].isWeb) then
        begin
          ChildNode := tvMenuList.Items.AddChild(ParentNode, AList[i].SubMenu[j].Caption);
          ChildNode.ImageIndex := 1;
          ChildNode.SelectedIndex := 2;
          ChildNode.Data := Addr(AList[i].SubMenu[j].DLL);
        end;
    end;
  result := true;
end;

{$WARNINGS ON}

procedure TMainFrm.tbtnExitClick(Sender: TObject);
begin
  Close;
end;

procedure TMainFrm.FormShow(Sender: TObject);
var
  hand: integer;
begin

  funcListModule(User.MenuList);
  StatusBar1.Panels.Items[1].Text := User.GetCNName;
  procGetIniInfo;
  

end;

{$WARNINGS OFF}

procedure TMainFrm.tvMenuListDblClick(Sender: TObject);
var
  DLL: string;
begin
  if tvMenuList.Selected.Level <> 2 then exit;
  DLL := string((tvMenuList.Selected.Data)^);
  screen.Cursor := crHourGlass;
  try
    funcCallDll(DLL);
  finally
    screen.Cursor := crDefault;
  end;
end;


function TMainFrm.funcCallDll(ADll: string): boolean;
var
  RunDll: TRunDLLFun;
  hdl: HWND; rst: integer;
begin
  result := false;
  try
    if CurrentFrmHandle <> 0 then
      SendMessage(CurrentFrmHandle, WM_CLOSE, 0, 0);
    if CurrentHandle <> 0 then
      if not Windows.FreeLibrary(CurrentHandle) then
        exit;
    hdl := Windows.LoadLibrary(Pchar(ADll));
    if (hdl = 0) then
    begin
      FuncShowMessage(Handle, '调用模块失败！' + ADll, 2);
      exit;
    end;
    if (hdl <> 0) then
    begin
      @RunDll := Windows.GetProcAddress(hdl, 'funcDllCall');
      if (@RunDll <> nil) then
      begin
        rst := Rundll(Handle, plControl.Handle, @DBConn, @User, @CurrentFrmHandle);
        if rst > 0 then
          result := true;
      end;
    end;
  except
    //Windows.FreeLibrary(hdl);

  end;
  CurrentHandle := hdl;
end;

{$WARNINGS ON}

procedure TMainFrm.N3Click(Sender: TObject);
var
  frm: TAboutFrm;
begin
  frm := TAboutFrm.Create(self);
  try
    frm.ShowModal;
  finally
    frm.Free;
  end;
end;




procedure TMainFrm.N7Click(Sender: TObject);
var
  frm: TLoginFrm;
begin
  frm := TLoginFrm.Create(self);
  try
    frm.ShowModal;
  finally
    frm.Free;
  end;
  if CurrentFrmHandle <> 0 then
    SendMessage(CurrentFrmHandle, WM_CLOSE, 0, 0);
  if CurrentHandle <> 0 then
    if Windows.FreeLibrary(CurrentHandle) then
      CurrentHandle := 0;
  funcListModule(User.MenuList);
  StatusBar1.Panels.Items[1].Text := User.GetCNName;
end;

procedure TMainFrm.Timer1Timer(Sender: TObject);
begin
  StatusBar1.Panels.Items[3].Text := formatDateTime('YYYY-MM-DD HH:NN:SS', now);
end;

procedure TMainFrm.N8Click(Sender: TObject);
var
  frm: TSetupFrm;
begin
  frm := TSetupFrm.Create(self);
  try
    frm.ShowModal;
  finally
    frm.Free;
  end;
end;

procedure TMainFrm.ProcWM_RELEASEDLL(var Msg: TMessage);
begin
  if CurrentFrmHandle <> 0 then
  begin
    SendMessage(CurrentFrmHandle, WM_CLOSE, 0, 0);
    CurrentFrmHandle := 0;
  end;
end;



procedure TMainFrm.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  //case Key of
   // VK_F1: funcCallDll('inppatinf'); //录入
  //  VK_F2: funcCallDll('datrep'); //日查询
  //  VK_F3: funcCallDll('motrep'); //月查询
 // end;        /
end;



procedure TMainFrm.FormResize(Sender: TObject);
begin
Application.BringToFront;

end;

end.

