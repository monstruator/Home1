#include <vcl.h>
#pragma hdrstop

#include <io.h>         //��� ������ � �������
#include <fcntl.h>      //��� ������ � �������
#include <sys\stat.h>   //��� ������ � �������

#include "Unit1.h"
#include "Unit2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;


//=============================================================================
//..................... ���������� ���������� ���������� ......................
//=============================================================================

#define BUFSIZE 255     //������� ������

unsigned char bufrd[BUFSIZE], bufwr[BUFSIZE]; //������� � ���������� ������

//---------------------------------------------------------------------------

HANDLE COMport;		//���������� �����

//��������� OVERLAPPED ���������� ��� ����������� ��������, ��� ���� ��� �������� ������ � ������ ����� �������� ������ ���������
//��� ��������� ���������� �������� ���������, ����� ��������� �� ����� �������� ���������
OVERLAPPED overlapped;		//����� ������������ ��� �������� ������ (��. ����� ReadThread)
OVERLAPPED overlappedwr;       	//����� ������������ ��� �������� ������ (��. ����� WriteThread)

//---------------------------------------------------------------------------

int handle;             	//���������� ��� ������ � ������ � ������� ���������� <io.h>

//---------------------------------------------------------------------------

bool fl=0;	//����, ����������� �� ���������� �������� ������ (1 - �������, 0 - �� �������)

unsigned long counter;	//������� �������� ������, ���������� ��� ������ �������� �����


//=============================================================================
//.............................. ���������� ������� ...........................
//=============================================================================

void COMOpen(void);             //������� ����
void COMClose(void);            //������� ����

//=============================================================================
//.............................. ���������� ������� ...........................
//=============================================================================

//---------------------------------------------------------------------------

//����� ��� ������ ������������������ ������ �� COM-����� � �����
class ReadThread : public TThread
{
 private:
        void __fastcall Printing();	//����� �������� ������ �� ����� � � ����
 protected:
        void __fastcall Execute();	//�������� ������� ������
 public:
        __fastcall ReadThread(bool CreateSuspended);	//����������� ������
};

//---------------------------------------------------------------------------

//����� ��� ������ ������������������ ������ �� ������ � COM-����
class WriteThread : public TThread
{
private:
 	void __fastcall Printing();	//����� ��������� �� �����
protected:
        void __fastcall Execute();      //�������� ������� ������
public:
        __fastcall WriteThread(bool CreateSuspended);	//����������� ������
};

//---------------------------------------------------------------------------


//=============================================================================
//.............................. ���������� ������� ...........................
//=============================================================================

//-----------------------------------------------------------------------------
//............................... ����� ReadThead .............................
//-----------------------------------------------------------------------------

ReadThread *reader;     //������ ������ ReadThread
       DWORD btr;
//---------------------------------------------------------------------------

//����������� ������ ReadThread, �� ��������� ������
__fastcall ReadThread::ReadThread(bool CreateSuspended) : TThread(CreateSuspended)
{}

//---------------------------------------------------------------------------

//������� ������� ������, ��������� ���� ������ �� COM-�����
void __fastcall ReadThread::Execute()
{
 COMSTAT comstat;		//��������� �������� ��������� �����, � ������ ��������� ������������ ��� ����������� ���������� �������� � ���� ������
 DWORD  temp, mask, signal;	//���������� temp ������������ � �������� ��������

 overlapped.hEvent = CreateEvent(NULL, true, true, NULL);	//������� ���������� ������-������� ��� ����������� ��������
 SetCommMask(COMport, EV_RXCHAR);                   	        //���������� ����� �� ������������ �� ������� ����� ����� � ����
 while(!Terminated)						//���� ����� �� ����� �������, ��������� ����
  {
   WaitCommEvent(COMport, &mask, &overlapped);               	//������� ������� ����� ����� (��� � ���� ������������� ��������)
   signal = WaitForSingleObject(overlapped.hEvent, INFINITE);	//������������� ����� �� ������� �����
   if(signal == WAIT_OBJECT_0)				        //���� ������� ������� ����� ���������
    {
     if(GetOverlappedResult(COMport, &overlapped, &temp, true)) //���������, ������� �� ����������� ������������� �������� WaitCommEvent
      if((mask & EV_RXCHAR)!=0)					//���� ��������� ������ ������� ������� �����
       {
        ClearCommError(COMport, &temp, &comstat);		//����� ��������� ��������� COMSTAT
        btr = comstat.cbInQue;                          	//� �������� �� �� ���������� �������� ������
        if(btr)                         			//���� ������������� ���� ����� ��� ������
        {
         ReadFile(COMport, bufrd, btr, &temp, &overlapped);     //��������� ����� �� ����� � ����� ���������
         counter+=btr;                                          //����������� ������� ������
         //Synchronize(Printing);                     		//�������� ������� ��� ������ ������ �� ����� � � ����

         //Synchronize(Printing);
        }
       }
    }
  }
 CloseHandle(overlapped.hEvent);		//����� ������� �� ������ ������� ������-�������
}

