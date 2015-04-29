#include <vcl.h>
#pragma hdrstop

#include <io.h>         //для работы с файлами
#include <fcntl.h>      //для работы с файлами
#include <sys\stat.h>   //для работы с файлами

#include "Unit1.h"
#include "Unit2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;


//=============================================================================
//..................... объявления глобальных переменных ......................
//=============================================================================

#define BUFSIZE 255     //ёмкость буфера

unsigned char bufrd[BUFSIZE], bufwr[BUFSIZE]; //приёмный и передающий буферы

//---------------------------------------------------------------------------

HANDLE COMport;		//дескриптор порта

//структура OVERLAPPED необходима для асинхронных операций, при этом для операции чтения и записи нужно объявить разные структуры
//эти структуры необходимо объявить глобально, иначе программа не будет работать правильно
OVERLAPPED overlapped;		//будем использовать для операций чтения (см. поток ReadThread)
OVERLAPPED overlappedwr;       	//будем использовать для операций записи (см. поток WriteThread)

//---------------------------------------------------------------------------

int handle;             	//дескриптор для работы с файлом с помощью библиотеки <io.h>

//---------------------------------------------------------------------------

bool fl=0;	//флаг, указывающий на успешность операций записи (1 - успешно, 0 - не успешно)

unsigned long counter;	//счётчик принятых байтов, обнуляется при каждом открытии порта


//=============================================================================
//.............................. объявления функций ...........................
//=============================================================================

void COMOpen(void);             //открыть порт
void COMClose(void);            //закрыть порт

//=============================================================================
//.............................. объявления потоков ...........................
//=============================================================================

//---------------------------------------------------------------------------

//поток для чтения последовательности байтов из COM-порта в буфер
class ReadThread : public TThread
{
 private:
        void __fastcall Printing();	//вывод принятых байтов на экран и в файл
 protected:
        void __fastcall Execute();	//основная функция потока
 public:
        __fastcall ReadThread(bool CreateSuspended);	//конструктор потока
};

//---------------------------------------------------------------------------

//поток для записи последовательности байтов из буфера в COM-порт
class WriteThread : public TThread
{
private:
 	void __fastcall Printing();	//вывод состояния на экран
protected:
        void __fastcall Execute();      //основная функция потока
public:
        __fastcall WriteThread(bool CreateSuspended);	//конструктор потока
};

//---------------------------------------------------------------------------


//=============================================================================
//.............................. реализация потоков ...........................
//=============================================================================

//-----------------------------------------------------------------------------
//............................... поток ReadThead .............................
//-----------------------------------------------------------------------------

ReadThread *reader;     //объект потока ReadThread
       DWORD btr;
//---------------------------------------------------------------------------

//конструктор потока ReadThread, по умолчанию пустой
__fastcall ReadThread::ReadThread(bool CreateSuspended) : TThread(CreateSuspended)
{}

//---------------------------------------------------------------------------

//главная функция потока, реализует приём байтов из COM-порта
void __fastcall ReadThread::Execute()
{
 COMSTAT comstat;		//структура текущего состояния порта, в данной программе используется для определения количества принятых в порт байтов
 DWORD  temp, mask, signal;	//переменная temp используется в качестве заглушки

 overlapped.hEvent = CreateEvent(NULL, true, true, NULL);	//создать сигнальный объект-событие для асинхронных операций
 SetCommMask(COMport, EV_RXCHAR);                   	        //установить маску на срабатывание по событию приёма байта в порт
 while(!Terminated)						//пока поток не будет прерван, выполняем цикл
  {
   WaitCommEvent(COMport, &mask, &overlapped);               	//ожидать события приёма байта (это и есть перекрываемая операция)
   signal = WaitForSingleObject(overlapped.hEvent, INFINITE);	//приостановить поток до прихода байта
   if(signal == WAIT_OBJECT_0)				        //если событие прихода байта произошло
    {
     if(GetOverlappedResult(COMport, &overlapped, &temp, true)) //проверяем, успешно ли завершилась перекрываемая операция WaitCommEvent
      if((mask & EV_RXCHAR)!=0)					//если произошло именно событие прихода байта
       {
        ClearCommError(COMport, &temp, &comstat);		//нужно заполнить структуру COMSTAT
        btr = comstat.cbInQue;                          	//и получить из неё количество принятых байтов
        if(btr)                         			//если действительно есть байты для чтения
        {
         ReadFile(COMport, bufrd, btr, &temp, &overlapped);     //прочитать байты из порта в буфер программы
         counter+=btr;                                          //увеличиваем счётчик байтов
         //Synchronize(Printing);                     		//вызываем функцию для вывода данных на экран и в файл

         //Synchronize(Printing);
        }
       }
    }
  }
 CloseHandle(overlapped.hEvent);		//перед выходом из потока закрыть объект-событие
}

