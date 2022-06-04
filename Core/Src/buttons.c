/*

*/

#include "buttons.h"

t_buttonState buttonSet, buttonSelect;
uint16_t countDeBouce;          //счетчик для антидребезга
t_key lastKey;                  //код нажатой кнопки
const uint16_t MAX_COUNT = 2;   //временная задержка для антидребезка кнопок

void init_BUTTONS(void)
{
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_2, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_3, LL_GPIO_PULL_UP);

  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_2, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_3, LL_GPIO_MODE_INPUT);
  
    buttonSelect = NO_PRESSED;
    buttonSet = NO_PRESSED;
}

void incCount(void){
    countDeBouce++;
}


void scanButtons(void){
    //если нажатия кнопок еще не обработаны выходим из сканирования
    if (getStateButSelect() || getStateButSet()) return;
    
    t_key currentKey;

    if (!(GPIOA->IDR & GPIO_IDR_2)) currentKey = BUTTON_SELECT;
    else if (!(GPIOA->IDR & GPIO_IDR_3)) currentKey = BUTTON_SET;
    else currentKey = NO_PRESENT;

    if (currentKey)
    {
        if (currentKey == lastKey)
        {
            if (countDeBouce >= MAX_COUNT)
            {
                setStateButtons(currentKey);
                lastKey = NO_PRESENT;
                countDeBouce = 0;
            }
            
        }
        else{
            lastKey = currentKey;
            countDeBouce = 0;
        }
        
    }
    else
    {
        countDeBouce = 0;
    }
}

t_buttonState getStateButSet(void){
    return buttonSet;
}

t_buttonState getStateButSelect(void){
    return buttonSelect;
}

void setStateButSet(t_buttonState state){
    buttonSet = state;
}

void setStateButSelect(t_buttonState state){
    buttonSelect = state;
}

void setStateButtons(t_key key){
    if (key == BUTTON_SELECT)
    {
        buttonSelect = PRESSED;
    }

    if (key == BUTTON_SET)
    {
        buttonSet = PRESSED;
    }
    
}