//-----------------------------------------------------------------------------
//............................... ����� WriteThead ............................
//-----------------------------------------------------------------------------

WriteThread *writer;     //������ ������ WriteThread

//---------------------------------------------------------------------------

//����������� ������ WriteThread, �� ��������� ������
__fastcall WriteThread::WriteThread(bool CreateSuspended) : TThread(CreateSuspended)
{}

//---------------------------------------------------------------------------

//������� ������� ������, ��������� �������� ������ �� ������ � COM-����
void __fastcall WriteThread::Execute()
{
 DWORD temp, signal;	//temp - ����������-��������

 overlappedwr.hEvent = CreateEvent(NULL, true, true, NULL);   	  //������� �������

 while(!Terminated)     //���� ����� �� ����� ��������, ��������� ����
 {
 //strlen(bufwr)
  WriteFile(COMport, bufwr, 2, &temp, &overlappedwr);  //�������� ����� � ���� (������������� ��������!)
  signal = WaitForSingleObject(overlappedwr.hEvent, INFINITE);	  //������������� �����, ���� �� ���������� ������������� �������� WriteFile
  if((signal == WAIT_OBJECT_0) && (GetOverlappedResult(COMport, &overlappedwr, &temp, true))) fl = true; //���� �������� ����������� �������, ���������� ��������������� ������
  else fl = false;

  //Synchronize(Printing);	//������� ��������� �������� � ������ ���������
  writer->Suspend();		//������������� ����� ������ � ����, ���� �� �� ����������� �����
 }
 CloseHandle(overlappedwr.hEvent);		//����� ������� �� ������ ������� ������-�������
}

//---------------------------------------------------------------------------

//����� ��������� �������� ������ �� �����
void __fastcall WriteThread::Printing()
{
 if(!fl)	//��������� ��������� ������
  {
   Form1->StatusBar1->Panels->Items[0]->Text  = "������ ��������";
   return;
  }
 Form1->StatusBar1->Panels->Items[0]->Text  = "�������� ������ �������";

}

//---------------------------------------------------------------------------


//=============================================================================
//............................. �������� ����� ................................
//=============================================================================

//---------------------------------------------------------------------------

//����������� �����, ������ � �� ����������� ������������� ��������� �����
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner)
{
 //������������� ��������� ����� ��� ������� ���������

 Form1->Button1->Enabled = false;
 Form1->Button2->Enabled = false;
   Form1->Button3->Enabled = false;
   Form1->Button4->Enabled = false;
   Form1->TrackBar1-> Enabled = false;

}

//---------------------------------------------------------------------------

//���������� ������� �� ������ "������� ����"
void __fastcall TForm1::SpeedButton1Click(TObject *Sender)
{
 if(SpeedButton1->Down)
  {
   COMOpen();                   //���� ������ ������ - ������� ����

   //��������/�������� �������� �� �����
   Form1->ComboBox1->Enabled = false;
  // Form1->ComboBox2->Enabled = false;
   Form1->Button1->Enabled = true;
   Form1->Button2->Enabled = true;
   Form1->Button3->Enabled = true;
   Form1->Button4->Enabled = true;
   Form1->Button5->Enabled = true;
   Form1->Button6->Enabled = true;
   Form1->TrackBar1-> Enabled = true;
  // Form1->CheckBox1->Enabled = true;
  // Form1->CheckBox2->Enabled = true;
   //Form2->Visible = true;
   Form1->SpeedButton1->Caption = "������� ����";	//������� ������� �� ������

   counter = 0;	//���������� ������� ������
  }

 else
  {
   COMClose();                  //���� ������ ������ - ������� ����

   Form1->SpeedButton1->Caption = "������� ����";	//������� ������� �� ������
   Form1->StatusBar1->Panels->Items[0]->Text = "";	//�������� ������ ������� ������ ���������

   //��������/�������� �������� �� �����
   Form1->ComboBox1->Enabled = true;
  // Form1->ComboBox2->Enabled = true;
   Form1->Button1->Enabled = false;
   Form1->Button2->Enabled = false;
   Form1->Button3->Enabled = false;
   Form1->Button4->Enabled = false;
   Form1->TrackBar1-> Enabled = false;
   //Form1->CheckBox1->Enabled = false;
   //Form1->CheckBox2->Enabled = false;
  }
}

