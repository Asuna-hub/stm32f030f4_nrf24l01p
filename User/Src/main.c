#include "main.h"

void System_inits(void){
	RCC_Init();
	SysTick_Init();
	DMA_SPI_Init();
	nrf_irq_init();
}
int main(void){
	System_inits();
	while(1);
}