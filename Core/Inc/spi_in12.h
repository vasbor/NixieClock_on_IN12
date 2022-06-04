/*
* Для управления четырехразрядным цифровым дисплеем на лампах ИН12
* используется интерфейс SPI, регистр сдвига 74HCT595 и счетчик К155ИД1.
* Четыре младших бита регистра сдвига формируют выводимое на дисплей число,
* четыре старших определяют активный разряд дисплея.
* Индикация динамическая, реализованная в прерывании TIM16.
* Также в обработчике прерывания TIM16 происходит опрос кнопок управления.
* Предусмотрено три режима работы дисплея:
*   ON_ALL -> все разряды активны,
*   ON_21 -> активны два правых разряда (левые погашены).
*   ON_43 -> активны два левых разряда (правые погашены).
*
* Настраивается периферия SPI1 в полудуплексном режиме
* Используемые выводы МК:
*   PA5 -> SCLK
*   PB7 -> SDATA
* Дополнительно:
*   PA6 -> LOAD (защелка сдвигового регистра, активный уровень - ВЫСОКИЙ)
*   PA4 -> OE (включение выходов сдвигового регистра, активный уровень - НИЗКИЙ)
*/

#ifndef __SPI_IN12_H
#define __SPI_IN12_H

#include "main.h"

//сигналы управления регистром сдвига
#define SHOW_OFF    GPIOA->BSRR |= GPIO_BSRR_BS_4   //выходы регистра сдвига в третьем состоянии
#define SHOW_ON     GPIOA->BSRR |= GPIO_BSRR_BR_4   //выходы регистра сдвига включены
#define LATCH_LOW   GPIOA->BSRR |= GPIO_BSRR_BR_6   //разрешение записи в регистр сдвига
#define LATCH_HIGT  GPIOA->BSRR |= GPIO_BSRR_BS_6   //запрет записи в регистр сдвига 

#define LEN_DISPLAY 4   //число разрядов дисплея

typedef enum{
    FLAG_RESET,
    FLAG_SET,
} statusIndicator;

typedef enum modeInd{   //режимы дисплея
    ON_ALL,
    ON_21,
    ON_43,
} t_modeInd;

void init_IN12(void);                                       //инициализация GPIO, SPI1, таймеров
void spiWriteByte(uint8_t byte);
void showFrame (void);                                      //индикация активных данных (даты или времени)
void setFrame(uint8_t r43, uint8_t r21, t_modeInd mode);    //заполнение буфера дисплея

void TIM16_IRQHandler(void);                                //динамическая индикация

#endif
