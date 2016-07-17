#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "src_GMP.h"
#include "src_WND.h"

// ----- ----- -----

// ----- перевод текста из utf8 в XChar2b (iso10646-1)

int utf8toXChar2b(XChar2b* xc2b_Out, int i_LenOut, const char* c_In, int i_LenIn, bool bl_CutN){
  int i_I, i_J;
  for(i_I = 0, i_J = 0; (i_I < i_LenIn) && (i_J < i_LenOut); i_I++)
  // перебираем входную строку, пока в ней остаются символы,
  // и остаётся место в выходной строке
  {
    unsigned char uc_Curr = c_In[i_I];    // текущий байт входной строки
    if (uc_Curr < 128)
    {
      // если текущий байт в рамках таблицы ASCII
      if (bl_CutN && (uc_Curr == 10))
        // текущий байт - символ новой строки, и включен режим пропуска
        // символа новой строки, пропускаем
        continue;
      else
      {
        xc2b_Out[i_J].byte1 = 0;
        xc2b_Out[i_J].byte2 = uc_Curr; 
        i_J++;
      }
    }
    else if (uc_Curr < 0xC0)
    {
      // текущий байт - "не первый" байт многобайтового символа;
      // 2-х и 3-х байтовые символы обрабатываются ниже, значит
      // если получили такую ситуацию - символ 4-х и более байтовый
      // и обработать его не можем, пропускаем
      continue;
    } else switch (uc_Curr & 0xF0)
    {
      case 0xF0:
        // текущий байт - первый байт 4-х и более байтового символа,
        // отобразить его в рамках XChar2b не можем (т.к. он использует
        // более 16 значащих бит), пропускаем
        continue;
      case 0xE0:
        // текущий байт - первый байт 3-х байтового символа
        // (использует 16 значащих бит: 4 + 6 + 6)
        if (i_LenIn < i_I + 2)
          // если во входной строке осталось меньше, чем 3 байта,
          // то нормально декодировать символ не сможем, выход
          return i_J;
        i_I++;
        xc2b_Out[i_J].byte1 = (uc_Curr << 4) | ((c_In[i_I] >> 2) & 0x0F);
        uc_Curr = c_In[i_I];
        i_I++;
        xc2b_Out[i_J].byte2 = (uc_Curr << 6) | (c_In[i_I] & 0x3F);
        i_J++;
        break; 
      default:
        // текущий байт - первый байт 2-х байтового символа
        // (использует 11 значащих бит: 5 + 6)
        if (i_LenIn < i_I + 1)
          // если во входной строке осталось меньше, чем 2 байта,
          // то нормально декодировать символ не сможем, выход
          return i_J;
        xc2b_Out[i_J].byte1 = (uc_Curr >> 2) & 0x07;
        i_I++;
        xc2b_Out[i_J].byte2 = (uc_Curr << 6) | (c_In[i_I] & 0x3F);
        i_J++;
        break;
		}
	}
	// возвращаем длину (в символах) получившейся строки XChar2b
	return i_J;
}

// ----- конец функции перевода текста из utf8 в XChar2b (iso10646-1)

int TestLeapYear(int i_yyyy)
{
  if ((i_yyyy % 400 == 0) || ((i_yyyy % 100 != 0) && (i_yyyy % 4 == 0)))
    return 1;
  return 0;
}

int YYYY2Num(int i_yyyy)
{
  return (i_yyyy - 1) * 365 + (i_yyyy - 1) / 4 - (i_yyyy - 1) / 100 + (i_yyyy - 1) / 400;
}

