.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

//===================================================================
// ECE 362 Lab Experiment 5
// Timers
//===================================================================

// RCC configuration registers
.equ  RCC,      0x40021000
.equ  AHBENR,   0x14
.equ  GPIOFEN,  0x00800000
.equ  GPIODEN,  0x00200000
.equ  GPIOCEN,  0x00080000
.equ  GPIOBEN,  0x00040000
.equ  GPIOAEN,  0x00020000
.equ  APB1ENR,  0x1c
.equ  TIM6EN,   1<<4
.equ  TIM7EN,   1<<5
.equ  TIM14EN,  1<<8

// NVIC configuration registers
.equ NVIC, 0xe000e000
.equ ISER, 0x100
.equ ICER, 0x180
.equ ISPR, 0x200
.equ ICPR, 0x280
.equ IPR,  0x400
.equ TIM14_IRQn, 19
.equ TIM6_DAC_IRQn, 17
.equ TIM7_IRQn, 18

// Timer configuration registers
.equ TIM6, 0x40001000
.equ TIM7, 0x40001400
.equ TIM14, 0x40002000
.equ TIM_CR1,  0x0
.equ TIM_CR2,  0x4
.equ TIM_DIER, 0xc
.equ TIM_SR,   0x10
.equ TIM_EGR,  0x14
.equ TIM_CNT,  0x24
.equ TIM_PSC,  0x28
.equ TIM_ARR,  0x2c

// Timer configuration register bits
.equ TIM_CR1_CEN,  1<<0
.equ TIM_DIER_UDE, 1<<8
.equ TIM_DIER_UIE, 1<<0
.equ TIM_SR_UIF,   1<<0

// GPIO configuration registers
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  GPIOA,    0x48000000
.equ  MODER,    0x00
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28


.global setup_pins
setup_pins:
	push {r0,r1,r2,r3, lr}
	//enable B-F
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOBEN
	ldr r3, =GPIOCEN
	orrs r2, r3
	ldr r3, =GPIODEN
	orrs r2, r3
	ldr r3, =GPIOFEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	//B in: 4,5,6,7,10,14,8 //potential c 8
	//B out: 0,1,2,3,11,13,15,9
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]
	ldr r2, =0xFCFFFFFF		//set pins 0-7 to 11
	ldr r3, =0x44440055		//setting pins 0-15 to not touching 12
	bics r1, r2				//CLEAR
	orrs r1, r3				//Set 4,5,6,7,10,14,8 to input while setting 0,1,2,3,11,13,15,9 to ouytput
	str r1, [r0, #MODER]	//store
	//pull up: 4,5
	//pull down: 6,7,10
	ldr r1, [r0, #PUPDR]
	ldr r2, =0x30FF00
	bics r1, r2
	ldr r2, =0x20c500
	orrs r1, r2
	str r1, [r0, #PUPDR]

	//Time for C inputs 10,11,12
	//outputs: 0,1,2,3,4,5,6,7,9
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]
	ldr r2, =0x03FCFFFF	//set pins 0-7 to 11
	ldr r3, =0x00045555	//setting pins 0-15 to not touching 12
	bics r1, r2				//CLEAR
	orrs r1, r3				//Set 4,5,6,7,10,14,8 to input while setting 0,1,2,3,11,13,15,9 to ouytput
	str r1, [r0, #MODER]
	//pull up:11
	//pull down: 10
	ldr r1, [r0, #PUPDR]
	ldr r2, =0xF00000
	bics r1, r2
	ldr r2, =0x600000
	orrs r1, r2
	str r1, [r0, #PUPDR]

	pop {r0,r1,r2,r3, pc}

.global setup_timer
setup_timer:
	push {r0-r3,lr}
	//Enable time 14
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]
	ldr r2, =TIM14EN
	orrs r1, r2
	str r2, [r0, #APB1ENR]

	//PSC moves
	ldr r0, =TIM14
	ldr r1, =65536-1
	ldr r2, =2518-1
	str r1, [r0, #TIM_PSC]
	str r2, [r0, #TIM_ARR]
	ldr r2, [r0, #TIM_DIER]
	ldr r1, =TIM_DIER_UIE
	orrs r1, r2
	str r1, [r0, #TIM_DIER]
	ldr r2, [r0, #TIM_CR1]
	ldr r1, =TIM_CR1_CEN
	orrs r1, r2
	str r1, [r0, #TIM_CR1]
	ldr r0, =NVIC
	ldr r3, =ISER
	ldr r2, =1<<TIM14_IRQn
	str r2, [r0, r3]

	pop {r0-r3,pc}

.global TIM14_IRQHandler
.type TIM14_IRQHandler, %function
TIM14_IRQHandler:
	push {r0-r3,lr}
	push {r0-r4,lr}
	ldr r0, =TIM14		//location of TIM6
	ldr r1, [r0, #TIM_SR]
	ldr r2, =TIM_SR_UIF	//i think this is right part 2.2
	mvns r2, r2
	ands r1, r2
	str r1, [r0, #TIM_SR]

	ldr r0, =GPIOB
	ldr r1, [r0, #ODR]
	movs r3, #1
	bics r1, r3
	movs r2, #2
	orrs r1, r2
	str r1, [r0, #ODR]

	ldr r1, [r0, IDR]
	movs r2, r1
	lsrs r2, r2, #7
	cmp r2, #1
	beq toggle
	bne end

end:
	pop {r0-r3,pc}


toggle:
	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	ldr r2, =0xff
	eors r1, r2
	str r1, [r0, #ODR]
	b end


//not sure if you want us to have a main or not
.global main
main:
	bl setup_pins
	bl setup_timer

//endless_loop:

//	b endless_loop
	//bl an_ISR
