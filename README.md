### 📋 Описание 
Библиотека для NRF24L01+ под микроконтроллеры STM32 для CMSIS. В этой инструкцие не будут описаны регистры NRF24L01+ и описание его команд, так как для этого есть официальная документация. Также не будет затронута настройка периферий SPI и DMA, функции задержки и наистройка пинов, они могут отличаться у различных моделей микроконтроллеров. В данном репозитории же представлен пример для STM32F0430F4P6, рекомендуется ориентироваться на него при настройке периферии. 

В заголовочном файле nrf24.h представлены маски регистров и битов nrf24, объявления необходимых функций, команды и некоторые вспомогательные вещи. Также там есть структуры регистров, которые значительно упрощают отладку датчика. 

### Функции
Функции с использованием DMA и без него выполняют одни и те же задачи 

## NRF24_Read_Reg(uint8_t rg), NRF24_Read_Reg_DMA(uint8_t rg)
Читает значение из регистра.

## NRF24_Write_Reg(uint8_t rg, uint8_t dt), NRF24_Write_Reg_DMA(uint8_t rg, uint8_t dt)
Записывает значение в регистр с перезаписью ВСЕГО регистра.

## NRF24_Write_Bit(uint8_t rg, uint8_t bit, BitAction value), NRF24_Write_Bit_DMA(uint8_t rg, uint8_t bit, BitAction value)
Записывает ОДИН бит в регситр без перезаписи всего регистра.

## NRF24_Write_Reg_Multiple(uint8_t reg, uint8_t *data, uint8_t data_size), NRF24_Write_Reg_Multiple_DMA(uint8_t reg, uint8_t *data, uint8_t data_size)
Аналогичен NRF24_Write_Reg, но записывает данные в регистр размером больше 1 байта (адрес).

## NRF24_Write_Payload(uint8_t *data, uint8_t data_size), NRF24_Write_Payload_DMA(uint8_t *data, uint8_t data_size)
Записывает данные в регистр TX payload.

## NRF24_Clear_Status(void), NRF24_Clear_Status_DMA(void)
Очищает биты статуса в регисре STATUS.

## NRF24_FLUSH_RX(void), NRF24_FLUSH_TX(void); NRF24_FLUSH_RX_DMA(void), NRF24_FLUSH_TX_DMA(void)
Очищают RX и TX буфферы соответственно.

## NRF24_Set_rx_addr(uint8_t *addr, uint8_t num_pipe), NRF24_Set_rx_addr_DMA(uint8_t *addr, uint8_t num_pipe)
Записывает адреса приёмников (подробней про адреса см. в разделе Основная настройка NRF24LO1+).

## NRF24_Set_tx_addr(uint8_t *addr), NRF24_Set_tx_addr_DMA(uint8_t *addr), 
Записывает адреса передатчика (подробней про адреса см. в разделе Основная настройка NRF24LO1+).

## NRF24_Read_RX(uint8_t *data, uint8_t data_size), NRF24_Read_RX_DMA(uint8_t *data, uint8_t data_size)
Считывает полученный данные из RX payload

## NRF24_RX_PAYLOAD_LENGHT(void), NRF24_RX_PAYLOAD_LENGHT_DMA(void)
Считывает длину полученных данный из RX payload (используется только с DPL)

## NRF24_Read_RX_DPL(uint8_t *data), NRF24_Read_RX_DMA_DPL(uint8_t *data)
Считывает полученный данные из RX payload c DPL

#Пример:
void SPI1_Init(void){
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //SPI clocking
	
	SPI1->CR1 = 0; //SPI clocking
	SPI1->CR1 |= SPI_CR1_BR_1; // 48000000 / 8 = 6 MHz
	SPI1->CR1 &= ~(SPI_CR1_BIDIMODE | SPI_CR1_RXONLY | SPI_CR1_BIDIOE); //2-line unidirectional data mode, full duplex
	SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA); //[0,0] mode (CPOL - 0, CPHA - 0)
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST; //MSB first
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI; //Software slave management enable
	SPI1->CR1 |= SPI_CR1_MSTR; //Master configuration
	
	SPI1->CR2 = 0; //reset all CR2 registers
	SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2; // 8 bit 
	SPI1->CR2 |= SPI_CR2_FRXTH; 
	
	SPI1->CR1 |= SPI_CR1_SPE; //SPI enable
}
//Функция передачи
uint8_t SPI_transfer_data(uint8_t dt) {
	while (!(SPI1->SR & SPI_SR_TXE));
	*(__IO uint8_t*)&SPI1->DR = dt;
	while (!(SPI1->SR & SPI_SR_RXNE));
	return (*(__IO uint8_t*)&SPI1->DR);
}

### Настройка DMA