//---------------------------------------------------------------------------

//���������� �������� �����
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
 if(reader)reader->Terminate(); 	//��������� ����� ������ �� �����, �������� if(reader) �����������, ����� ��������� ������
 if(writer)writer->Terminate();         //��������� ����� ������ � ����, �������� if(writer) �����������, ����� ��������� ������
 if(COMport)CloseHandle(COMport);       //������� ����
 if(handle)close(handle);               //������� ����, � ������� ������ ������ ����������� ������
}
//---------------------------------------------------------------------------


//������ "��������"
void __fastcall TForm1::Button1Click(TObject *Sender)
{
        memset(bufwr,0,BUFSIZE);  //�������� ����������� ���������� �����, ����� ������ �� ������������� ���� �� �����
        PurgeComm(COMport, PURGE_TXCLEAR);  //�������� ���������� ����� �����

        if (Form1->Edit1->Text!="")
        {
        //strcpy(bufwr,Form1->Edit1->Text.c_str());      //������� � ����������� ���������� ����� ������ �� Edit1
        bufwr[0] = StrToInt(Form1->Edit1->Text);
        writer->Resume();	//������������ ����� ������ � ����
        }
        else
        {
            bufwr[0] = 0xAB;
            //bufwr[1] = 10;
            //bufwr[2] = 20;
            //bufwr[3] = 30;
            writer->Resume();	//������������ ����� ������ � ����
        }
}


//---------------------------------------------------------------------------

//=============================================================================
//........................... ���������� ������� ..............................
//=============================================================================

//---------------------------------------------------------------------------

