object datemngfrm: Tdatemngfrm
  Left = 235
  Top = 103
  Caption = #25910#36153#26102#38388#27573#31649#29702
  ClientHeight = 555
  ClientWidth = 984
  Color = clBtnFace
  Font.Charset = GB2312_CHARSET
  Font.Color = clWindowText
  Font.Height = -16
  Font.Name = #23435#20307
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 16
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 984
    Height = 555
    Align = alClient
    TabOrder = 0
    object Panel2: TPanel
      Left = 1
      Top = 1
      Width = 982
      Height = 32
      Align = alTop
      TabOrder = 0
      object Label1: TLabel
        Left = 398
        Top = 4
        Width = 88
        Height = 23
        Caption = #25910#36153#31649#29702
        Font.Charset = ANSI_CHARSET
        Font.Color = clBlue
        Font.Height = -21
        Font.Name = #21326#25991#23435#20307
        Font.Style = [fsBold]
        ParentFont = False
      end
    end
    object Panel3: TPanel
      Left = 1
      Top = 33
      Width = 982
      Height = 435
      Align = alClient
      TabOrder = 1
      object Panel5: TPanel
        Left = 1
        Top = 1
        Width = 980
        Height = 433
        Align = alClient
        Caption = 'Panel5'
        TabOrder = 0
        object DBGrid1: TDBGrid
          Left = 1
          Top = 1
          Width = 978
          Height = 431
          Align = alClient
          DataSource = DataSource1
          Options = [dgTitles, dgIndicator, dgColumnResize, dgColLines, dgRowLines, dgTabs, dgConfirmDelete, dgCancelOnExit, dgMultiSelect]
          ReadOnly = True
          TabOrder = 0
          TitleFont.Charset = GB2312_CHARSET
          TitleFont.Color = clWindowText
          TitleFont.Height = -16
          TitleFont.Name = #23435#20307
          TitleFont.Style = []
          OnCellClick = DBGrid1CellClick
          Columns = <
            item
              Expanded = False
              FieldName = 'fee_type'
              Title.Caption = #21345#31867#21035
              Width = 79
              Visible = True
            end
            item
              Expanded = False
              FieldName = 'money'
              Title.Caption = #25910#36153#37329#39069'/'#23567#26102
              Width = 127
              Visible = True
            end
            item
              Expanded = False
              FieldName = 'OPERATOR'
              Title.Caption = #25805#20316#21592
              Width = 153
              Visible = True
            end
            item
              Expanded = False
              FieldName = 'RESERVE'
              Title.Caption = #22791#27880
              Width = 302
              Visible = True
            end>
        end
      end
    end
    object Panel4: TPanel
      Left = 1
      Top = 468
      Width = 982
      Height = 86
      Align = alBottom
      TabOrder = 2
      object Label14: TLabel
        Left = 19
        Top = 31
        Width = 80
        Height = 16
        Caption = #25910#36153#31867#21035#65306
      end
      object Label2: TLabel
        Left = 281
        Top = 31
        Width = 64
        Height = 16
        Caption = #25910#36153#39069#65306
      end
      object datemod: TButton
        Left = 579
        Top = 31
        Width = 57
        Height = 25
        Caption = #20462#25913
        TabOrder = 0
        OnClick = datemodClick
      end
      object deldate: TButton
        Left = 651
        Top = 31
        Width = 55
        Height = 25
        Caption = #21024#38500
        TabOrder = 1
        OnClick = deldateClick
      end
      object dateadd: TButton
        Left = 508
        Top = 31
        Width = 58
        Height = 25
        Caption = #28155#21152
        TabOrder = 2
        OnClick = dateaddClick
      end
      object money: TEdit
        Left = 340
        Top = 29
        Width = 157
        Height = 24
        TabOrder = 3
      end
      object ComboBox1: TComboBox
        Left = 116
        Top = 31
        Width = 145
        Height = 24
        TabOrder = 4
        OnChange = ComboBox1Change
      end
    end
  end
  object DataSource1: TDataSource
    Left = 169
    Top = 113
  end
  object SaveDialog1: TSaveDialog
    Filter = '.xls|excel '#25991#20214
    Left = 129
    Top = 113
  end
end
