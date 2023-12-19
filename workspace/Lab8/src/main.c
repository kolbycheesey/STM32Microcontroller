
//============================================================================
// ECE 362 lab experiment 8 -- SPI and DMA
//============================================================================

#include "stm32f0xx.h"
#include "lcd.h"
#include <stdio.h> // for sprintf()

// Be sure to change this to your login...
const char login[] = "chen2774";

// Prototypes for misc things in lcd.c
void nano_wait(unsigned int);

// Write your subroutines below.
void setup_bb()
{
	RCC->AHBENR|= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= ~0xcf000000;
	GPIOB->MODER |= 0x45000000;
	GPIOB->ODR &= ~(1 << 13);
	GPIOB->ODR |= (1 << 12);
}
void small_delay()
{
	nano_wait(10000000);
}
void bb_write_bit(int i)
{
	GPIOB->ODR &= ~(1 << 15);
	GPIOB->ODR |= (i << 15);
	small_delay();
	GPIOB->ODR |= (1 << 13);
	small_delay();
	GPIOB->ODR &= ~(1 << 13);
}
void bb_write_byte(int i)
{
	int k = i >> 7;
	k &= 1;
	bb_write_bit(k);
	k = i >> 6;
	k &= 1;
	bb_write_bit(k);
	k = i >> 5;
	k &= 1;
	bb_write_bit(k);
	k = i >> 4;
	k &= 1;
	bb_write_bit(k);
	k = i >> 3;
	k &= 1;
	bb_write_bit(k);
	k = i >> 2;
	k &= 1;
	bb_write_bit(k);
	k = i >> 1;
	k &= 1;
	bb_write_bit(k);
	k = i >> 0;
	k &= 1;
	bb_write_bit(k);

}
void bb_cmd(int i)
{
	GPIOB->ODR &= ~(1 << 12);
	small_delay();
	bb_write_bit(0);
	bb_write_bit(0);
	bb_write_byte(i);
	small_delay();
	GPIOB->ODR |= (1 << 12);
	small_delay();
}
void bb_data(int i)
{
	GPIOB->ODR &= ~(1 << 12);
	small_delay();
	bb_write_bit(1);
	bb_write_bit(0);
	bb_write_byte(i);
	small_delay();
	GPIOB->ODR |= (1 << 12);
	small_delay();
}
void bb_init_oled()
{
	nano_wait(1000000);
	bb_cmd(0x38);
	bb_cmd(0x08);
	bb_cmd(0x01);
	nano_wait(2000000);
	bb_cmd(0x06);
	bb_cmd(0x02);
	bb_cmd(0x0c);
}
void bb_display1(const char *i)
{
	bb_cmd(0x02);
	for (int k = 0; i[k] != '\0'; k++)
	{
			bb_data(i[k]);
	}
}
void bb_display2(const char *i)
{
	bb_cmd(0xc0);
	for (int k = 0; i[k] != '\0'; k++)
	{
			bb_data(i[k]);
	}
}
void setup_spi2()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	GPIOB->MODER &= ~(0xcf000000);
	GPIOB->MODER |= 0x8a000000;
	SPI2->CR1 |= SPI_CR1_BR;
	SPI2->CR2 = SPI_CR2_DS_0 | SPI_CR2_DS_3 | SPI_CR2_SSOE | SPI_CR2_NSSP;
	SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
	SPI2->CR1 |= SPI_CR1_SPE;
}
void spi_cmd(int i)
{
	for(;;)
	{
		if(SPI2->SR & SPI_SR_TXE)
		{
			break;
		}
	}
	SPI2->DR |= i;
}
void spi_data(int i)
{
	for(;;)
		{
			if(SPI2->SR & SPI_SR_TXE)
			{
				break;
			}
		}
		SPI2->DR |= i | 0x200;

}
void spi_init_oled()
{
	nano_wait(1000000);
	spi_cmd(0x38);
	spi_cmd(0x08);
	spi_cmd(0x01);
	nano_wait(2000000);
	spi_cmd(0x06);
	spi_cmd(0x02);
	spi_cmd(0x0c);
}
void spi_display1(const char *i)
{
	spi_cmd(0x02);
	for (int k = 0; i[k] != '\0'; k++)
	{
		spi_data(i[k]);
	}
}
void spi_display2(const char *i)
{
	spi_cmd(0xc0);
	for (int k = 0; i[k] != '\0'; k++)
	{
		spi_data(i[k]);
	}
}
// Write your subroutines above.

