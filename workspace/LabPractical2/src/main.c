/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
//Gonna work backwards like we do with the labs

int globalcount = 0;	//this should be set in the tim7 interrupt
int enable = 0;		//enable variable set by ISR

void TIM7_IRQHandler(void) {
	//this is the call of the interrupt
	TIM7->SR &= ~TIM_SR_UIF;
	if(getkey() == '8'){
		enable = 1;
		globalcount = 0;
	}
	if(enable == 1){
		globalcount++;
	}
	if(getkey() == 'c') {
		enable = 0;
	}

	char buffer[20];
	sprintf(buffer, "%08d",globalcount);
	for(int i = 0; i < strlen(buffer); i++){
		putchar(buffer[i]);
	}
	//sprintf(buffer, "%08d",globalcount);

	//putchar(buffer);
	//carriage return
	putchar('\r');
}

void setup_tim7(void){
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
	TIM7->PSC = 48000-1;		//48000000/48000 = 1000
	TIM7->ARR = 100-1;		//1000/100 = 10 gives us 10 times a second
	TIM7->DIER |= TIM_DIER_UIE;
	NVIC->ISER[0] = 1<<TIM7_IRQn;
	TIM7->CR1 |= TIM_CR1_CEN;
	//
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= 0x3;

}

//Lab 7 including thinking the getkey is what i want for most of this
void enable_ports()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER &= 0xffc00000;
	GPIOC->MODER |= 0x155555;
	GPIOB->MODER &= 0xffff0000;
	GPIOB->MODER |= 0x55;
	GPIOB->PUPDR &= 0xffff00ff;
	GPIOB->PUPDR |= 0xaa00;
}

char offset;
char history[16];
//char display[8];
char queue[2];
int  qin;
int  qout;

//============================================================================
// set_row()    (Autotest #5)
// Set the row active on the keypad matrix.
// Parameters: none
//============================================================================
void set_row()
{
	int row = offset & 3;
	GPIOB->BSRR = 0xf0000 | (1 << row);
}

//============================================================================
// get_cols()    (Autotest #6)
// Read the column pins of the keypad matrix.
// Parameters: none
// Return value: The 4-bit value read from PC[7:4].
//============================================================================
int get_cols()
{

	return ((GPIOB->IDR >> 4) & 0xf);
}

//============================================================================
// insert_queue()    (Autotest #7)
// Insert the key index number into the two-entry queue.
// Parameters: n: the key index number
//============================================================================
void insert_queue(int n)
{
	    n |= 0x80;
	    queue[qin] = n;
	    qin ^= 0x1;
}

//============================================================================
// update_hist()    (Autotest #8)
// Check the columns for a row of the keypad and update history values.
// If a history entry is updated to 0x01, insert it into the queue.
// Parameters: none
//============================================================================
void update_hist(int cols)
{
	int row = offset & 3;
	for(int i = 0; i<4; i++){
	    history[4*row+i] = (history[4*row+i]<<1) +((cols>>i)&1);
	    if(history[4*row+i] == 0x1)
		{ insert_queue(4*row+i);}}
}

//============================================================================
// getkey()    (Autotest #11)
// Wait for an entry in the queue.  Translate it to ASCII.  Return it.
// Parameters: none
// Return value: The ASCII value of the button pressed.
//============================================================================
int getkey()
{
	int holder = 0;
	for(;;){
	    asm volatile ("wfi" : :);
	    if((queue[qout]) == 0)
		continue;
	    else{
		holder = queue[qout];
		queue[qout] = 0;
		qout ^= 0x1;
		//holder &= 0x7f;
	    }
	    holder &= 0x7f;
	    if(holder == 0)
		return '1';
	    if(holder == 1)
	    	return '2';
	    if(holder == 2)
		return '3';
	    if(holder == 3)
	    	return 'A';
	    if(holder == 4)
    		return '4';
    	    if(holder == 5)
    	    	return '5';
    	    if(holder == 6)
    		return '6';
    	    if(holder == 7)
   	    	return 'B';
    	    if(holder == 8)
    		return '7';
    	    if(holder == 9)
    	    	return '8';
    	    if(holder == 10)
    		return '9';
    	    if(holder == 11)
    	    	return 'C';
    	    if(holder == 12)
    		return '*';
    	    if(holder == 13)
    	    	return '0';
    	    if(holder == 14)
    		return '#';
    	    if(holder == 15)
    	    	return 'D';
	}
    //return 0; // replace this
}

//============================================================================
// This is a partial ASCII font for 7-segment displays.
// See how it is used below.
//============================================================================
const char font[] = {
        [' '] = 0x00,
        ['0'] = 0x3f,
        ['1'] = 0x06,
        ['2'] = 0x5b,
        ['3'] = 0x4f,
        ['4'] = 0x66,
        ['5'] = 0x6d,
        ['6'] = 0x7d,
        ['7'] = 0x07,
        ['8'] = 0x7f,
        ['9'] = 0x67,
        ['A'] = 0x77,
        ['B'] = 0x7c,
        ['C'] = 0x39,
        ['D'] = 0x5e,
        ['*'] = 0x49,
        ['#'] = 0x76,
        ['.'] = 0x80,
        ['?'] = 0x53,
        ['b'] = 0x7c,
        ['r'] = 0x50,
        ['g'] = 0x6f,
        ['i'] = 0x10,
        ['n'] = 0x54,
        ['u'] = 0x1c,
};

//Time for lab 10 stuff.

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

int __io_putchar(int ch){
	return simple_putchar(ch);
	//return better_putchar(ch);
}

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

// bummer was not able to get it finished and printing i wish the answers for some things would be put around

int main(void)
{
	setup_tim7();
	setup_usart5();

	//Think I need these not sure
	//setbuf(stdin, 0);
	//setbuf(stdout, 0);
	//setbuf(stderr, 0);

	for(;;);
}