//-----------------------------------------------------------------------------
//............................... поток WriteThead ............................
//-----------------------------------------------------------------------------

WriteThread *writer;     //объект потока WriteThread

//---------------------------------------------------------------------------

//конструктор потока WriteThread, по умолчанию пустой
__fastcall WriteThread::WriteThread(bool CreateSuspended) : TThread(CreateSuspended)
{}

//---------------------------------------------------------------------------

//главная функция потока, выполняет передачу байтов из буфера в COM-порт
void __fastcall WriteThread::Execute()
{
 DWORD temp, signal;	//temp - переменная-заглушка

 overlappedwr.hEvent = CreateEvent(NULL, true, true, NULL);   	  //создать событие

 while(!Terminated)     //пока поток не будет завершён, выполнять цикл
 {
 //strlen(bufwr)
  WriteFile(COMport, bufwr, 2, &temp, &overlappedwr);  //записать байты в порт (перекрываемая операция!)
  signal = WaitForSingleObject(overlappedwr.hEvent, INFINITE);	  //приостановить поток, пока не завершится перекрываемая операция WriteFile
  if((signal == WAIT_OBJECT_0) && (GetOverlappedResult(COMport, &overlappedwr, &temp, true))) fl = true; //если операция завершилась успешно, установить соответствующий флажок
  else fl = false;

  //Synchronize(Printing);	//вывести состояние операции в строке состояния
  writer->Suspend();		//приостановить поток записи в порт, пока он не потребуется снова
 }
 CloseHandle(overlappedwr.hEvent);		//перед выходом из потока закрыть объект-событие
}

//---------------------------------------------------------------------------

//вывод состояния передачи данных на экран
void __fastcall WriteThread::Printing()
{
 if(!fl)	//проверяем состояние флажка
  {
   Form1->StatusBar1->Panels->Items[0]->Text  = "Ошибка передачи";
   return;
  }
 Form1->StatusBar1->Panels->Items[0]->Text  = "Передача прошла успешно";

}

//---------------------------------------------------------------------------


//=============================================================================
//............................. элементы формы ................................
//=============================================================================

//---------------------------------------------------------------------------

//конструктор формы, обычно в нём выполняется инициализация элементов формы
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner)
{
 //инициализация элементов формы при запуске программы

 Form1->Button1->Enabled = false;
 Form1->Button2->Enabled = false;
   Form1->Button3->Enabled = false;
   Form1->Button4->Enabled = false;
   Form1->TrackBar1-> Enabled = false;

}

//---------------------------------------------------------------------------

