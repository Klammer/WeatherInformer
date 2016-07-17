#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "src_WND.h"

void WND_Init(WND_Info* wi_WND)
{
  wi_WND->i_DepthB = 4;
  wi_WND->i_Depth = wi_WND->i_DepthB << 3;


  wi_WND->dspl_Root = XOpenDisplay(NULL);
  int int_ScreenRoot = XDefaultScreen(wi_WND->dspl_Root);
  Window wnd_Root = XDefaultRootWindow(wi_WND->dspl_Root);

  wi_WND->i_Width = XDisplayWidth(wi_WND->dspl_Root, int_ScreenRoot);
  wi_WND->i_Height = XDisplayHeight(wi_WND->dspl_Root, int_ScreenRoot);

  XVisualInfo xvi_Informer;
  XMatchVisualInfo(wi_WND->dspl_Root, int_ScreenRoot, wi_WND->i_Depth, TrueColor, &xvi_Informer);

  wi_WND->xswa_WND;
  wi_WND->xswa_WND.colormap = XCreateColormap(wi_WND->dspl_Root, wnd_Root, xvi_Informer.visual, AllocNone);
  wi_WND->xswa_WND.border_pixel = None;
  wi_WND->xswa_WND.background_pixel = None;
  wi_WND->xswa_WND.override_redirect = True;

  wi_WND->wnd_WND = XCreateWindow(wi_WND->dspl_Root, wnd_Root,
                               0, 0, wi_WND->i_Width, wi_WND->i_Height, 0,
                               wi_WND->i_Depth, InputOutput, xvi_Informer.visual,
                               CWOverrideRedirect | CWColormap | CWBorderPixel | CWBackPixel,
                               &wi_WND->xswa_WND);
  Atom atm_DelWindow = XInternAtom(wi_WND->dspl_Root, "WM_DELETE_WINDOW", false);
  XSetWMProtocols(wi_WND->dspl_Root, wi_WND->wnd_WND, &atm_DelWindow, 1);

  XMapWindow(wi_WND->dspl_Root, wi_WND->wnd_WND);
  XLowerWindow(wi_WND->dspl_Root, wi_WND->wnd_WND);

  XSelectInput(wi_WND->dspl_Root, wi_WND->wnd_WND, ButtonPressMask);
  // ButtonPressMask - работает прекрасно на клик мыши, ловим ButtonPress
  // KeyPressMask не работает - видимо, окно всё же не получает фокус с настройками как выше
  // ExposureMask - хз как работает => согласно докам, устанавливать вручную не нужно

  wi_WND->gc_WND = XCreateGC(wi_WND->dspl_Root, wi_WND->wnd_WND, 0, NULL);
  wi_WND->vsl_Root = DefaultVisual(wi_WND->dspl_Root, 0);
  
  Font fnt_Font = XLoadFont(wi_WND->dspl_Root, "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso10646-1");
  //Font fnt_Font = XLoadFont(wi_WND->dspl_Root, "-misc-fixed-medium-r-normal--18-120-100-100-c-90-koi8-r");
  //Font fnt_Font = XLoadFont(wi_WND->dspl_Root, "-misc-fixed-medium-r-normal--10-100-75-75-c-60-koi8-r");
  //Font fnt_Font = XLoadFont(wi_WND->dspl_Root, "-misc-fixed-medium-r-normal--13-120-75-75-c-70-koi8-r");
  XSetFont(wi_WND->dspl_Root, wi_WND->gc_WND, fnt_Font);
}

void WND_SetForeground(WND_Info* wi_WND, unsigned char uc_R, unsigned char uc_G, unsigned char uc_B)
{
  XColor xc_Color;
  xc_Color.red = ((unsigned short)uc_R << 8) | uc_R;
  xc_Color.green = ((unsigned short)uc_G << 8) | uc_G;
  xc_Color.blue = ((unsigned short)uc_B << 8) | uc_B;
  xc_Color.flags = DoRed | DoGreen | DoBlue;
  XAllocColor(wi_WND->dspl_Root, wi_WND->xswa_WND.colormap, &xc_Color);
  XSetForeground(wi_WND->dspl_Root, wi_WND->gc_WND, xc_Color.pixel);
}

void WND_DrawString(WND_Info* wi_WND,  int i_X, int i_Y, char* c_Buf, int i_LenBuf)
{
  XDrawString(wi_WND->dspl_Root, wi_WND->wnd_WND, wi_WND->gc_WND, i_X, i_Y, c_Buf, i_LenBuf);
}

void WND_DrawString16(WND_Info* wi_WND,  int i_X, int i_Y, XChar2b* xc2b_Buf, int i_LenBuf)
{
  XDrawString16(wi_WND->dspl_Root, wi_WND->wnd_WND, wi_WND->gc_WND, i_X, i_Y, xc2b_Buf, i_LenBuf);
}
