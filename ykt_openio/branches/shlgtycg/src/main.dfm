object mainfrm: Tmainfrm
  Left = 285
  Top = 188
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = #20307#32946#22330#39302#25910#36153#31995#32479
  ClientHeight = 466
  ClientWidth = 862
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  WindowState = wsMaximized
  OnClose = FormClose
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object mainp: TPanel
    Left = 0
    Top = 0
    Width = 862
    Height = 466
    Align = alClient
    Color = clGradientInactiveCaption
    Ctl3D = False
    ParentCtl3D = False
    TabOrder = 0
    object Label1: TLabel
      Left = 1224
      Top = 8
      Width = 32
      Height = 13
      Caption = 'Label1'
      Visible = False
    end
    object photo: TPanel
      Left = 208
      Top = 113
      Width = 193
      Height = 225
      TabOrder = 0
      object Image1: TImage
        Left = 1
        Top = 1
        Width = 191
        Height = 223
        Align = alClient
        Center = True
        Stretch = True
      end
    end
    object messagep: TPanel
      Left = 432
      Top = 64
      Width = 345
      Height = 345
      Color = clGradientInactiveCaption
      TabOrder = 1
      object name: TPanel
        Left = 16
        Top = 104
        Width = 281
        Height = 65
        Alignment = taLeftJustify
        Caption = #22995#21517#65306#29579#23567#20108
        Font.Charset = GB2312_CHARSET
        Font.Color = clWindowText
        Font.Height = -35
        Font.Name = #21326#25991#23435#20307
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 0
      end
      object studentno: TPanel
        Left = 16
        Top = 184
        Width = 321
        Height = 65
        Alignment = taLeftJustify
        Caption = #23398#24037#21495#65306'8939293909'
        Font.Charset = GB2312_CHARSET
        Font.Color = clWindowText
        Font.Height = -35
        Font.Name = #21326#25991#23435#20307
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 1
      end
      object Pbala: TPanel
        Left = 16
        Top = 16
        Width = 281
        Height = 65
        Alignment = taLeftJustify
        Caption = #21345#20313#39069#65306
        Font.Charset = GB2312_CHARSET
        Font.Color = clWindowText
        Font.Height = -35
        Font.Name = #21326#25991#23435#20307
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 2
      end
      object entertime: TPanel
        Left = 16
        Top = 264
        Width = 321
        Height = 65
        Alignment = taLeftJustify
        Caption = #36827#22330#26102#38388#65306
        Font.Charset = GB2312_CHARSET
        Font.Color = clWindowText
        Font.Height = -35
        Font.Name = #21326#25991#23435#20307
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 3
      end
    end
    object welcome: TPanel
      Left = 1
      Top = 400
      Width = 860
      Height = 65
      Align = alBottom
      Anchors = [akLeft, akTop, akBottom]
      BevelOuter = bvNone
      Caption = #29579#23567#20108#65292#27426#36814#20320#30340#21040#26469#65281
      Color = clGradientInactiveCaption
      Ctl3D = False
      Font.Charset = GB2312_CHARSET
      Font.Color = clWindowText
      Font.Height = -35
      Font.Name = #38582#20070
      Font.Style = [fsBold]
      ParentCtl3D = False
      ParentFont = False
      TabOrder = 2
    end
    object Panel3: TPanel
      Left = -8
      Top = 0
      Width = 361
      Height = 49
      BevelOuter = bvNone
      Color = clGradientInactiveCaption
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -24
      Font.Name = #23435#20307
      Font.Style = []
      ParentFont = False
      TabOrder = 3
    end
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 136
    Top = 24
  end
  object Timer2: TTimer
    Interval = 120000
    OnTimer = Timer2Timer
    Left = 224
    Top = 32
  end
end