//������� �������� � ������������� �����
void COMOpen()
{
 char n_com[16];
 String portname;     	 //��� ����� (��������, "COM1", "COM2" � �.�.)
 DCB dcb;                //��������� ��� ����� ������������� ����� DCB
 COMMTIMEOUTS timeouts;  //��������� ��� ��������� ���������

 //portname = Form1->ComboBox1->Text;	//�������� ��� ���������� �����
 strcpy(n_com,("\\\\.\\"+Form1->ComboBox1->Text).c_str());

 //������� ����, ��� ����������� �������� ����������� ����� ������� ���� FILE_FLAG_OVERLAPPED
 //COMport = CreateFile(portname.c_str(),GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
 COMport = CreateFile(n_com,GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
 //�����:
 // - portname.c_str() - ��� ����� � �������� ����� �����, c_str() ����������� ������ ���� String � ������ � ���� ������� ���� char, ����� ������� �� ������
 // - GENERIC_READ | GENERIC_WRITE - ������ � ����� �� ������/�������
 // - 0 - ���� �� ����� ���� ������������� (shared)
 // - NULL - ���������� ����� �� �����������, ������������ ���������� ������������ �� ���������
 // - OPEN_EXISTING - ���� ������ ����������� ��� ��� ������������ ����
 // - FILE_FLAG_OVERLAPPED - ���� ���� ��������� �� ������������� ����������� ��������
 // - NULL - ��������� �� ���� ������� �� ������������ ��� ������ � �������

 if(COMport == INVALID_HANDLE_VALUE)            //���� ������ �������� �����
  {
   Form1->SpeedButton1->Down = false;           //������ ������
   Form1->StatusBar1->Panels->Items[0]->Text = "�� ������� ������� ����";       //������� ��������� � ������ ���������
   return;
  }

 //������������� �����

 dcb.DCBlength = sizeof(DCB); 	//� ������ ���� ��������� DCB ���������� ������� � �����, ��� ����� �������������� ��������� ��������� ����� ��� �������� ������������ ���������

 //������� ��������� DCB �� �����
 if(!GetCommState(COMport, &dcb))	//���� �� ������� - ������� ���� � ������� ��������� �� ������ � ������ ���������
  {
   COMClose();
   Form1->StatusBar1->Panels->Items[0]->Text  = "�� ������� ������� DCB";
   return;
  }

 //������������� ��������� DCB
 dcb.BaudRate = StrToInt("115200");       //����� �������� �������� 115200 ���
 dcb.fBinary = TRUE;                                    //�������� �������� ����� ������
 dcb.fOutxCtsFlow = FALSE;                              //��������� ����� �������� �� �������� CTS
 dcb.fOutxDsrFlow = FALSE;                              //��������� ����� �������� �� �������� DSR
 dcb.fDtrControl = DTR_CONTROL_DISABLE;                 //��������� ������������� ����� DTR
 dcb.fDsrSensitivity = FALSE;                           //��������� ��������������� �������� � ��������� ����� DSR
 dcb.fNull = FALSE;                                     //��������� ���� ������� ������
 dcb.fRtsControl = RTS_CONTROL_DISABLE;                 //��������� ������������� ����� RTS
 dcb.fAbortOnError = FALSE;                             //��������� ��������� ���� �������� ������/������ ��� ������
 dcb.ByteSize = 8;                                      //����� 8 ��� � �����
 dcb.Parity = 0;                                        //��������� �������� ��������
 dcb.StopBits = 0;                                      //����� ���� ����-���

 //��������� ��������� DCB � ����
 if(!SetCommState(COMport, &dcb))	//���� �� ������� - ������� ���� � ������� ��������� �� ������ � ������ ���������
  {
   COMClose();
   Form1->StatusBar1->Panels->Items[0]->Text  = "�� ������� ���������� DCB";
   return;
  }

 //���������� ��������
 timeouts.ReadIntervalTimeout = 0;	 	//������� ����� ����� ���������
 timeouts.ReadTotalTimeoutMultiplier = 0;	//����� ������� �������� ������
 timeouts.ReadTotalTimeoutConstant = 0;         //��������� ��� ������ �������� �������� ������
 timeouts.WriteTotalTimeoutMultiplier = 0;      //����� ������� �������� ������
 timeouts.WriteTotalTimeoutConstant = 0;        //��������� ��� ������ �������� �������� ������

 //�������� ��������� ��������� � ����
 if(!SetCommTimeouts(COMport, &timeouts))	//���� �� ������� - ������� ���� � ������� ��������� �� ������ � ������ ���������
  {
   COMClose();
   Form1->StatusBar1->Panels->Items[0]->Text  = "�� ������� ���������� ����-����";
   return;
  }

 //���������� ������� �������� ����� � ��������
 SetupComm(COMport,2000,2000);
   
 //������� ��� ������� ������������ ���� ��� ������ ����������� ������
 handle = open("test.txt", O_CREAT | O_APPEND | O_BINARY | O_WRONLY, S_IREAD | S_IWRITE);

 if(handle==-1)		//���� ��������� ������ �������� �����
  {
   Form1->StatusBar1->Panels->Items[1]->Text = "������ �������� �����";	//������� ��������� �� ���� � ��������� ������
  }
 else { Form1->StatusBar1->Panels->Items[0]->Text = "���� ������ �������"; } //����� ������� � ������ ��������� ��������� �� �������� �������� ����� 

 PurgeComm(COMport, PURGE_RXCLEAR);	//�������� ����������� ����� �����

 reader = new ReadThread(false);	//������� � ��������� ����� ������ ������
 reader->FreeOnTerminate = true;        //���������� ��� �������� ������, ����� �� ������������� ����������� ����� ����������

 writer = new WriteThread(true);               //������� ����� ������ ������ � ����
 writer->FreeOnTerminate = true;                //���������� ��� ��������, ����� ����� ������������� ����������� ����� ����������

}

//---------------------------------------------------------------------------

//������� �������� �����
void COMClose()
{
 //���� ����� ������ ����������, ������ ��� ������� �� ���������� � ��������� ���, ����� �� �������� ����������;
 if(writer) //�������� if(writer) �����������, ����� ��������� ������;
  {
   writer->Terminate();
   writer->Resume();
  }

 if(reader)reader->Terminate();         //���� ����� ������ ��������, ��������� ���; �������� if(reader) �����������, ����� ��������� ������
 CloseHandle(COMport);                  //������� ����
 COMport=0;				//�������� ���������� ��� ����������� �����
 close(handle);				//������� ���� ��� ������ ����������� ������
 handle=0;				//�������� ���������� ��� ����������� �����
}

//-------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
HANDLE hdcom;
AnsiString TmpStr1;
Form1->Timer1->Enabled = true;
for (int i=1;i<=30;i++)
      {
       TmpStr1="\\\\.\\COM"+IntToStr(i);
        hdcom=CreateFile(TmpStr1.c_str(),GENERIC_READ|GENERIC_WRITE,
                        0,NULL,OPEN_EXISTING,0,NULL);
        if (hdcom!=INVALID_HANDLE_VALUE)
          {
           ComboBox1->Items->Add("COM"+IntToStr(i));
           CloseHandle(hdcom);
          }
      }
  ComboBox1->ItemIndex=0;
}
//---------------------------------------------------------------------------





