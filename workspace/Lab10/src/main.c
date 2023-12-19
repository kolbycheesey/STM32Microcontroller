
//============================================================================
// ECE 362 lab experiment 10 -- Asynchronous Serial Communication
//============================================================================

#include "stm32f0xx.h"
#include "ff.h"
#include "diskio.h"
#include "fifo.h"
#include "tty.h"
#include <string.h> // for memset()
#include <stdio.h> // for printf()

void advance_fattime(void);
void command_shell(void);
int better_putchar(int);
int better_getchar();

// Write your subroutines below.

void setup_usart5(){

	//Enable GPIOC and D
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	RCC->AHBENR |= RCC_AHBENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

	//Configue PC12 to USART5_TX 12 to AF2 and 2 to af2
	GPIOC->MODER |= 2<<(2*12);
	GPIOC->AFR[1] |= 2<<(4*4);

	//Configure PD2 to USART5_RX d 2 to aft
	GPIOD->MODER |= 2<<(2*2);
	GPIOD->AFR[0] |= 2<<(4*2);

	//Configure Usart5 given 2.1 instrucitons
	USART5->CR1 &= ~USART_CR1_UE;
	USART5->CR1 &= ~USART_CR1_M  &  ~USART_CR1_PCE & ~USART_CR1_OVER8; //i think its m but not sure will check again later
	USART5->CR2 &= ~USART_CR2_STOP_1 & ~USART_CR2_STOP_0;
	USART5->BRR = 0x1A1;
	USART5->CR1 = USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;	//page 986

	while(((USART5->ISR & USART_ISR_REACK) != USART_ISR_REACK) && ((USART5->ISR & USART_ISR_TEACK) != USART_ISR_TEACK))
	    {

	    }

}

//2.2
int simple_putchar(int inputx){
	//Wait for ISR TXE to be set
	//Write argument to USART TDR
	//int something = 0;
	while ((USART5->ISR & USART_ISR_TXE) != USART_ISR_TXE)
	    {

	    }
	USART5->TDR = inputx;
	//Return argument that was passed in
	return inputx;
	//Test by uncommenting lines for through 2.2 in main
}

//2.3
int simple_getchar(void){
	//Wait for ISR RXNE bit to be set
	while((USART5->ISR & USART_ISR_RXNE) != USART_ISR_RXNE)
	    {

	    }
	//return value of RDR
	//right shift x spaces and and with 1 to get answer then return it unless rdr is a 16 bit register then just return that bitch
	return USART5->RDR;
	//uncomment section through 2.3
}

//support for printf 2.4
int __io_putchar(int ch){
	//return simple_putchar(ch);
	return better_putchar(ch);
}
int __io_getchar(void){
	//return simple_getchar();
	//return better_getchar();
	//line_buffer_getchar();
	return interrupt_getchar();
}
//after this recomment 2.2 and 2.3 code uncomment 2.4 and 2.5

int better_putchar(int ch){
	if(ch == '\n')
	    {
		while((USART5->ISR & USART_ISR_TXE) != USART_ISR_TXE)
		    {

		    }
		USART5->TDR = '\r';
	    }
	while((USART5->ISR & USART_ISR_TXE) != USART_ISR_TXE)
		{

		}
	USART5->TDR = ch;
	    //}
	return ch;
}
int better_getchar(){
	while((USART5->ISR & USART_ISR_RXNE) != USART_ISR_RXNE)
	    {

	    }
	if(USART5->RDR == '\r')
	    {
		return '\n';
	    }

	return USART5->RDR;
}

int interrupt_getchar(){
	while(fifo_newline(&input_fifo) == 0){
		asm volatile ("wfi");	//wait for interrupt
	}
	char ch = fifo_remove(&input_fifo);
	return ch;

}

void USART3_4_5_6_7_8_IRQHandler (void){
	if(USART5->ISR & USART_ISR_ORE){
		USART5->ICR |= USART_ICR_ORECF;
	}
	char ch = USART5->RDR;
	if(fifo_full(&input_fifo)){
		return ;
	}
	insert_echo_char(ch);
}

void enable_tty_interrupt() {
	USART5->CR1 |= USART_CR1_RXNEIE;
	NVIC->ISER[0] |= 1<<29;

}
// Write your subroutines above.

void setup_spi1() {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(3 << (1*2) | 3 << (5*2) | 3 << (6*2) | 3 << (7*2));
	GPIOA->MODER |= (1 << (1*2) | 2 << (5*2) | 2 << (6*2) | 2 << (7*2));
	GPIOA->PUPDR |= 1 << (2*6);
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_SPE;
	SPI1->CR1 |= SPI_CR1_BR;
	SPI1->CR1 &= ~(SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);
	SPI1->CR1 |= SPI_CR1_MSTR;
	SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_NSSP | SPI_CR2_FRXTH ;
	SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_high_speed() {
	SPI1->CR1 &= ~SPI_CR1_SPE;
	SPI1->CR1 &= ~(SPI_CR1_BR_0 |SPI_CR1_BR_1 | SPI_CR1_BR_2);
	SPI1->CR1 |= SPI_CR1_BR_1;
	SPI1->CR1 |= SPI_CR1_SPE;
}

void TIM14_IRQHandler(void) {
	TIM14->SR &= ~TIM_SR_UIF;
	advance_fattime();
}

void setup_tim14() {
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	TIM14->PSC = 47999;
	TIM14->ARR = 1999;
	TIM14->CR1 |= 1;
	TIM14->DIER |= TIM_DIER_UIE;
	NVIC->ISER[0] |= 1<<19;
}

const char testline[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\r\n";

int main()
{
    setup_usart5();

    // Uncomment these when you're asked to...
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);

    // Test 2.2 simple_putchar()
    //
    //for(;;)
    //    for(const char *t=testline; *t; t++)
    //        simple_putchar(*t);

    // Test for 2.3 simple_getchar()
    //
    //for(;;)
    //    simple_putchar( simple_getchar() );

    // Test for 2.4 and 2.5 __io_putchar() and __io_getchar()
    //
    //printf("Hello!\n");
    //for(;;)
    //    putchar( getchar() );

    // Test for 2.6
    //
    //for(;;) {
    //    printf("Enter string: ");
    //    char line[100];
    //    fgets(line, 99, stdin);
    //    line[99] = '\0'; // just in case
    //    printf("You entered: %s", line);
    //}

    // Test for 2.7
    //
    //enable_tty_interrupt();
    //for(;;) {
    //    printf("Enter string: ");
    //    char line[100];
    //    fgets(line, 99, stdin);
    //    line[99] = '\0'; // just in case
    //    printf("You entered: %s", line);
    //}

    // Test for 2.8 Test the command shell and clock.
    //
    enable_tty_interrupt();
    setup_tim14();
    FATFS fs_storage;
    FATFS *fs = &fs_storage;
    f_mount(fs, "", 1);
    command_shell();

    return 0;
}
