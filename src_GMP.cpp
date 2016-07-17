#include <curl/curl.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>

#ifndef NULL
#define NULL 0
#endif

#include "src_CMN.h"
#include "src_GMP.h"

// ----- ----- -----

// ----- нормализация строки

#define CH_TABLE_NUM 2
char GMP_c_ChTable[CH_TABLE_NUM][2][4] = {{{-30, -120, -110, '\0'}, "-"},
                                          {"\n", ""}};

void GMP_Normalize(char* c_Str)
{
  char* c_Occur;
  char* c_BasePos;

  for(int i_I = 0; i_I < CH_TABLE_NUM; i_I++)
  {
    c_BasePos = c_Str;
    do
    {
      c_Occur = strstr(c_BasePos, GMP_c_ChTable[i_I][0]);
      if (c_Occur)
      {
        *c_Occur = GMP_c_ChTable[i_I][1][0];
        strcpy((c_Occur + 1), (c_Occur + strlen(GMP_c_ChTable[i_I][0])));
      }
      c_BasePos = c_Occur + 1;
    } while(c_Occur);
  }
}

// ----- конец нормализации строки

// ----- инициализация структуры

void GMP_InitWCount(GMP_WCount &wc_Object)
{
  wc_Object.us_Year     = 16383;
  wc_Object.us_Month    = 16383;
  wc_Object.us_Day      = 16383;
  wc_Object.us_Time     = 16383;
  wc_Object.s_Sky_len   = 0;
  wc_Object.c_Sky       = NULL;
  wc_Object.ss_Temp     = 16383;
  wc_Object.us_Press    = 16383;
  wc_Object.us_WindT    = 16383;
  wc_Object.us_WindS    = 16383;
  wc_Object.us_Hum      = 16383;
  wc_Object.ss_TempPerc = 16383;
}

// ----- конец инициализации структуры

// ----- чтение URL, заполнение буфера

unsigned int GMP_ui_WriteFunc(char* c_In, unsigned int ui_Size, unsigned int ui_Num, TidyBuffer* tb_Out)
{
  unsigned int ui_TSize;
  ui_TSize = ui_Size * ui_Num;
  tidyBufAppend(tb_Out, c_In, ui_TSize);
  return(ui_TSize);
}
// ----- конец чтения URL, заполнения буфера

// ----- рекурсивный обход и анализ узла с отсчётами