int MM2Num(int i_yyyy, int i_mm)
{
  int i_month_days_pre[][12]	= {{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
                                 {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};
  return i_month_days_pre[TestLeapYear(i_yyyy)][i_mm - 1];
}

int Date2Num(int i_yyyy, int i_mm, int i_dd)
{
  return YYYY2Num(i_yyyy) + MM2Num(i_yyyy, i_mm) + i_dd;
}

// ----- обновление содержимого окна

int GUI_Renew(
  WND_Info* wi_WND,
  XImage* ximg_Image,
  unsigned int* ui_Image,
  unsigned int ui_IWidth,
  unsigned int ui_IHeight,
  int* i_Iter
)
{
  // количество заполняемых отсчётов погоды
  unsigned int ui_CntNum = 24;
  // массив погодных отсчётов
  GMP_WCount wc_Cnts[24];
  // инициализируем массив погодных отсчётов
  for(int i_I = 0; i_I < ui_CntNum; i_I++)
    GMP_InitWCount(wc_Cnts[i_I]);

  // позиция информера
  int i_X = 825;//30;
  int i_Y = 300;//10;
  // счётчик строк и высота одной строки
  int i_LCntr = 0;
  int i_LH = 12;
  // размер табуляции
  int i_LInt = 5;

  // страница с прогнозом
  char c_URL[255] = "https://www.gismeteo.ru/city/hourly/4690/#tab_wdaily1";

  // заполняем массив погодных отсчётов
  GMP_i_ReadGMURL(c_URL, ui_CntNum, wc_Cnts);

  // очищаем область отрисовки прогноза
  XClearArea(wi_WND->dspl_Root, wi_WND->wnd_WND, 30, 0, 675, 335, false);

  // устанавливаем пастельный цвет пера
  WND_SetForeground(wi_WND, 65, 125, 65);

  // строка utf8
  char c_Buf[256];
  // строка XChar2b (iso10646-1)
  XChar2b xc2b_Buf[128];
  int i_LenXC2bBuf;
  
  // строки шаблонов для вывода
  char c_Mask_00[] = "PID: %d";
  char c_Mask_01[] = "%s, %.2u.%.2u.%.4u";
  char c_Mask_01_01[] = "%.2u.%.2u.%.4u -";
  char c_Mask_02[] = "%.2u:00   %i (%i),";
  char c_Mask_03[] = "ветер %s,";
  char c_Mask_04[] = "%u м/с,";
  char c_Mask_05[] = "%s";

  // выводим PID процесса
  snprintf(c_Buf, sizeof(c_Buf), "PID: %d", getpid());
  WND_DrawString(wi_WND, i_X, i_Y + i_LCntr * i_LH, c_Buf, strlen(c_Buf));
  i_LCntr += 2;

  // текущее время
  time_t tt_Now;
  tm* tm_Now;
  int i_Now;
  int i_DCurr;
  
  time(&tt_Now);
  tm_Now = localtime(&tt_Now);
  i_Now = Date2Num(tm_Now->tm_year + 1900, tm_Now->tm_mon + 1, tm_Now->tm_mday);

  bool bl_Today = true;
  bool bl_Tomorrow = true;
  bool bl_AfterTomorrow = true;

  bool bl_Future = true;

  // вывод отсчётов
  
  for(int i_I = 0; i_I < ui_CntNum; i_I++)
  {
    i_DCurr = Date2Num(wc_Cnts[i_I].us_Year, wc_Cnts[i_I].us_Month, wc_Cnts[i_I].us_Day);

    if ((i_Now == i_DCurr) && bl_Today)
    {
      // устанавливаем яркий цвет пера
      WND_SetForeground(wi_WND, 125, 255, 125);

      i_LCntr++;
      bl_Today = false;
      snprintf(c_Buf, sizeof(c_Buf), c_Mask_01, "сегодня", wc_Cnts[i_I].us_Day, wc_Cnts[i_I].us_Month, wc_Cnts[i_I].us_Year);
      i_LenXC2bBuf = utf8toXChar2b(xc2b_Buf, 128, c_Buf, strlen(c_Buf), true);
      WND_DrawString16(wi_WND, i_X, i_Y + i_LCntr * i_LH, xc2b_Buf, i_LenXC2bBuf);
      i_LCntr+=2;

      // устанавливаем пастельный цвет пера
      WND_SetForeground(wi_WND, 65, 125, 65);
    }
    else if ((i_DCurr - i_Now == 1) && bl_Tomorrow)
    {
      i_LCntr++;
      bl_Tomorrow = false;
      snprintf(c_Buf, sizeof(c_Buf), c_Mask_01, "завтра", wc_Cnts[i_I].us_Day, wc_Cnts[i_I].us_Month, wc_Cnts[i_I].us_Year);
      i_LenXC2bBuf = utf8toXChar2b(xc2b_Buf, 128, c_Buf, strlen(c_Buf), true);
      WND_DrawString16(wi_WND, i_X, i_Y + i_LCntr * i_LH, xc2b_Buf, i_LenXC2bBuf);
      i_LCntr+=2;
    }
    else if ((i_DCurr - i_Now == 2) && bl_AfterTomorrow)
    {
      i_LCntr++;
      bl_AfterTomorrow = false;
      snprintf(c_Buf, sizeof(c_Buf), c_Mask_01, "послезавтра", wc_Cnts[i_I].us_Day, wc_Cnts[i_I].us_Month, wc_Cnts[i_I].us_Year);
      i_LenXC2bBuf = utf8toXChar2b(xc2b_Buf, 128, c_Buf, strlen(c_Buf), true);
      WND_DrawString16(wi_WND, i_X, i_Y + i_LCntr * i_LH, xc2b_Buf, i_LenXC2bBuf);
      i_LCntr+=2;
    }

    if ((wc_Cnts[i_I].us_Time > tm_Now->tm_hour) && (bl_Future))
    {
      bl_Future = false;
      // устанавливаем яркий цвет пера
      WND_SetForeground(wi_WND, 125, 255, 125);
    }

    snprintf(c_Buf, sizeof(c_Buf), c_Mask_02, wc_Cnts[i_I].us_Time, wc_Cnts[i_I].ss_Temp, wc_Cnts[i_I].ss_TempPerc);
    i_LenXC2bBuf = utf8toXChar2b(xc2b_Buf, 128, c_Buf, strlen(c_Buf), true);
    WND_DrawString16(wi_WND, i_X + i_LInt, i_Y + i_LCntr * i_LH, xc2b_Buf, i_LenXC2bBuf);

    snprintf(c_Buf, sizeof(c_Buf), c_Mask_03, wc_Cnts[i_I].c_WindTmp);
    i_LenXC2bBuf = utf8toXChar2b(xc2b_Buf, 128, c_Buf, strlen(c_Buf), true);
    WND_DrawString16(wi_WND, i_X + i_LInt + 120, i_Y + i_LCntr * i_LH, xc2b_Buf, i_LenXC2bBuf);

    snprintf(c_Buf, sizeof(c_Buf), c_Mask_04, wc_Cnts[i_I].us_WindS);
    i_LenXC2bBuf = utf8toXChar2b(xc2b_Buf, 128, c_Buf, strlen(c_Buf), true);
    WND_DrawString16(wi_WND, i_X + i_LInt + 190, i_Y + i_LCntr * i_LH, xc2b_Buf, i_LenXC2bBuf);

    snprintf(c_Buf, sizeof(c_Buf), c_Mask_05, wc_Cnts[i_I].c_SkyTmp);
    i_LenXC2bBuf = utf8toXChar2b(xc2b_Buf, 128, c_Buf, strlen(c_Buf), true);
    WND_DrawString16(wi_WND, i_X + i_LInt + 240, i_Y + i_LCntr * i_LH, xc2b_Buf, i_LenXC2bBuf);

    i_LCntr++;
  }
}

// ----- конец функции обновления содержимого окна

// ----- ПОТОК ОТРИСОВКИ GUI

void* main_GUI(void* arg)
{
  WND_Info* wi_WND = (WND_Info*)arg;
// ----- установки для GUI

  bool bl_IsRun = true;
  XEvent xevnt_E;

  WND_Init(wi_WND);

// ----- конец установок для GUI

// ----- вывод изображения

  unsigned int ui_IWidth = 200;
  unsigned int ui_IHeight = 200;
  unsigned int* ui_Image = (unsigned int*)malloc(ui_IWidth * ui_IHeight * wi_WND->i_DepthB);

  XImage* ximg_Image = XCreateImage(wi_WND->dspl_Root, wi_WND->vsl_Root, wi_WND->i_Depth, ZPixmap, 0,
                                    (char*)ui_Image, ui_IWidth, ui_IHeight, wi_WND->i_Depth, 0);

  int i_Iter = 0;

// ----- конец вывода изображения

  //XFlush(wi_WND->dspl_Root);

  //printf("Start...\n");
  //fflush(NULL);

  while (bl_IsRun)
  {
    XNextEvent(wi_WND->dspl_Root, &xevnt_E);

    switch(xevnt_E.type)
    {
      case ClientMessage:
        if (xevnt_E.xclient.message_type == XInternAtom(wi_WND->dspl_Root, "WM_PROTOCOLS", true) &&
            xevnt_E.xclient.data.l[0] == (long)XInternAtom(wi_WND->dspl_Root, "WM_DELETE_WINDOW", true))
          // закрытие окна крестиком; на всякий случай пусть останется
          raise(SIGTERM);
        else
          switch(xevnt_E.xclient.data.l[0])
          {
            case 158500:
              //printf("==| Stop\n");
              //fflush(NULL);
              bl_IsRun = false;
              break;
            case 158501:
              //printf("==| Usr 1\n");
              //fflush(NULL);
              GUI_Renew(wi_WND, ximg_Image, ui_Image, ui_IWidth, ui_IHeight, &i_Iter);
              break;
            case 158502:
              //printf("==| Usr 2\n");
              //fflush(NULL);
              //XClearArea(wi_WND->dspl_Root, wi_WND->wnd_WND, 30, 0, 675, 335, false);
              break;
            case 158503:
              //printf("==| Alarm\n");
              //fflush(NULL);
              GUI_Renew(wi_WND, ximg_Image, ui_Image, ui_IWidth, ui_IHeight, &i_Iter);
              //Draw((unsigned int*)ui_Image, ui_IWidth, ui_IHeight, i_Iter++);
              //XPutImage(wi_WND->dspl_Root, wi_WND->wnd_WND, wi_WND->gc_WND, ximg_Image, 0, 0, 550, 5, ui_IWidth, ui_IHeight);
              break;
            default:
              //printf("==| ClientMessage - default\n");
              //fflush(NULL);
              break;
          }
        break;
      case Expose:
        //printf("==| Expose\n");
        //fflush(NULL);
        break;
      case ButtonPress:
        //printf("==| ButtonPress\n");
        //fflush(NULL);
        //raise(SIGTERM);
        break;
      default:
        //printf("==| default\n");
        //fflush(NULL);
      break;
    }
  }

  XDestroyImage(ximg_Image);

  return 0;
}

// ----- КОНЕЦ ПОТОКА ОТРИСОВКИ GUI

// ----- ОСНОВНОЙ ПОТОК ПРИЛОЖЕНИЯ

int main(int argc, char** argv)
{
  bool bl_IsRun = true;
  
  WND_Info wi_WND;

  // -----

  pthread_t pth_GUI;
  pthread_create(&pth_GUI, NULL, main_GUI, &wi_WND);

  // -----

  itimerval itv_Timer;
  itv_Timer.it_interval.tv_sec = 3600;//300;
  itv_Timer.it_interval.tv_usec = 0;
  itv_Timer.it_value.tv_sec = 1;
  itv_Timer.it_value.tv_usec = 0;
  setitimer(ITIMER_REAL, &itv_Timer, NULL);

  // -----

  sigset_t sigs_SigMask;
  sigemptyset(&sigs_SigMask);
  sigaddset(&sigs_SigMask, SIGTERM);
  sigaddset(&sigs_SigMask, SIGUSR1);
  sigaddset(&sigs_SigMask, SIGUSR2);
  sigaddset(&sigs_SigMask, SIGALRM);
  pthread_sigmask(SIG_BLOCK, &sigs_SigMask, NULL);
  int i_Signal;

  // -----

  XClientMessageEvent xcme_Event;
  xcme_Event.type = ClientMessage;
  xcme_Event.format = 32;

  // -----

  while (bl_IsRun)
  {
    sigwait(&sigs_SigMask, &i_Signal);

    //printf("Signal: %i\n", i_Signal);
    //fflush(NULL);

    switch(i_Signal)
    {
      case SIGTERM:
        xcme_Event.data.l[0] = 158500;
        XSendEvent(wi_WND.dspl_Root, wi_WND.wnd_WND, false, 0, (XEvent*)&xcme_Event);
        XFlush(wi_WND.dspl_Root);
        bl_IsRun = false;
        break;
      case SIGUSR1:
        xcme_Event.data.l[0] = 158501;
        XSendEvent(wi_WND.dspl_Root, wi_WND.wnd_WND, false, 0, (XEvent*)&xcme_Event);
        XFlush(wi_WND.dspl_Root);
        break;
      case SIGUSR2:
        xcme_Event.data.l[0] = 158502;
        XSendEvent(wi_WND.dspl_Root, wi_WND.wnd_WND, false, 0, (XEvent*)&xcme_Event);
        XFlush(wi_WND.dspl_Root);
        break;
      case SIGALRM:
        xcme_Event.data.l[0] = 158503;
        XSendEvent(wi_WND.dspl_Root, wi_WND.wnd_WND, false, 0, (XEvent*)&xcme_Event);
        XFlush(wi_WND.dspl_Root);
        break;
      default:
        break;
    }
  }

  // -----

  pthread_join(pth_GUI, NULL);

  return 0;
}

// ----- КОНЕЦ ОСНОВНОГО ПОТОКА ПРИЛОЖЕНИЯ
