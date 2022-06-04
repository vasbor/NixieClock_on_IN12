/*
* в часах используется микросхема RTC DS3231,
* подключенная по интерфейсу I2C1, используются выводы MCU:
*   PA9  -> SKL;
*   PA10 -> SDA;
* Для хранения данных в микросхеме DS3231 используется 19 однобайтовых регистров.
* В текущей реализации используются только регистры минут, часов, даты, месяца и года.
*/

#ifndef __DS3231_H
#define __DS3231_H

#include "main.h"

#define ADDRESS_DS3231  0x68    //адрес на шине I2C (from datasheet DS3231)
#define NUM_REG_DS3231  19      //количество Timekeeping Registers (from datasheet DS3231)

//адреса регистров DS3231
typedef enum {
  SECONDS,
  MINUTES,
  HOURS, 
  DAY,
  DATE,
  MONTH_CENTURY,
  YEAR,
  ALARM_1_SECONDS,
  ALARM_1_MINUTES,
  ALARM_1_HOURS,
  ALARM_1_DAY,
  ALARM_1_DATE,
  ALARM_2_MINUTES,
  ALARM_2_HOURS,
  ALARM_2_DAY,
  ALARM_2_DATE,
  CONTROL,
  STATUS,
  ALIGN_OFFSET,
  MSB_OF_TEMP,
  LSB_OF_TEMP,
} addr_register;

//структура для хранения минут и часов
//TODO: для единообразия добавить в структуру дату, месяц и год (и чтение с DMA?)
typedef struct ds3231
{
  uint8_t minutes;
  uint8_t hours;
} t_ds3231;

void init_DS3231(void);                                   //инициализация GPIO и интерфейса I2C1
uint8_t readReg_DS3231(addr_register addr);               //чтение одного регистра DS3231
void writeReg_DS3231(addr_register addr, uint8_t value);  //запись в регистр DS3231

t_ds3231 *getMinutesHours(void);                          //чтение регистров минут и часов
uint8_t getDate(void);
uint8_t getMonth(void);
uint8_t getYear(void);
#endif
