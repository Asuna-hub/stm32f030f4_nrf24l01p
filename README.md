# Описание 
Библиотека для NRF24L01+ под микроконтроллеры STM32 для CMSIS. В этой документации не будут описаны регистры NRF24L01+ и описание его команд, так как для этого есть официальная документация. Также не будет затронута настройка периферий SPI и DMA, функции задержки и наистройка пинов, они могут отличаться у различных моделей микроконтроллеров. В данном репозитории же представлен пример для STM32F030F4P6, рекомендуется ориентироваться на него при настройке периферии. 

В заголовочном файле nrf24.h представлены маски регистров и битов nrf24, объявления необходимых функций, команды и некоторые вспомогательные вещи. Также там есть структуры регистров, которые значительно упрощают отладку датчика. 

# Функции
Функции с использованием DMA и без него выполняют одни и те же задачи 

- NRF24_Read_Reg(_DMA)\
Читает значение из регистра.

- NRF24_Write_Reg(_DMA)\
Записывает значение в регистр с перезаписью ВСЕГО регистра.

- NRF24_Write_Bit(_DMA)\
Записывает ОДИН бит в регистр без перезаписи всего регистра.

- NRF24_Write_Reg_Multiple(_DMA)\
Аналогичен NRF24_Write_Reg, но записывает данные в регистр размером больше 1 байта (адрес).

- NRF24_Write_Payload(_DMA)\
Записывает данные в регистр TX payload.

- NRF24_Clear_Status(_DMA)\
Очищает биты статуса в регисре STATUS.

- NRF24_FLUSH_RX(_DMA), NRF24_FLUSH_TX(_DMA)\
Очищают RX и TX буфферы соответственно.

- NRF24_Set_rx_addr(_DMA)\
Записывает адреса 1-5 труб приёмников.

- NRF24_Set_P0_TX_addr(_DMA)\
Записывает адрес нулевой трубы и трубы "передачи"(TX), важно чтобы они были одинаковыми.

- NRF24_Read_RX(_DMA)\
Считывает полученный данные из RX payload

- NRF24_RX_PAYLOAD_LENGHT(_DMA)\
Считывает длину полученных данный из RX payload (только с DPL).

- NRF24_Read_RX(_DMA)_DPL\
Считывает полученный данные из RX payload (только с DPL).

# Настройка NRF24L01+
## Основная настройка 
Инициализировав всю необходимую периферию, подняв CSN и опустив CE, переходим к базовой настройке датчика, которая должна быть на всех устройствах одинаковой. Примеры настроек будут представлены для 2 модулей.
- Включаем автоподтверждение для труб, это необязательно, но желательно. При получении данных приёмник будет посылать автоподтверждение на передатчик и у того, если есть, будет генерироваться прерывание(см. Прерывания).
```
NRF24_Write_Reg(NRF24_REG_EN_AA, 1 << NRF24_ENAA_P0); // автоподтверждение первой трубы
```
- Устанавливаем длину адреса (1, 3, 5 байт).
```
NRF24_Write_Reg(NRF24_REG_SETUP_AW, NRF24_SETUP_AW_5bytes); // длина - 5 байт
```
- Записываем адреса в их регистры.
```
uint8_t rx_txAddr[] = {0xE7, 0x78, 0x78, 0x78, 0x78};
NRF24_Set_P0_TX_addr(rx_txAddr);
NRF24_Set_rx_addr(rx_txAddr, NRF24_REG_RX_ADDR_P0);
```
- Устанавливаем количество повторных отправок и интервал между ними (работает только при влючённом автоподтверждении).
```
NRF24_Write_Reg(NRF24_REG_SETUP_RETR, NRF24_SETUP_RETR_ARD_250uS | NRF24_SETUP_RETR_ARC_15); // 15 повторов с интервалов 250 мкс
```
- Выбираем частотный канал.
```
NRF24_Write_Reg(NRF24_REG_RF_CH, 76); // F = 2.400 MHz + 76
```
- Настраиваем скорость передачи и мощность передатчика.
```
NRF24_Write_Reg(NRF24_REG_RF_SETUP, NRF24_DataRate_1M | NRF24_OutputPower_M18dBm); //Output power:0dBm, Datarate:1Mbps
```
### Без DPL
- Устанавливаем длину payload для всех труб.
```
NRF24_Write_Reg(NRF24_REG_RX_PW_P0, 32); // длина payload для 0 трубы - 32 байта
```
### С DPL
DPL(dynamic payload lenght) - это динамическая длина RX payload, благодаря ей не нужно настраивать длину payload для всех труб.
- Включаем DPL
```
NRF24_Write_Reg(NRF24_REG_FEATURE, 1 << NRF24_EN_DPL);
```
- Включаем DPL для всех труб
```
NRF24_Write_Reg(NRF24_REG_DPL, 1 << NRF24_DPL_P0); // DPL для 0 трубы
```
Далее настройка режимов будет с использованием DPL.
## TX mode (передача)
Для TX режима необходимо настроить регистр CONFIG следующем образом: PWR_UP = 1, PRIM_RX = 0, EN_CRC = 1. Затем очищаем RX и TX буферы и записываем данные в TX payload. После этого поднимаем CE. Теперь датчик передаёт данные из буфера, после некоторой задержки опускаем CE. 

