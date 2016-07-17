#include <cstring>
//#include <cstdio>   // для printf

#include "src_CMN.h"

// ----- ----- -----

char* CMN_c_Substr(char* c_Dst, const char* cnst_c_Src, unsigned int ui_Pos, unsigned int ui_Cnt)
{
  if (!c_Dst || !cnst_c_Src || (strlen(cnst_c_Src) < ui_Pos))
  {
    return NULL;
  }

  unsigned int ui_Len = 0;
  ui_Len = strlen(cnst_c_Src + ui_Pos);
  if (ui_Len > ui_Cnt)
  {
    ui_Len = ui_Cnt;
  }

  strncpy(c_Dst, cnst_c_Src + ui_Pos, ui_Len);
  c_Dst[ui_Len] = '\0';
    
  return c_Dst;
}

// ----- ----- -----
