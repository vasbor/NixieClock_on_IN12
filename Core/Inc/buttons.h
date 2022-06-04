/*
* Подключение кнопок к MCU:
* PA2 -> But_Select Кнопка выбора переключает режимы индикации 
* PA3 -> But_Set Кнопка перехода в режим настройки и
* изменения значений времени (каждое нажатие увеличивает значение на 1)
*
* Нажатия кнопок отслеживаются в функции scanButtons() циклическим опросом
*
*/

#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "main.h"

typedef enum key
{
    NO_PRESENT,
    BUTTON_SELECT,
    BUTTON_SET,
} t_key;

typedef enum buttonState
{                       //состояния кнопок
    NO_PRESSED,         //не нажата
    PRESSED             //нажата
} t_buttonState;

void init_BUTTONS(void);
void incCount(void);
void scanButtons(void);
t_buttonState getStateButSet(void);
t_buttonState getStateButSelect(void);
void setStateButtons(t_key key);
void setStateButSet(t_buttonState state);
void setStateButSelect(t_buttonState state);

#endif