void show_counter(short buffer[])
{
    for(int i=0; i<10000; i++) {
        char line[17];
        sprintf(line,"% 16d", i);
        for(int b=0; b<16; b++)
            buffer[1+b] = line[b] | 0x200;
    }
}
void spi_enable_dma(const short *i)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel5->CPAR = &SPI2->DR;
	DMA1_Channel5->CMAR = (uint32_t)i;
	DMA1_Channel5->CNDTR = 34;
	DMA1_Channel5->CCR |= DMA_CCR_DIR;
	DMA1_Channel5->CCR |= DMA_CCR_MINC;
	DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;
	DMA1_Channel5->CCR |= DMA_CCR_CIRC;
	DMA1_Channel5->CCR |= DMA_CCR_EN;
	for(;;)
	{
		if((SPI2->SR & SPI_SR_TXE) == SPI_SR_TXE)
		{
			break;
		}
	}
	SPI2->CR2 |= SPI_CR2_TXDMAEN;

}
void setup_spi1()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~0xcff0;
	GPIOA->MODER |= 0x8a00;
	GPIOA->MODER |= 0X50;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_BR;
	SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
	SPI1->CR2 = SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_SSOE | SPI_CR2_NSSP;
	SPI1->CR1 |= SPI_CR1_SPE;
}
void internal_clock();
void demo();
void autotest();

extern const Picture *image;

int main(void)
{
    //internal_clock();
    //demo();
    autotest();

    //setup_bb();
    //bb_init_oled();
    //bb_display1("Hello,");
    //bb_display2(login);

    //setup_spi2();
    //spi_init_oled();
    //spi_display1("Hello again,");
    //spi_display2(login);

    short buffer[34] = {
            0x02, // This word sets the cursor to the beginning of line 1.
            // Line 1 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0xc0, // This word sets the cursor to the beginning of line 2.
            // Line 2 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
    };

    //spi_enable_dma(buffer);
    show_counter(buffer);

    //setup_spi1();
    LCD_Init();
    LCD_Clear(BLACK);
    LCD_DrawLine(10,20,100,200, WHITE);
    LCD_DrawRectangle(10,20,100,200, GREEN);
    LCD_DrawFillRectangle(120,20,220,200, RED);
    LCD_Circle(50, 260, 50, 1, BLUE);
    LCD_DrawFillTriangle(130,130, 130,200, 190,160, YELLOW);
    LCD_DrawChar(150,155, BLACK, WHITE, 'X', 16, 1);
    LCD_DrawString(140,60,  WHITE, BLACK, "ECE 362", 16, 0);
    LCD_DrawString(140,80,  WHITE, BLACK, "has the", 16, 1);
    LCD_DrawString(130,100, BLACK, GREEN, "best toys", 16, 0);
    LCD_DrawPicture(110,220,(const Picture *)&image);
}


