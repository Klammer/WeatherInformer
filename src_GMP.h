#ifndef __GISMETEOPARSER_FUNCS_H
#define __GISMETEOPARSER_FUNCS_H

// ----- ----- -----

/*
  Структура отсчёта погоды.
  Параметры полей:
  - 16383 - значения по-умолчанию (для s_Sky_len - 0)
  - 32767 - при получении возникла ошибка
  - s_Sky_len = -1 - при получении c_Sky возникла ошибка
*/
struct GMP_WCount
{
  unsigned short us_Year;    // 0 - 3000  + год
  unsigned short us_Month;   // 1 - 12    + месяц
  unsigned short us_Day;     // 1 - 31    + день
  unsigned short us_Time;    // 0 - 24    + время
  short s_Sky_len;
  char* c_Sky;               // + облачность     td class="cltext"
  signed short ss_Temp;      // -50 - 50  + температура воздуха    span class='value m_temp c'
  unsigned short us_Press;   // 0 - 1000  + атмосферное давление, мм ртутного столба   span class='value m_press torr'
  unsigned short us_WindT;   // 0 - 10      направление ветра    dt class="wicon wind6"
  unsigned short us_WindS;   // 0 - 100     скорость ветра     span class='value m_wind ms'
  unsigned short us_Hum;     // 0 - 100   + влажность воздуха, %
  signed short ss_TempPerc;  // -50 - 50  + температура ощущаемая   span class='value m_temp c'
  
  char c_SkyTmp[256];
  char c_WindTmp[64];
};

// ----- ----- -----

/*
  Инициализация отсчёта. Заполняет поля значениями по-умолчанию.
*/
void GMP_InitWCount(GMP_WCount &wc_Object);

// ----- ----- -----

/*
  Парсит страницу gismeteo.ru и заполняет массив отсчётов.
  Принимает:
    c_URL - строка, содержащая URL анализируемой страницы
    ui_CntNum - максимальное количество отсчётов, которое можно обработать (размер wc_Cnts)
    wc_Cnts - массив отсчётов
*/
int GMP_i_ReadGMURL(char* c_URL, unsigned int ui_CntNum, GMP_WCount* wc_Cnts);

// ----- ----- -----

#endif
