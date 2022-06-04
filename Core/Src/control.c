/*

*/

#include "control.h"
#include "buttons.h"

t_readyToRead fReady;
t_stateShow stateShow;
t_buttonState butSet, butSelect;

const uint8_t MAX_MINUTES = 59;
const uint8_t MAX_HOURS = 23;
const uint8_t MAX_DATE = 31;
const uint8_t MAX_MONTH = 12;
const uint8_t MAX_YEAR = 99;


void init_Control(void){
    //PA0, PA1 as output push-pull low speed (светодиоды "двоеточия")
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 
    GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //*** Init TIM14 *************************************************
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;

    TIM14->PSC = (uint16_t)48000U-1;
    
    TIM14->ARR = (uint16_t)100;

    TIM14->DIER |= TIM_DIER_UIE;

    TIM14->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM14_IRQn);
    //****************************************************************

    //*** init TIM17 *************************************************
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;

    TIM17->PSC = (uint16_t)48000U-1; //1kHz

    TIM17->ARR = (uint16_t)1000;

    TIM17->DIER |= TIM_DIER_UIE;

    TIM17->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM17_IRQn);
    //**************************************************************

    stateShow = TIME_SHOW;
}

void clockControl(void){
    if (getStateButSelect())
    {
        stateShow++;
        if (stateShow > YEAR_SHOW) stateShow = TIME_SHOW;
        setStateButSelect(NO_PRESSED);
    }    
    
    if (fReady && stateShow == TIME_SHOW)       //индикация часов и минут
    {
        t_ds3231 *currentTime = getMinutesHours();
        setFrame(currentTime->hours, currentTime->minutes, ON_ALL);
        fReady = NO_READY;      
    }

    if (stateShow == DATE_SHOW)                 //индикация даты (число и месяц)
    {
        uint8_t date = getDate();
        uint8_t month = getMonth();
        setFrame(date, month, ON_ALL);
    }

    if (stateShow == YEAR_SHOW)                 //индикация года
    {
        uint8_t year = getYear();
        setFrame((uint8_t)20, year, ON_ALL);
    }

    if (getStateButSet())
    {
        stateShow = MINUTES_SET;
        //сохраняем текущие значения времени и даты в буферы
        t_ds3231 *buffTime = getMinutesHours();
        uint8_t buffMinutes = buffTime->minutes;
        uint8_t buffHours = buffTime->hours;
        uint8_t buffDate = getDate();
        uint8_t buffMonth = getMonth();
        uint8_t buffYear = getYear();

        setStateButSet(NO_PRESSED);
        setStateButSelect(NO_PRESSED);
        while (1)
        {
            if (getStateButSelect())
            {
                stateShow++;
                setStateButSelect(NO_PRESSED);
            }

            if (stateShow == MINUTES_SET)
            {
                if (getStateButSet())
                {
                    buffMinutes++;
                    if (buffMinutes > MAX_MINUTES) buffMinutes = 0;
                    setStateButSet(NO_PRESSED);
                }

                setFrame(buffHours, buffMinutes, ON_21);
                
                if (buffMinutes != buffTime->minutes)
                {
                    writeReg_DS3231(MINUTES, buffMinutes);
                    writeReg_DS3231(SECONDS, (uint8_t)0);
                }
            }

            if (stateShow == HOURS_SET)
            {   
                if (getStateButSet())
                {
                    buffHours++;
                    if (buffHours > MAX_HOURS) buffHours = 0;
                    setStateButSet(NO_PRESSED);
                }

                setFrame(buffHours, buffMinutes, ON_43);

                if (buffHours != buffTime->hours)
                {
                    writeReg_DS3231(HOURS, buffHours);
                }
            }

            if (stateShow == DATE_SET)
            {
                if (getStateButSet())
                {
                    buffDate++;
                    if (buffDate > MAX_DATE) buffDate = 1;
                    setStateButSet(NO_PRESSED);
                }
                
                setFrame(buffDate, buffMonth, ON_43);

                if (buffDate != getDate())
                {
                    writeReg_DS3231(DATE, buffDate);
                }
            }

            if (stateShow == MONTH_SET)
            {
                if (getStateButSet())
                {
                    buffMonth++;
                    if (buffMonth > MAX_MONTH) buffMonth = 1;
                    setStateButSet(NO_PRESSED);
                }
                
                setFrame(buffDate, buffMonth, ON_21);

                if (buffMonth != getMonth())
                {
                    writeReg_DS3231(MONTH_CENTURY, buffMonth);
                }
            }

            if (stateShow == YEAR_SET)
            {   
                if (getStateButSet())
                {
                    buffYear++;
                    if (buffYear > MAX_YEAR) buffYear = 0;
                    setStateButSet(NO_PRESSED);
                }
                
                setFrame((uint8_t)20, buffYear, ON_21);

                if (buffYear != getYear())
                {
                    writeReg_DS3231(YEAR, buffYear);
                }
            }

            if (stateShow > YEAR_SET)
            {
                setStateButSelect(NO_PRESSED);
                setStateButSet(NO_PRESSED);
                break;
            }
        }
        
    }
    
}
void TIM14_IRQHandler(void){
    fReady = READY;
    incCount();
    TIM14->SR &= ~TIM_SR_UIF;
}

void TIM17_IRQHandler(void){        //управление светодиодами "двоеточия"
    if (stateShow == TIME_SHOW){    //светодиоды меняют состояние
        GPIOA->ODR ^= GPIO_ODR_0;
        GPIOA->ODR ^= GPIO_ODR_1;
    }
    
    if (stateShow == DATE_SHOW){    //светодиоды включены
        GPIOA->ODR |= GPIO_ODR_0;
        GPIOA->ODR |= GPIO_ODR_1;
    }

    if (stateShow == YEAR_SHOW){    //светодиоды выключены
        GPIOA->ODR &= ~GPIO_ODR_0;
        GPIOA->ODR &= ~GPIO_ODR_1;
    }
    
    TIM17->SR &= ~TIM_SR_UIF;
}