В режим передатчика рекомендуется входить с записанными в TX payload данными, т.е. перед поднятием CE и выходить из него после конца передачи.
```
	uint8_t str[] = "Hello STM32 by nRF24\n";
	NRF24_Write_Reg(NRF24_REG_CONFIG, 1 << NRF24_PWR_UP | 0 << NRF24_PRIM_RX | 1 << NRF24_EN_CRC);
	NRF24_FLUSH_TX();
	NRF24_FLUSH_RX();
	NRF24_Write_Payload(str, sizeof(str));
	NRF24_CE_HIGH;
	delay_uS(150);
	NRF24_CE_LOW;
```
## RX mode
Для TX режима необходимо настроить регистр CONFIG следующем образом: PWR_UP = 1, PRIM_RX = 1, EN_CRC = 1. Затем очищаем RX и TX буферы и поднимаем CE. После получения данных генерируется прерывание, тогда можно считывать данные с RX payload.
```
NRF24_Write_Reg(NRF24_REG_CONFIG, 1 << NRF24_PWR_UP | 1 << NRF24_PRIM_RX | 1 << NRF24_EN_CRC);
NRF24_FLUSH_RX();
NRF24_FLUSH_TX();
NRF24_CE_HIGH;
while(!(nrf24_irq_flaq)); // Флаг прерывания
uint8_t result[32];
uint8_t stat = NRF24_Read_RX_DPL(result);
```
# Description
A library for NRF24L01+ for STM32 microcontrollers using CMSIS. This documentation will not cover the NRF24L01+ registers or command descriptions, as they are available in the official datasheet. Additionally, it will not cover the configuration of SPI and DMA peripherals, delay functions, or pin settings, as these may differ between various microcontroller models. The repository includes an example for STM32F030F4P6, and it is recommended to refer to it when setting up peripherals.

The header file nrf24.h contains masks for NRF24 registers and bits, function declarations, commands, and some helper utilities. It also includes register structures that significantly simplify sensor debugging.

# Functions
Functions with and without DMA perform the same tasks.

- NRF24_Read_Reg(_DMA)
Reads a value from a register.

- NRF24_Write_Reg(_DMA)
Writes a value to a register, overwriting the ENTIRE register.

- NRF24_Write_Bit(_DMA)
Writes a SINGLE bit to a register without overwriting the entire register.

- NRF24_Write_Reg_Multiple(_DMA)
Similar to NRF24_Write_Reg, but writes data to a register larger than 1 byte (e.g., an address).

- NRF24_Write_Payload(_DMA)
Writes data to the TX payload register.

- NRF24_Clear_Status(_DMA)
Clears status bits in the STATUS register.

- NRF24_FLUSH_RX(_DMA), NRF24_FLUSH_TX(_DMA)
Clears the RX and TX buffers, respectively.

- NRF24_Set_rx_addr(_DMA)
Writes addresses for receiver pipes 1–5.

- NRF24_Set_P0_TX_addr(_DMA)
Writes the address for pipe 0 and the "transmit" (TX) pipe; these must be identical.

- NRF24_Read_RX(_DMA)
Reads received data from the RX payload.

- NRF24_RX_PAYLOAD_LENGTH(_DMA)
Reads the length of received data from the RX payload (only with DPL enabled).

