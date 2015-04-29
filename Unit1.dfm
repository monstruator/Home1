object Form1: TForm1
  Left = 201
  Top = 136
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = #1056#1072#1073#1086#1090#1072' '#1089' COM-'#1087#1086#1088#1090#1086#1084' ('#1042#1077#1088#1089#1080#1103' 2 - TThread)'
  ClientHeight = 466
  ClientWidth = 629
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  Scaled = False
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label8: TLabel
    Left = 120
    Top = 56
    Width = 175
    Height = 27
    Caption = '                         '
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label13: TLabel
    Left = 16
    Top = 24
    Width = 69
    Height = 27
    Caption = #1058#1077#1084#1087'1'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clBlack
    Font.Height = -24
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label14: TLabel
    Left = 16
    Top = 56
    Width = 69
    Height = 27
    Caption = #1058#1077#1084#1087'2'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label15: TLabel
    Left = 16
    Top = 88
    Width = 72
    Height = 27
    Caption = #1042#1083#1072#1078#1085
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Label16: TLabel
    Left = 120
    Top = 24
    Width = 175
    Height = 27
    Caption = '                         '
    Color = clBtnFace
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'Arial'
    Font.Style = []
    ParentColor = False
    ParentFont = False
  end
  object Label17: TLabel
    Left = 120
    Top = 88
    Width = 175
    Height = 27
    Caption = '                         '
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object GroupBox1: TGroupBox
    Left = 472
    Top = 224
    Width = 137
    Height = 129
    Caption = #1053#1072#1089#1090#1088#1086#1081#1082#1080' '#1087#1086#1088#1090#1072
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    object Label1: TLabel
      Left = 16
      Top = 24
      Width = 66
      Height = 13
      Caption = #1053#1086#1084#1077#1088' '#1087#1086#1088#1090#1072
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
    end
    object SpeedButton1: TSpeedButton
      Left = 16
      Top = 72
      Width = 105
      Height = 25
      AllowAllUp = True
      GroupIndex = 1
      Caption = #1054#1090#1082#1088#1099#1090#1100' '#1087#1086#1088#1090
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      Glyph.Data = {
        E6000000424DE60000000000000076000000280000000E0000000E0000000100
        0400000000007000000000000000000000001000000000000000000000000000
        80000080000000808000800000008000800080800000C0C0C000808080000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33003333333333333300300000000333330000B7B7B7B03333000B0B7B7B7B03
        33000BB0B7B7B7B033000FBB0000000033000BFB0B0B0B0333000FBFBFBFB003
        33000BFBFBF00033330030BFBF03333333003800008333333300333333333333
        33003333333333333300}
      ParentFont = False
      OnClick = SpeedButton1Click
    end
    object ComboBox1: TComboBox
      Left = 16
      Top = 40
      Width = 97
      Height = 21
      Style = csDropDownList
      Color = clWhite
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ItemHeight = 13
      ParentFont = False
      TabOrder = 0
    end
  end
  object GroupBox2: TGroupBox
    Left = 496
    Top = 360
    Width = 121
    Height = 81
    Caption = #1055#1077#1088#1077#1076#1072#1095#1072' '#1076#1072#1085#1085#1099#1093
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 1
    object Edit1: TEdit
      Left = 8
      Top = 16
      Width = 89
      Height = 21
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      MaxLength = 254
      ParentFont = False
      TabOrder = 0
    end
    object Button1: TButton
      Left = 8
      Top = 40
      Width = 89
      Height = 25
      Caption = #1055#1077#1088#1077#1076#1072#1090#1100
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = Button1Click
    end
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 447
    Width = 629
    Height = 19
    Panels = <
      item
        Width = 140
      end
      item
        Width = 85
      end
      item
        Width = 50
      end>
    SimplePanel = False
    SizeGrip = False
  end
  object Edit2: TEdit
    Left = 144
    Top = 344
    Width = 121
    Height = 21
    TabOrder = 3
    Visible = False
  end
  object Button2: TButton
    Left = 48
    Top = 312
    Width = 75
    Height = 25
    Caption = 'TEPM1'
    TabOrder = 4
    Visible = False
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 48
    Top = 344
    Width = 75
    Height = 25
    Caption = 'TEMP2'
    TabOrder = 5
    Visible = False
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 48
    Top = 376
    Width = 75
    Height = 25
    Caption = 'TEMP3'
    TabOrder = 6
    Visible = False
    OnClick = Button4Click
  end
  object Edit3: TEdit
    Left = 56
    Top = 184
    Width = 209
    Height = 21
    TabOrder = 7
    Visible = False
  end
  object TrackBar1: TTrackBar
    Left = 24
    Top = 216
    Width = 201
    Height = 45
    Max = 20
    Min = 1
    Orientation = trHorizontal
    Frequency = 1
    Position = 1
    SelEnd = 0
    SelStart = 0
    TabOrder = 8
    TickMarks = tmBottomRight
    TickStyle = tsAuto
    Visible = False
    OnChange = TrackBar1Change
  end
  object Edit4: TEdit
    Left = 248
    Top = 224
    Width = 57
    Height = 21
    TabOrder = 9
    Visible = False
  end
  object Timer1: TTimer
    Interval = 100
    OnTimer = Timer1Timer
    Left = 568
    Top = 160
  end
end