unsigned int GMP_ui_ProcDay(TidyDoc td_Tree, TidyNode tn_Node, unsigned int ui_CntNum, GMP_WCount* wc_Cnts, unsigned int ui_CntCurr)
{
  TidyNode tn_Count;

  TidyAttr ta_Attr;

  TidyNode tn_Param_l1;
  TidyNode tn_Param_l2;
  TidyNode tn_Param_l3;

  TidyBuffer tb_Buff;
  tidyBufInit(&tb_Buff);

  unsigned int ui_Count;  // счётчик отсчётов времени
  char c_BuffStr[255] = "\0";
  char* c_ErrPtr; // указатель на ошибку преобразований строки в число

  const char* c_NodeName, * c_AttrName, * c_AttrVal;
  const char* c_NodeName_l1, * c_AttrName_l1, * c_AttrVal_l1;
  const char* c_NodeName_l2, * c_AttrName_l2, * c_AttrVal_l2;
  const char* c_NodeName_l3, * c_AttrName_l3, * c_AttrVal_l3;

  // перебираем вложенные объекты
  // отсчёты времени (строки таблицы в тэге <tr>...</tr>)
  // - отчётов времени - не более 8;
  // - не должны выйти за границы wc_Cnts
  for (tn_Count = tidyGetChild(tn_Node), ui_Count = 0;
       tn_Count && (ui_Count < 8) && (ui_CntCurr < ui_CntNum);
       tn_Count = tidyGetNext(tn_Count), ui_Count++, ui_CntCurr++)
  {
    // если текущий вложенный объект - тег
    if (c_NodeName = tidyNodeGetName(tn_Count))
    {
      ta_Attr = tidyAttrFirst(tn_Count);

      // если у него есть параметры
      if (c_AttrName = tidyAttrName(ta_Attr))
      {
        c_AttrVal = tidyAttrValue(ta_Attr);

        // если это строка таблицы <tr>...</tr> - то это отсчёт времени
        if (!strcmp(c_NodeName, "tr") and
            !strcmp(c_AttrName, "class") and
            !strncmp(c_AttrVal, "wrow", 4))
        {
          ta_Attr = tidyAttrNext(ta_Attr);

          if (c_AttrName = tidyAttrName(ta_Attr))
          {
// -----    дата, время
            c_AttrVal = tidyAttrValue(ta_Attr);

            if (strlen(c_AttrVal) == 18)
            {
              c_ErrPtr = NULL;
              CMN_c_Substr(c_BuffStr, c_AttrVal, 5, 4);
              wc_Cnts[ui_CntCurr].us_Year = strtoul(c_BuffStr, &c_ErrPtr, 10);
              if (*c_ErrPtr)
              {
                wc_Cnts[ui_CntCurr].us_Year = 32767;
              }
              c_ErrPtr = NULL;
              CMN_c_Substr(c_BuffStr, c_AttrVal, 10, 2);
              wc_Cnts[ui_CntCurr].us_Month = strtoul(c_BuffStr, &c_ErrPtr, 10);
              if (*c_ErrPtr)
              {
                wc_Cnts[ui_CntCurr].us_Month = 32767;
              }
              c_ErrPtr = NULL;
              CMN_c_Substr(c_BuffStr, c_AttrVal, 13, 2);
              wc_Cnts[ui_CntCurr].us_Day = strtoul(c_BuffStr, &c_ErrPtr, 10);
              if (*c_ErrPtr)
              {
                wc_Cnts[ui_CntCurr].us_Day = 32767;
              }
              c_ErrPtr = NULL;
              CMN_c_Substr(c_BuffStr, c_AttrVal, 16, 2);
              wc_Cnts[ui_CntCurr].us_Time = strtoul(c_BuffStr, &c_ErrPtr, 10);
              if (*c_ErrPtr)
              {
                wc_Cnts[ui_CntCurr].us_Time = 32767;
              }
            }
          }
// -----    конец считывания даты, времени

          // перебираем вложенные теги отсчёта - параметры
          for (tn_Param_l1 = tidyGetChild(tn_Count); tn_Param_l1; tn_Param_l1 = tidyGetNext(tn_Param_l1))
          {
            // если вложенный объект - тег (имеет имя)
            if (c_NodeName_l1 = tidyNodeGetName(tn_Param_l1))
            {
              // если это тег <td>...</td>
              if (!strcmp(c_NodeName_l1, "td"))
              {
                ta_Attr = tidyAttrFirst(tn_Param_l1);

                // если у него есть параметры
                if(c_AttrName_l1 = tidyAttrName(ta_Attr))
                {
                  // если первый параметр - класс
                  if (!strcmp(c_AttrName_l1, "class"))
                  {
                    c_AttrVal_l1 = tidyAttrValue(ta_Attr);

                    // если класс - cltext - это облачность
                    if (!strcmp(c_AttrVal_l1, "cltext"))
                    {
                      tn_Param_l2 = tidyGetChild(tn_Param_l1);

                      // если вложенный объект - не тег - то это искомое значение
                      if (!tidyNodeGetName(tn_Param_l2))
                      {
                        tidyNodeGetText(td_Tree, tn_Param_l2, &tb_Buff);
// -----          ----- облачность
                        //printf("%*.0s%s\n", 4, " ", tb_Buff.bp ? (char*)tb_Buff.bp : "");
                        strcpy(wc_Cnts[ui_CntCurr].c_SkyTmp, tb_Buff.bp ? (char*)tb_Buff.bp : "");
                        tidyBufFree(&tb_Buff);
                      }
                    }
                    // если класс - temp - это температура воздуха
                    else if (!strcmp(c_AttrVal_l1, "temp"))
                    {
                      // искомое значение - во вложенном теге
                      tn_Param_l2 = tidyGetChild(tn_Param_l1);

                      // если вложенный объект - тег (имеет имя)
                      if (tidyNodeGetName(tn_Param_l2))
                      {
                        tn_Param_l3 = tidyGetChild(tn_Param_l2);

                        // если вложенный объект - не тег - то это искомое значение
                        if (!tidyNodeGetName(tn_Param_l3))
                        {
                          tidyNodeGetText(td_Tree, tn_Param_l3, &tb_Buff);
// -----            ----- температура воздуха
                          strcpy(c_BuffStr, tb_Buff.bp ? (char*)tb_Buff.bp : "\n");
                          GMP_Normalize(c_BuffStr);
                          c_ErrPtr = NULL;
                          wc_Cnts[ui_CntCurr].ss_Temp = strtol(c_BuffStr, &c_ErrPtr, 10);
                          if (*c_ErrPtr)
                          {
                            wc_Cnts[ui_CntCurr].ss_Temp = 32767;
                          }
                          tidyBufFree(&tb_Buff);
// -----            ----- конец температуры воздуха
                        }
                      }
                    }
                  }
                }
                // если параметров нет
                else
                {
                  tn_Param_l2 = tidyGetChild(tn_Param_l1);

                  // если есть вложенные теги
                  if (c_NodeName_l2 = tidyNodeGetName(tn_Param_l2))
                  {
                    // если первый вложенный тег - span
                    if (!strcmp(c_NodeName_l2, "span"))
                    {
                      ta_Attr = tidyAttrFirst(tn_Param_l2);
                      // если у него есть параметры
                      if(c_AttrName_l2 = tidyAttrName(ta_Attr))
                      {
                        // если первый параметр - класс
                        if (!strcmp(c_AttrName_l2, "class"))
                        {
                          // если класс - value m_press torr - это атмосферное давление
                          if (!strcmp(tidyAttrValue(ta_Attr), "value m_press torr"))
                          {
                            tn_Param_l3 = tidyGetChild(tn_Param_l2);

                            // если вложенный объект - не тег - то это искомое значение
                            if (!tidyNodeGetName(tn_Param_l3))
                            {
                              tidyNodeGetText(td_Tree, tn_Param_l3, &tb_Buff);
// -----                ----- атмосферное давление
                              strcpy(c_BuffStr, tb_Buff.bp ? (char*)tb_Buff.bp : "\n");
                              GMP_Normalize(c_BuffStr);
                              c_ErrPtr = NULL;
                              wc_Cnts[ui_CntCurr].us_Press = strtoul(c_BuffStr, &c_ErrPtr, 10);
                              if (*c_ErrPtr)
                              {
                                wc_Cnts[ui_CntCurr].us_Press = 32767;
                              }
                              tidyBufFree(&tb_Buff);
// -----                ----- конец атмосферного давления
                            }
                          }
                          // если класс - value m_temp c - это температура ощущаемая
                          else if (!strcmp(tidyAttrValue(ta_Attr), "value m_temp c"))
                          {
                            tn_Param_l3 = tidyGetChild(tn_Param_l2);

                            // если вложенный объект - не тег - то это искомое значение
                            if (!tidyNodeGetName(tn_Param_l3))
                            {
                              tidyNodeGetText(td_Tree, tn_Param_l3, &tb_Buff);
// -----                ----- температура ощущаемая
                              strcpy(c_BuffStr, tb_Buff.bp ? (char*)tb_Buff.bp : "\n");
                              GMP_Normalize(c_BuffStr);
                              c_ErrPtr = NULL;
                              wc_Cnts[ui_CntCurr].ss_TempPerc = strtol(c_BuffStr, &c_ErrPtr, 10);
                              if (*c_ErrPtr)
                              {
                                wc_Cnts[ui_CntCurr].ss_TempPerc = 32767;
                              }
                              tidyBufFree(&tb_Buff);
// -----                ----- конец температуры ощущаемой
                            }
                          }
                        }
                      }
                    }
                    // если первый вложенный тег - dl
                    else if (!strcmp(c_NodeName_l2, "dl"))
                    {
                      ta_Attr = tidyAttrFirst(tn_Param_l2);
                      // если у него есть параметры
                      if(c_AttrName_l2 = tidyAttrName(ta_Attr))
                      {
                        // если первый параметр - класс
                        if (!strcmp(c_AttrName_l2, "class"))
                        {
                          // если класс - wind - это данные о ветре
                          if (!strcmp(tidyAttrValue(ta_Attr), "wind"))
                          {
                            // в первом вложенном теге - направление
                            tn_Param_l2 = tidyGetChild(tn_Param_l2);
                            
                            // если вложенный объект - тег (имеет имя)
                            if (tidyNodeGetName(tn_Param_l2))
                            {
                              tn_Param_l3 = tidyGetChild(tn_Param_l2);

                              // если вложенный объект - не тег - то это искомое значение
                              if (!tidyNodeGetName(tn_Param_l3))
                              {
                                tidyNodeGetText(td_Tree, tn_Param_l3, &tb_Buff);
// -----                  ----- направление ветра
                                //printf("%*.0s%s\n", 4, " ", tb_Buff.bp ? (char*)tb_Buff.bp : "");
                                strcpy(wc_Cnts[ui_CntCurr].c_WindTmp, tb_Buff.bp ? (char*)tb_Buff.bp : "");
                                tidyBufFree(&tb_Buff);
// -----                  ----- конец направления ветра
                              }
                            }

                            // в первом вложенном теге второго вложенного тега - скорость в м/с
                            tn_Param_l2 = tidyGetNext(tn_Param_l2);

                            // если вложенный объект - тег (имеет имя)
                            if (tidyNodeGetName(tn_Param_l2))
                            {
                              tn_Param_l3 = tidyGetChild(tn_Param_l2);

                              // если вложенный объект - тег (имеет имя)
                              if (tidyNodeGetName(tn_Param_l3))
                              {
                                tn_Param_l3 = tidyGetChild(tn_Param_l3);

                                // если вложенный объект - не тег - то это искомое значение
                                if (!tidyNodeGetName(tn_Param_l3))
                                {
                                  tidyNodeGetText(td_Tree, tn_Param_l3, &tb_Buff);
// -----                    ----- скорость ветра
                                  strcpy(c_BuffStr, tb_Buff.bp ? (char*)tb_Buff.bp : "\n");
                                  GMP_Normalize(c_BuffStr);
                                  c_ErrPtr = NULL;
                                  wc_Cnts[ui_CntCurr].us_WindS = strtoul(c_BuffStr, &c_ErrPtr, 10);
                                  if (*c_ErrPtr)
                                  {
                                    wc_Cnts[ui_CntCurr].us_WindS = 32767;
                                  }
                                  tidyBufFree(&tb_Buff);
// -----                    ----- конец скорости ветра
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                  // если вложенных тегов нет - это влажность воздуха
                  else
                  {
                    tidyNodeGetText(td_Tree, tn_Param_l2, &tb_Buff);
// -----      ----- влажность воздуха
                    strcpy(c_BuffStr, tb_Buff.bp ? (char*)tb_Buff.bp : "\n");
                    GMP_Normalize(c_BuffStr);
                    c_ErrPtr = NULL;
                    wc_Cnts[ui_CntCurr].us_Hum = strtoul(c_BuffStr, &c_ErrPtr, 10);
                    if (*c_ErrPtr)
                    {
                      wc_Cnts[ui_CntCurr].us_Hum = 32767;
                    }
                    tidyBufFree(&tb_Buff);
// -----      ----- конец влажности воздуха
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return ui_CntCurr;
}

// ----- конец рекурсивного обхода и анализа узла с отсчётами

// ----- рекурсивный обход дерева в поисках узлов с отсчётами

void GMP_ProcTree(TidyDoc td_Tree, TidyNode tn_Node, unsigned int ui_CntNum, GMP_WCount* wc_Cnts)
{
  TidyNode tn_Child;
  bool bl_IsDay = false;
  unsigned int ui_CntCurr = 0;

  for (tn_Child = tidyGetChild(tn_Node); tn_Child; tn_Child = tidyGetNext(tn_Child))
  {
    bl_IsDay = false;

    if (tidyNodeGetName(tn_Child))
    {
      if (!strcmp(tidyNodeGetName(tn_Child), "tbody") and
          !strcmp(tidyAttrName(tidyAttrFirst(tn_Child)), "id") and
          !strncmp(tidyAttrValue(tidyAttrFirst(tn_Child)), "tbwdaily", 8))
      {
        bl_IsDay = true;
      }
    }

    if (bl_IsDay)
    {
      if (ui_CntCurr < ui_CntNum)
      {
        ui_CntCurr = GMP_ui_ProcDay(td_Tree, tn_Child, ui_CntNum, wc_Cnts, ui_CntCurr);
      }
    }
    else
    {
      GMP_ProcTree(td_Tree, tn_Child, ui_CntNum, wc_Cnts);
    }
  }
}

// ----- конец рекурсивного обхода дерева в поисках узлов с отсчётами

// ----- ----- -----

int GMP_i_ReadGMURL(char* c_URL, unsigned int ui_CntNum, GMP_WCount* wc_Cnts)
{
  int i_Error = 0;

// ----- инициализация HTMLTidy

  TidyDoc td_Tree;
  TidyBuffer tb_TreeBuff = {0};
  TidyBuffer tb_TreeErrBuff = {0};

  td_Tree = tidyCreate();
  tidySetInCharEncoding(td_Tree, "utf8");
  tidySetOutCharEncoding(td_Tree, "utf8");
  tidyOptSetBool(td_Tree, TidyForceOutput, yes);
  tidyOptSetInt(td_Tree, TidyWrapLen, 0);
  tidySetErrorBuffer(td_Tree, &tb_TreeErrBuff);
  tidyBufInit(&tb_TreeBuff);

// ----- конец инициализации HTMLTidy

// ----- инициализация CURL

  CURL* cu_Connection;
  char c_ErrBuff[CURL_ERROR_SIZE];
  struct curl_slist* cusl_Header = NULL;

  cusl_Header = curl_slist_append(cusl_Header, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:31.0) Gecko/20100101 Firefox/31.0 Iceweasel/31.6.0");
  cusl_Header = curl_slist_append(cusl_Header, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
  cusl_Header = curl_slist_append(cusl_Header, "Accept-Language: ru-ru,ru;q=0.8,en-us;q=0.5,en;q=0.3");
  cusl_Header = curl_slist_append(cusl_Header, "Accept-Encoding: none");
  cusl_Header = curl_slist_append(cusl_Header, "Connection: keep-alive");
  cusl_Header = curl_slist_append(cusl_Header, "DNT: 1");

  cu_Connection = curl_easy_init();
  curl_easy_setopt(cu_Connection, CURLOPT_URL, c_URL);
  curl_easy_setopt(cu_Connection, CURLOPT_HTTPHEADER, cusl_Header);
  curl_easy_setopt(cu_Connection, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(cu_Connection, CURLOPT_ERRORBUFFER, c_ErrBuff);
  curl_easy_setopt(cu_Connection, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(cu_Connection, CURLOPT_VERBOSE, 0);
  curl_easy_setopt(cu_Connection, CURLOPT_WRITEFUNCTION, GMP_ui_WriteFunc);

  curl_easy_setopt(cu_Connection, CURLOPT_WRITEDATA, &tb_TreeBuff);

// ----- конец инициализации CURL

// ----- чтение и парсинг URL

  i_Error = curl_easy_perform(cu_Connection); // устанавливаем подключение и читаем URL в буфер
  if (!i_Error)
  {
    i_Error = tidyParseBuffer(td_Tree, &tb_TreeBuff); // парсим буфер
    if (i_Error >= 0)
    {
      i_Error = tidyCleanAndRepair(td_Tree);  // чиним содержимое буфера
      if (i_Error >= 0)
      {
        i_Error = tidyRunDiagnostics(td_Tree);  // проводим диагностику (заполняем буфер ошибок)
        if (i_Error >= 0)
        {
          // обходим получившееся дерево и заполняем wc_Cnts
          GMP_ProcTree(td_Tree, tidyGetRoot(td_Tree), ui_CntNum, wc_Cnts);
        }
      }
    }
  }

// ----- конец чтения и парсинга URL

// ----- очистка объектов

  tidyBufFree(&tb_TreeErrBuff);
  tidyBufFree(&tb_TreeBuff);
  tidyRelease(td_Tree);
  curl_slist_free_all(cusl_Header);
  curl_easy_cleanup(cu_Connection);

// ----- конец очистки объектов

  return i_Error;
}

// ----- ----- -----
