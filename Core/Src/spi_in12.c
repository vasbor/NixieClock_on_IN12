/*

*/

#include "spi_in12.h"

const uint8_t scanLineArr[] = {0xE0, 0xD0, 0xB0, 0x70};     //коды разрядов дисплея для режима ON_ALL
const uint8_t scanLine_mask21[] = {0xE0, 0xD0, 0xFF, 0xFF}; //коды разрядов дисплея для режима ON_43
const uint8_t scanLine_mask43[] = {0xFF, 0xFF, 0xB0, 0x70}; //коды разрядов дисплея для режима ON_21
const uint8_t digArr[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09}; //коды десятичных цифр(0...9)

volatile uint8_t indCount = 0;    //счетчик скан-линий для динамической индикации
uint8_t frameBuff[LEN_DISPLAY];          //храним значения разрядов дисплея
t_modeInd modeInd;

void init_IN12(void){
    //*** Init GPIO ************************************************
    //PA4, PA6 as output push-pull high speed (сигналы OE, LATCH)
    GPIOA->MODER |= (GPIO_MODER_MODER4_0 | GPIO_MODER_MODER6_0);
    GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR4 | GPIO_OSPEEDER_OSPEEDR6);

    //****GPIO for SPI1**********************************************
    //PA5 -> SCK, PA7 -> MOSI
    GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1);  //AF
    GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR5 | GPIO_OSPEEDER_OSPEEDR7);  //high speed
    //***************************************************************

    //*** Init SPI1 ************************************************
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 |= (SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);
    SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR);
    //SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= SPI_CR1_BR_1;
    SPI1->CR1 &= ~SPI_CR1_CPHA;
    SPI1->CR1 &= ~SPI_CR1_CPOL;
    SPI1->CR2 |= (SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2);
 
    SPI1->CR1 |= SPI_CR1_SPE;
    //**************************************************************

    //*** init TIM16 ****************************************
    RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;

    //TIM16->PSC = (uint16_t)8000U-1; //1kHz
    //TIM17->PSC = (uint16_t)8000U-1; //1kHz
    TIM16->PSC = (uint16_t)48000U-1; //1kHz

    TIM16->ARR = (uint16_t)4;       //4ms 

    TIM16->DIER |= TIM_DIER_UIE;

    TIM16->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM16_IRQn);
    //**************************************************************

    modeInd = ON_ALL;
    SHOW_ON;
    
}

void spiWriteByte(uint8_t byte){
	while (!(SPI1->SR & SPI_SR_TXE));	//ожидание освобожления регистра
	*((uint8_t *)&SPI1->DR) = byte;     //запись байта регистр данных SPI    
}

void showFrame (void){
        SHOW_OFF;
        LATCH_LOW;
        
        uint8_t byte;

        switch (modeInd)
        {
        case ON_21:
            byte = digArr[frameBuff[indCount]] | scanLine_mask21[indCount];
            break;

        case ON_43:
            byte = digArr[frameBuff[indCount]] | scanLine_mask43[indCount];
            break;
        default:
            byte = digArr[frameBuff[indCount]] | scanLineArr[indCount];
            break;
        }

        spiWriteByte(byte);

        while (SPI1->SR & SPI_SR_BSY);  //ожидание окончания передачи
        LATCH_HIGT;
        indCount++;
        if (indCount == LEN_DISPLAY) indCount = 0;
        SHOW_ON;
}

void setFrame(uint8_t r43, uint8_t r21, t_modeInd mode){
    modeInd = mode;
        
    frameBuff[0] = r21 % 10;
    frameBuff[1] = r21 / 10;
    frameBuff[2] = r43 % 10;
    frameBuff[3] = r43 / 10;
}


void TIM16_IRQHandler(void){
    showFrame();
    scanButtons();
    TIM16->SR &= ~TIM_SR_UIF;
}