//обработчик нажатия на кнопку "Открыть порт"
void __fastcall TForm1::SpeedButton1Click(TObject *Sender)
{
 if(SpeedButton1->Down)
  {
   COMOpen();                   //если кнопка нажата - открыть порт

   //показать/спрятать элементы на форме
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
   Form1->SpeedButton1->Caption = "Закрыть порт";	//сменить надпись на кнопке

   counter = 0;	//сбрасываем счётчик байтов
  }

 else
  {
   COMClose();                  //если кнопка отжата - закрыть порт

   Form1->SpeedButton1->Caption = "Открыть порт";	//сменить надпись на кнопке
   Form1->StatusBar1->Panels->Items[0]->Text = "";	//очистить первую колонку строки состояния

   //показать/спрятать элементы на форме
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

//обработчик закрытия формы
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
 if(reader)reader->Terminate(); 	//завершить поток чтения из порта, проверка if(reader) обязательна, иначе возникают ошибки
 if(writer)writer->Terminate();         //завершить поток записи в порт, проверка if(writer) обязательна, иначе возникают ошибки
 if(COMport)CloseHandle(COMport);       //закрыть порт
 if(handle)close(handle);               //закрыть файл, в который велась запись принимаемых данных
}
//---------------------------------------------------------------------------


//кнопка "Передать"
void __fastcall TForm1::Button1Click(TObject *Sender)
{
        memset(bufwr,0,BUFSIZE);  //очистить программный передающий буфер, чтобы данные не накладывались друг на друга
        PurgeComm(COMport, PURGE_TXCLEAR);  //очистить передающий буфер порта

        if (Form1->Edit1->Text!="")
        {
        //strcpy(bufwr,Form1->Edit1->Text.c_str());      //занести в программный передающий буфер строку из Edit1
        bufwr[0] = StrToInt(Form1->Edit1->Text);
        writer->Resume();	//активировать поток записи в порт
        }
        else
        {
            bufwr[0] = 0xAB;
            //bufwr[1] = 10;
            //bufwr[2] = 20;
            //bufwr[3] = 30;
            writer->Resume();	//активировать поток записи в порт
        }
}


//---------------------------------------------------------------------------

//=============================================================================
//........................... реализации функций ..............................
//=============================================================================

//---------------------------------------------------------------------------

//функция открытия и инициализации порта
void COMOpen()
{
 char n_com[16];
 String portname;     	 //имя порта (например, "COM1", "COM2" и т.д.)
 DCB dcb;                //структура для общей инициализации порта DCB
 COMMTIMEOUTS timeouts;  //структура для установки таймаутов

 //portname = Form1->ComboBox1->Text;	//получить имя выбранного порта
 strcpy(n_com,("\\\\.\\"+Form1->ComboBox1->Text).c_str());

 //открыть порт, для асинхронных операций обязательно нужно указать флаг FILE_FLAG_OVERLAPPED
 //COMport = CreateFile(portname.c_str(),GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
 COMport = CreateFile(n_com,GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
 //здесь:
 // - portname.c_str() - имя порта в качестве имени файла, c_str() преобразует строку типа String в строку в виде массива типа char, иначе функция не примет
 // - GENERIC_READ | GENERIC_WRITE - доступ к порту на чтение/записть
 // - 0 - порт не может быть общедоступным (shared)
 // - NULL - дескриптор порта не наследуется, используется дескриптор безопасности по умолчанию
 // - OPEN_EXISTING - порт должен открываться как уже существующий файл
 // - FILE_FLAG_OVERLAPPED - этот флаг указывает на использование асинхронных операций
 // - NULL - указатель на файл шаблона не используется при работе с портами

 if(COMport == INVALID_HANDLE_VALUE)            //если ошибка открытия порта
  {
   Form1->SpeedButton1->Down = false;           //отжать кнопку
   Form1->StatusBar1->Panels->Items[0]->Text = "Не удалось открыть порт";       //вывести сообщение в строке состояния
   return;
  }

 //инициализация порта

 dcb.DCBlength = sizeof(DCB); 	//в первое поле структуры DCB необходимо занести её длину, она будет использоваться функциями настройки порта для контроля корректности структуры

 //считать структуру DCB из порта
 if(!GetCommState(COMport, &dcb))	//если не удалось - закрыть порт и вывести сообщение об ошибке в строке состояния
  {
   COMClose();
   Form1->StatusBar1->Panels->Items[0]->Text  = "Не удалось считать DCB";
   return;
  }

 //инициализация структуры DCB
 dcb.BaudRate = StrToInt("115200");       //задаём скорость передачи 115200 бод
 dcb.fBinary = TRUE;                                    //включаем двоичный режим обмена
 dcb.fOutxCtsFlow = FALSE;                              //выключаем режим слежения за сигналом CTS
 dcb.fOutxDsrFlow = FALSE;                              //выключаем режим слежения за сигналом DSR
 dcb.fDtrControl = DTR_CONTROL_DISABLE;                 //отключаем использование линии DTR
 dcb.fDsrSensitivity = FALSE;                           //отключаем восприимчивость драйвера к состоянию линии DSR
 dcb.fNull = FALSE;                                     //разрешить приём нулевых байтов
 dcb.fRtsControl = RTS_CONTROL_DISABLE;                 //отключаем использование линии RTS
 dcb.fAbortOnError = FALSE;                             //отключаем остановку всех операций чтения/записи при ошибке
 dcb.ByteSize = 8;                                      //задаём 8 бит в байте
 dcb.Parity = 0;                                        //отключаем проверку чётности
 dcb.StopBits = 0;                                      //задаём один стоп-бит

 //загрузить структуру DCB в порт
 if(!SetCommState(COMport, &dcb))	//если не удалось - закрыть порт и вывести сообщение об ошибке в строке состояния
  {
   COMClose();
   Form1->StatusBar1->Panels->Items[0]->Text  = "Не удалось установить DCB";
   return;
  }

 //установить таймауты
 timeouts.ReadIntervalTimeout = 0;	 	//таймаут между двумя символами
 timeouts.ReadTotalTimeoutMultiplier = 0;	//общий таймаут операции чтения
 timeouts.ReadTotalTimeoutConstant = 0;         //константа для общего таймаута операции чтения
 timeouts.WriteTotalTimeoutMultiplier = 0;      //общий таймаут операции записи
 timeouts.WriteTotalTimeoutConstant = 0;        //константа для общего таймаута операции записи

 //записать структуру таймаутов в порт
 if(!SetCommTimeouts(COMport, &timeouts))	//если не удалось - закрыть порт и вывести сообщение об ошибке в строке состояния
  {
   COMClose();
   Form1->StatusBar1->Panels->Items[0]->Text  = "Не удалось установить тайм-ауты";
   return;
  }

 //установить размеры очередей приёма и передачи
 SetupComm(COMport,2000,2000);
   
 //создать или открыть существующий файл для записи принимаемых данных
 handle = open("test.txt", O_CREAT | O_APPEND | O_BINARY | O_WRONLY, S_IREAD | S_IWRITE);

 if(handle==-1)		//если произошла ошибка открытия файла
  {
   Form1->StatusBar1->Panels->Items[1]->Text = "Ошибка открытия файла";	//вывести сообщение об этом в командной строке
  }
 else { Form1->StatusBar1->Panels->Items[0]->Text = "Файл открыт успешно"; } //иначе вывести в строке состояния сообщение об успешном открытии файла 

 PurgeComm(COMport, PURGE_RXCLEAR);	//очистить принимающий буфер порта

 reader = new ReadThread(false);	//создать и запустить поток чтения байтов
 reader->FreeOnTerminate = true;        //установить это свойство потока, чтобы он автоматически уничтожался после завершения

 writer = new WriteThread(true);               //создать поток записи данных в порт
 writer->FreeOnTerminate = true;                //установить это свойство, чтобы поток автоматически уничтожался после завершения

}

//---------------------------------------------------------------------------

//функция закрытия порта
void COMClose()
{
 //если поток записи существует, подать ему команду на завершение и запустить его, чтобы он выполнил завершение;
 if(writer) //проверка if(writer) обязательна, иначе возникают ошибки;
  {
   writer->Terminate();
   writer->Resume();
  }

 if(reader)reader->Terminate();         //если поток чтения работает, завершить его; проверка if(reader) обязательна, иначе возникают ошибки
 CloseHandle(COMport);                  //закрыть порт
 COMport=0;				//обнулить переменную для дескриптора порта
 close(handle);				//закрыть файл для записи принимаемых данных
 handle=0;				//обнулить переменную для дескриптора файла
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
        memset(bufwr,0,BUFSIZE);  //очистить программный передающий буфер, чтобы данные не накладывались друг на друга
        PurgeComm(COMport, PURGE_TXCLEAR);  //очистить передающий буфер порта

        if (Form1->Edit2->Text!="")
        {
        bufwr[0]=0xF0;
        //strcpy(bufwr,Form1->Edit1->Text.c_str());      //занести в программный передающий буфер строку из Edit1
        bufwr[1] = StrToInt(Form1->Edit2->Text);
        //bufwr[0] = bufwr[0] | 0x00;
        writer->Resume();	//активировать поток записи в порт
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
    memset(bufwr,0,BUFSIZE);  //очистить программный передающий буфер, чтобы данные не накладывались друг на друга
        PurgeComm(COMport, PURGE_TXCLEAR);  //очистить передающий буфер порта

        if (Form1->Edit2->Text!="")
        {
        bufwr[0]=0xF5;
        bufwr[1] = StrToInt(Form1->Edit2->Text);
        writer->Resume();	//активировать поток записи в порт
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
        memset(bufwr,0,BUFSIZE);  //очистить программный передающий буфер, чтобы данные не накладывались друг на друга
        PurgeComm(COMport, PURGE_TXCLEAR);  //очистить передающий буфер порта

        if (Form1->Edit2->Text!="")
        {
        bufwr[0]=0xF2;
        bufwr[1] = StrToInt(Form1->Edit2->Text);
        writer->Resume();	//активировать поток записи в порт
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TrackBar1Change(TObject *Sender)
{
   Form1->Edit4->Text=IntToStr(Form1->TrackBar1->Position);
   memset(bufwr,0,BUFSIZE);  //очистить программный передающий буфер, чтобы данные не накладывались друг на друга
   PurgeComm(COMport, PURGE_TXCLEAR);  //очистить передающий буфер порта

   //if (Form1->Edit2->Text!="")
   {
        bufwr[0]=0xF3;
        bufwr[1] = StrToInt(Form1->Edit4->Text);
        writer->Resume();	//активировать поток записи в порт
   }
}



void __fastcall TForm1::Button5Click(TObject *Sender)
{
     memset(bufwr,0,BUFSIZE);  //очистить программный передающий буфер, чтобы данные не накладывались друг на друга
        PurgeComm(COMport, PURGE_TXCLEAR);  //очистить передающий буфер порта

        bufwr[0]=0xF5;
        bufwr[1] = 1;
        writer->Resume();	//активировать поток записи в порт
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
        memset(bufwr,0,BUFSIZE);  //очистить программный передающий буфер, чтобы данные не накладывались друг на друга
        PurgeComm(COMport, PURGE_TXCLEAR);  //очистить передающий буфер порта

        bufwr[0]=0xF5;
        bufwr[1] = 0;
        writer->Resume();	//активировать поток записи в порт
}
//---------------------------------------------------------------------------