void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
        int sum=0;
        for (int i=0;i<9;i++) sum+=bufrd[i];
    if (sum==bufrd[9])
    {
       Form1->Label16 ->Caption = IntToStr(bufrd[0]);
       Form1->Label8 ->Caption = IntToStr(bufrd[1]);
       Form1->Label17->Caption = IntToStr(bufrd[2]);
       Form1->Edit3->Text = "";
       for(int i=0;i<10;i++) Form1->Edit3->Text = Form1->Edit3->Text + IntToStr (bufrd[i]) + " ";
       if (bufrd[0] < 25) Form1->Label16->Font->Color = 0x00FF00;
       if (bufrd[0] < 19) Form1->Label16->Font->Color = 0xFFFF00;
       if (bufrd[0] > 25) Form1->Label16->Font->Color = 0x0FFFFF;
       if (bufrd[0] > 35) Form1->Label16->Font->Color = 0x0000FF;

       if (bufrd[1] < 25) Form1->Label8->Font->Color = 0x00FF00;
       if (bufrd[1] < 19) Form1->Label8->Font->Color = 0xFFFF00;
       if (bufrd[1] > 25) Form1->Label8->Font->Color = 0x0FFFFF;
       if (bufrd[1] > 35) Form1->Label8->Font->Color = 0x0000FF;

       //Form1->Label8 ->Caption = IntToStr(bufrd[1]);
       //Form1->Label17->Caption = IntToStr(bufrd[2]);
     }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button2Click(TObject *Sender)
{
        memset(bufwr,0,BUFSIZE);  //�������� ����������� ���������� �����, ����� ������ �� ������������� ���� �� �����
        PurgeComm(COMport, PURGE_TXCLEAR);  //�������� ���������� ����� �����

        if (Form1->Edit2->Text!="")
        {
        bufwr[0]=0xF0;
        //strcpy(bufwr,Form1->Edit1->Text.c_str());      //������� � ����������� ���������� ����� ������ �� Edit1
        bufwr[1] = StrToInt(Form1->Edit2->Text);
        //bufwr[0] = bufwr[0] | 0x00;
        writer->Resume();	//������������ ����� ������ � ����
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
    memset(bufwr,0,BUFSIZE);  //�������� ����������� ���������� �����, ����� ������ �� ������������� ���� �� �����
        PurgeComm(COMport, PURGE_TXCLEAR);  //�������� ���������� ����� �����

        if (Form1->Edit2->Text!="")
        {
        bufwr[0]=0xF5;
        bufwr[1] = StrToInt(Form1->Edit2->Text);
        writer->Resume();	//������������ ����� ������ � ����
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
        memset(bufwr,0,BUFSIZE);  //�������� ����������� ���������� �����, ����� ������ �� ������������� ���� �� �����
        PurgeComm(COMport, PURGE_TXCLEAR);  //�������� ���������� ����� �����

        if (Form1->Edit2->Text!="")
        {
        bufwr[0]=0xF2;
        bufwr[1] = StrToInt(Form1->Edit2->Text);
        writer->Resume();	//������������ ����� ������ � ����
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TrackBar1Change(TObject *Sender)
{
   Form1->Edit4->Text=IntToStr(Form1->TrackBar1->Position);
   memset(bufwr,0,BUFSIZE);  //�������� ����������� ���������� �����, ����� ������ �� ������������� ���� �� �����
   PurgeComm(COMport, PURGE_TXCLEAR);  //�������� ���������� ����� �����

   //if (Form1->Edit2->Text!="")
   {
        bufwr[0]=0xF3;
        bufwr[1] = StrToInt(Form1->Edit4->Text);
        writer->Resume();	//������������ ����� ������ � ����
   }
}



void __fastcall TForm1::Button5Click(TObject *Sender)
{
     memset(bufwr,0,BUFSIZE);  //�������� ����������� ���������� �����, ����� ������ �� ������������� ���� �� �����
        PurgeComm(COMport, PURGE_TXCLEAR);  //�������� ���������� ����� �����

        bufwr[0]=0xF5;
        bufwr[1] = 1;
        writer->Resume();	//������������ ����� ������ � ����
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
        memset(bufwr,0,BUFSIZE);  //�������� ����������� ���������� �����, ����� ������ �� ������������� ���� �� �����
        PurgeComm(COMport, PURGE_TXCLEAR);  //�������� ���������� ����� �����

        bufwr[0]=0xF5;
        bufwr[1] = 0;
        writer->Resume();	//������������ ����� ������ � ����
}
//---------------------------------------------------------------------------

