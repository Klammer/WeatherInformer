#ifndef __BGWNDINIT_FUNCS_H
#define __BGWNDINIT_FUNCS_H

// ----- ----- -----

/*
  Структура окна (всех аспектов, которые могут понадобиться).
*/
struct WND_Info
{
  int i_DepthB;
  int i_Depth;

  Display* dspl_Root;
  Window wnd_WND;
  int i_Width;
  int i_Height;
  XSetWindowAttributes xswa_WND;
  GC gc_WND;
  Visual* vsl_Root;
};

// ----- ----- -----

/*
  Инициализация окна. Заполняет структуру.
*/
void WND_Init(WND_Info* wi_WND);

/*
  Установка цвета пера.
*/
void WND_SetForeground(WND_Info* wi_WND, unsigned char uc_R, unsigned char uc_G, unsigned char uc_B);

/*
  Вывод строки utf8.
*/
void WND_DrawString(WND_Info* wi_WND,  int i_X, int i_Y, char* c_Buf, int i_LenBuf);

/*
  Вывод строки XChar2b (iso10646-1).
*/
void WND_DrawString16(WND_Info* wi_WND,  int i_X, int i_Y, XChar2b* xc2b_Buf, int i_LenBuf);

// ----- ----- -----

#endif