/*
//============================================================================
// ECE 362 lab experiment 8 -- SPI and DMA
//============================================================================

#include "stm32f0xx.h"
#include "lcd.h"
#include <stdio.h> // for sprintf()

// Be sure to change this to your login...
const char login[] = "kolb3";

// Prototypes for misc things in lcd.c
void nano_wait(unsigned int);

// Write your subroutines below.

void setup_bb(){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;  	//enable b
	GPIOB->MODER &= 0x30FFFFFF; 		//previously used ~(0xCF000000);
	GPIOB->MODER |= 0x45000000;

	//RCC->AHBENR |= RCC_APB1ENR_SPI2EN;	//uh not sure if this should be before
	GPIOB->ODR &= ~(0x1 << 13);
	GPIOB->ODR |= 0x1 << 12;			//think this should work

}

void small_delay(){
	nano_wait(10000);
}

void bb_write_bit(int i){

	GPIOB->ODR &= 0xbfff;
	GPIOB->ODR |= i << 15;
	small_delay();

	GPIOB->ODR |= 0x1 << 13;
	small_delay();

	GPIOB->ODR &= 0xdff;
	small_delay();
}

void bb_write_byte(int i){
	bb_write_bit(i>>7 & 1);
	bb_write_bit(i>>6 & 1);
	bb_write_bit(i>>5 & 1);
	bb_write_bit(i>>4 & 1);
	bb_write_bit(i>>3 & 1);
	bb_write_bit(i>>2 & 1);
	bb_write_bit(i>>1 & 1);
	bb_write_bit(i>>0 & 1);
}

void bb_cmd(int x){
	//set the NSS pin low
	GPIOB->ODR &= ~(0x1 << 12);	//set NSS to low
	small_delay();
	bb_write_bit(0);		//RS
	bb_write_bit(0);		//R/W
	bb_write_byte(x);
	small_delay();
	GPIOB->ODR |= (0x1 << 12);	//set NSS to high
	small_delay();
}

void bb_data(int x){
	GPIOB->ODR &= ~(0x1 << 12);	//set NSS to low
	small_delay();
	bb_write_bit(1);
	bb_write_bit(0);
	bb_write_byte(x);
	small_delay();
	GPIOB->ODR |= (0x1 << 12);	//set NSS to high
	small_delay();
}

void bb_init_oled(){
	nano_wait(1000000);
	bb_cmd(0x38);
	bb_cmd(0x08);
	bb_cmd(0x01);
	nano_wait(2000000);
	bb_cmd(0x06);
	bb_cmd(0x02);
	bb_cmd(0x0c);
}

void bb_display1(const char * stuff) {
	bb_cmd(0x02);
	int i = 0;
	while(stuff[i] != '\0'){
		bb_data(stuff[i]);
		i++;
	}
}

void bb_display2(const char * wordVariable){
	bb_cmd(0xc0);
	int i = 0;
	while(wordVariable[i] != '\0'){
		bb_data(wordVariable[i]);
		i++;
	}
}
void setup_spi2(){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	GPIOB->MODER &= ~(0xcf000000);
	GPIOB->MODER |= 0x8a000000;
	//RCC->AHBENR |= RCC_APB1ENR_SPI2EN;	//uh not sure if this should be before
	//SPI2->CR1 &= ~(0xc038);
	//SPI2->CR1 |= (0xc038);
	SPI2->CR1 |= SPI_CR1_BR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_SPE | SPI_CR1_MSTR;
	//SPI2->CR2 &= ~(0x0f0c);
	//SPI2->CR2 |= (0x090c);
	SPI2->CR2 = SPI_CR2_DS_3 | SPI_CR2_DS_0 | SPI_CR2_NSSP | SPI_CR2_SSOE;
	//SPI->CR2 = SPI_CR2_DS | SPI__CR2_NSSP | SPI_CR2_SSOE;
}

void spi_cmd(int x){
	//wait for txe bit to be 1
	while(!(SPI2->SR & SPI_SR_TXE)){
		continue;
	}
	SPI2->DR |= x;
}

void spi_data(int x){
	//same thing as cmd but or 0x200 into value x before saving it
	//wait for txe bit to be 1
	x |= 0x200;
	while(!(SPI2->SR & SPI_SR_TXE)){
		continue;
	}
	SPI2->DR |= x;
}

void spi_init_oled(){
	nano_wait(1000000);
	spi_cmd(0x38);
	spi_cmd(0x08);
	spi_cmd(0x01);
	nano_wait(2000000);
	spi_cmd(0x06);
	spi_cmd(0x02);
	spi_cmd(0x0c);
}

void spi_display1(const char * newWord){
	spi_cmd(0x02);
	int i = 0;
	while(newWord[i] != '\0'){
		spi_data(newWord[i]);
		i++;
	}
}

void spi_display2(const char * newerWord){
	spi_cmd(0xc0);
	int i = 0;
	while(newerWord[i] != '\0'){
		spi_data(newerWord[i]);
		i++;
	}
}

void spi_enable_dma(const short * thingy){
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel5->CPAR = &SPI2->DR;
	DMA1_Channel5->CMAR = thingy;
	DMA1_Channel5->CNDTR = 34;
	DMA1_Channel5->CCR |= DMA_CCR_DIR;
	DMA1_Channel5->CCR |= DMA_CCR_MINC;
	DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;
	DMA1_Channel5->CCR |= DMA_CCR_CIRC;
	DMA1_Channel5->CCR |= DMA_CCR_EN;
	for(;;){
		if((SPI2->SR & SPI_SR_TXE) == SPI_SR_TXE){
			break;
		}
	}
	SPI2->CR2 |= SPI_CR2_TXDMAEN;
}

void setup_spi1(){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(0x0000cff0);
	GPIOA->MODER |= 0x00008a50;
	//GPIOA -> AFR[0] &= ~(0xf0ff0000)
	RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_BR;
	SPI1->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR;
	SPI1->CR2 = SPI_CR2_NSSP | SPI_CR2_SSOE | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
	SPI1->CR1 |= SPI_CR1_SPE;
}



// Write your subroutines above.

void show_counter(short buffer[])
{
    for(int i=0; i<10000; i++) {
        char line[17];
        sprintf(line,"% 16d", i);
        for(int b=0; b<16; b++)
            buffer[1+b] = line[b] | 0x200;
    }
}
void internal_clock();
void demo();
void autotest();

extern const Picture *image;

int main(void)
{
    //internal_clock();
    //demo();
    autotest();

    setup_bb();
    bb_init_oled();
    bb_display1("Hello,");
    bb_display2(login);

    setup_spi2();
    spi_init_oled();
    spi_display1("Hello again,");
    spi_display2(login);

    short buffer[34] = {
            0x02, // This word sets the cursor to the beginning of line 1.
            // Line 1 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0xc0, // This word sets the cursor to the beginning of line 2.
            // Line 2 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
    };

    spi_enable_dma(buffer);
    show_counter(buffer);

    setup_spi1();
    LCD_Init();
    LCD_Clear(BLACK);
    LCD_DrawLine(10,20,100,200, WHITE);
    LCD_DrawRectangle(10,20,100,200, GREEN);
    LCD_DrawFillRectangle(120,20,220,200, RED);
    LCD_Circle(50, 260, 50, 1, BLUE);
    LCD_DrawFillTriangle(130,130, 130,200, 190,160, YELLOW);
    LCD_DrawChar(150,155, BLACK, WHITE, 'X', 16, 1);
    LCD_DrawString(140,60,  WHITE, BLACK, "ECE 362", 16, 0);
    LCD_DrawString(140,80,  WHITE, BLACK, "has the", 16, 1);
    LCD_DrawString(130,100, BLACK, GREEN, "best toys", 16, 0);
    LCD_DrawPicture(110,220,(const Picture *)&image);
}*/