- NRF24_Read_RX(_DMA)_DPL
Reads received data from the RX payload (only with DPL enabled).

# Configuring NRF24L01+
## Basic Setup
After initializing all necessary peripherals, setting CSN high and CE low, proceed to the basic sensor configuration, which should be identical on all devices. Configuration examples are provided for two modules.

- Enable auto-acknowledgment for pipes. This is optional but recommended. Upon receiving data, the receiver will send an auto-acknowledgment to the transmitter, which will generate an interrupt if enabled (see Interrupts).
```
NRF24_Write_Reg(NRF24_REG_EN_AA, 1 << NRF24_ENAA_P0); // auto-acknowledgment for pipe 0
```
- Set the address length (1, 3, or 5 bytes).
```
NRF24_Write_Reg(NRF24_REG_SETUP_AW, NRF24_SETUP_AW_5bytes); // length - 5 bytes
```
- Write addresses to their registers.
```
uint8_t rx_txAddr[] = {0xE7, 0x78, 0x78, 0x78, 0x78};
NRF24_Set_P0_TX_addr(rx_txAddr);
NRF24_Set_rx_addr(rx_txAddr, NRF24_REG_RX_ADDR_P0);
```
- Set the number of retransmissions and the interval between them (only works when auto-acknowledgment is enabled).
```
NRF24_Write_Reg(NRF24_REG_SETUP_RETR, NRF24_SETUP_RETR_ARD_250uS | NRF24_SETUP_RETR_ARC_15); // 15 retries with a 250 µs interval
```
- Select the frequency channel.
```
NRF24_Write_Reg(NRF24_REG_RF_CH, 76); // F = 2.400 MHz + 76
```
- Configure the data rate and transmitter power.
```
NRF24_Write_Reg(NRF24_REG_RF_SETUP, NRF24_DataRate_1M | NRF24_OutputPower_M18dBm); // Output power: 0 dBm, Datarate: 1 Mbps
```
### Without DPL
Set the payload length for all pipes.
```
NRF24_Write_Reg(NRF24_REG_RX_PW_P0, 32); // payload length for pipe 0 - 32 bytes
```
### With DPL
DPL (Dynamic Payload Length) enables dynamic RX payload length, eliminating the need to configure payload length for all pipes.
- Enable DPL.
```
NRF24_Write_Reg(NRF24_REG_FEATURE, 1 << NRF24_EN_DPL);
```
- Enable DPL for all pipes.
```
NRF24_Write_Reg(NRF24_REG_DPL, 1 << NRF24_DPL_P0); // DPL for pipe 0
```
Subsequent mode configurations assume DPL is enabled.

## TX Mode (Transmission)
For TX mode, configure the CONFIG register as follows: PWR_UP = 1, PRIM_RX = 0, EN_CRC = 1. Then clear the RX and TX buffers and write data to the TX payload. After that, set CE high. The sensor will now transmit data from the buffer. After a short delay, set CE low.

It is recommended to enter transmitter mode with data already written to the TX payload (i.e., before setting CE high) and exit after transmission is complete.
```
uint8_t str[] = "Hello STM32 by nRF24\n";
NRF24_Write_Reg(NRF24_REG_CONFIG, 1 << NRF24_PWR_UP | 0 << NRF24_PRIM_RX | 1 << NRF24_EN_CRC);
NRF24_FLUSH_TX();
NRF24_FLUSH_RX();
NRF24_Write_Payload(str, sizeof(str));
NRF24_CE_HIGH;
delay_uS(150);
NRF24_CE_LOW;
```
## RX Mode (Reception)
For RX mode, configure the CONFIG register as follows: PWR_UP = 1, PRIM_RX = 1, EN_CRC = 1. Then clear the RX and TX buffers and set CE high. Upon receiving data, an interrupt is generated, after which data can be read from the RX payload.
```
NRF24_Write_Reg(NRF24_REG_CONFIG, 1 << NRF24_PWR_UP | 1 << NRF24_PRIM_RX | 1 << NRF24_EN_CRC);
NRF24_FLUSH_RX();
NRF24_FLUSH_TX();
NRF24_CE_HIGH;
while(!(nrf24_irq_flag)); // Interrupt flag
uint8_t result[32];
uint8_t stat = NRF24_Read_RX_DPL(result);
```
