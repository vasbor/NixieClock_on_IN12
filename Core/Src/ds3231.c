/*

*/

#include "ds3231.h"

t_ds3231 time;  //хранение минут и часов

void init_DS3231(void){
    //****GPIO for I2C1***********************************************
    //PA9->SCL, PA10->SDA (pull-up не включаем потому, что есть внешние резисторы подтяжки)
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);                           //AF
    GPIOA->OTYPER |= (GPIO_OTYPER_OT_9 | GPIO_OTYPER_OT_10);                                //Open Drain
    GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEEDR9 | GPIO_OSPEEDR_OSPEEDR10);                     //high speed
    GPIOA->AFR[1] |= ((0x04 << GPIO_AFRH_AFSEL9_Pos) | (0x04 << GPIO_AFRH_AFSEL10_Pos));    //AF4
    //****************************************************************

    //*** Init I2C1***************************************************
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = (uint32_t)0x2000090E;   //расчитано в CubeMX
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;
    I2C1->CR2 = (ADDRESS_DS3231 << 1);

    I2C1->CR1 |= I2C_CR1_PE;
    //****************************************************************
}

uint8_t readReg_DS3231(addr_register addr){

    I2C1->CR2 &= ~I2C_CR2_AUTOEND;
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;       //режим записи
    I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C1->CR2 |= (uint32_t)(0x01)<<16;  //записываем один байт (адрес регистра DS3231)
    while(!(I2C1->ISR & I2C_ISR_TXE));  //ожидание освобождения регистра передачи
    
    I2C1->CR2 |= I2C_CR2_START;         //старт передачи
    while(!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = (uint32_t)addr;        //адрес регистра минут DS3231
    //проверка регистра ISR_NACKF
    while(!(I2C1->ISR & I2C_ISR_TC));   //ожидание завершения передачи

    I2C1->CR2 |=I2C_CR2_AUTOEND;        //автозавершение включено
    I2C1->CR2 |= I2C_CR2_RD_WRN;        //режим чтения

    I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C1->CR2 |= (uint32_t)(0x01)<<16;  //будем считывать 1 байт из регистра, заданного <addr>
    
    I2C1->CR2 |= I2C_CR2_START;         //старт передачи
    
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    return I2C1->RXDR;
}

t_ds3231* getMinutesHours(void){
    uint8_t reg_minutes, reg_hours; //для хранения считанных данных из ркгистров DS3231

    //ожидание освобождения шины
    //I2C1->CR1 |= I2C_CR1_PE;
    I2C1->CR2 &= ~I2C_CR2_AUTOEND;
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;   //режим записи
    I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C1->CR2 |= (uint32_t)(0x01)<<16;  //записываем один байт (адрес регистра DS3231)
    while(!(I2C1->ISR & I2C_ISR_TXE));  //ожидание освобождения регистра передачи
    
    I2C1->CR2 |= I2C_CR2_START;     //старт передачи
    while(!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = (uint32_t)0x01;              //адрес регистра минут DS3231
    //проверка регистра ISR_NACKF
    while(!(I2C1->ISR & I2C_ISR_TC));   //ожидание завершения передачи

    I2C1->CR2 |=I2C_CR2_AUTOEND;    //автозавершение включено
    I2C1->CR2 |= I2C_CR2_RD_WRN;    //режим чтения
    //I2C1->CR2 |= 0<<16;
    I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C1->CR2 |= (uint32_t)(0x02)<<16;  //будем считывать 2 байта (регистр минут и регистр часов)
    
    I2C1->CR2 |= I2C_CR2_START;     //старт передачи
    
    while(!(I2C1->ISR & I2C_ISR_RXNE));
    reg_minutes = I2C1->RXDR;

    while(!(I2C1->ISR & I2C_ISR_RXNE));
    reg_hours = I2C1->RXDR;

    time.minutes = 10 * (reg_minutes >> 4) + (reg_minutes & 0x0f);
    time.hours = 10 * (reg_hours >> 4) + (reg_hours & 0x0f);

    return &time;
}

void writeReg_DS3231(addr_register addr, uint8_t value){

    uint8_t writeToReg = (uint8_t)((value % 10) & 0x0f) | ((value / 10) << 4) ;

    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;           //режим записи
    I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C1->CR2 |= (uint32_t)(0x02)<<16;  
    while(!(I2C1->ISR & I2C_ISR_TXE));      //ожидание освобождения регистра передачи
    
    I2C1->CR2 |= I2C_CR2_START;             //старт передачи
    while(!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = (uint32_t)addr;            //адрес регистра минут DS3231
    while(!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = (uint32_t)writeToReg;
}

uint8_t getDate(void){
    uint8_t date_reg = readReg_DS3231(DATE);
    return 10 * (date_reg >> 4) + (date_reg & 0x0f);
}

uint8_t getMonth(void){
    uint8_t month_reg = readReg_DS3231(MONTH_CENTURY);
    return 10 * (month_reg >> 4) + (month_reg & 0x0f);
}

uint8_t getYear(void){
    uint8_t year_reg = readReg_DS3231(YEAR);
    return 10 * (year_reg >> 4) + (year_reg & 0x0f);
}
