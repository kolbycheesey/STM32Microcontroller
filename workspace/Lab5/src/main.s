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
.equ  GPIOCEN,  0x00080000
.equ  GPIOBEN,  0x00040000
.equ  GPIOAEN,  0x00020000
.equ  APB1ENR,  0x1c
.equ  TIM6EN,   1<<4
.equ  TIM7EN,   1<<5

// NVIC configuration registers
.equ NVIC, 0xe000e000
.equ ISER, 0x100
.equ ICER, 0x180
.equ ISPR, 0x200
.equ ICPR, 0x280
.equ IPR,  0x400
.equ TIM6_DAC_IRQn, 17
.equ TIM7_IRQn, 18

// Timer configuration registers
.equ TIM6, 0x40001000
.equ TIM7, 0x40001400
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

//===========================================================================
// enable_ports  (Autotest 1)
// Enable the RCC clock for GPIO ports B and C.
// Parameters: none
// Return value: none
.global enable_ports
enable_ports:
	push {lr}
	// Student code goes below
	push {r0,r1,r2,r3}
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOBEN
	ldr r3, =GPIOCEN
	orrs r2, r3
	orrs r1, r2
	str r1, [r0, #AHBENR]
	//B and C on now need PB0-3 to be out 4-7 to be in 4-7 also
	//need to be pulled low C0-6 need to be outputs as well
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]
	ldr r2, =0xFFFF	//set pins 0-7 to 11
	ldr r3, =0x55		//setting pins 0-3 to 01 for output
	//mvns r2, r2			//clear 0-7  //think i can bics also
	bics r1, r2			//clear 0-7
	orrs r1, r3			//set 0-3 to 01 for output
	str r1, [r0, #MODER]//store
	//pupdr on 4-7
	ldr r1, [r0, #PUPDR]
	ldr r2, =0xFF00
	bics r1, r2
	ldr r2, =0xAA00
	orrs r1, r2
	str r1, [r0, #PUPDR]
	//GPIOC pins 0-6
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]
	ldr r2, =0x3FFFFF		//clear all bits for pins 0-10
	bics r1, r2
	ldr r2, =0x155555 	//output 01 for pins 0-6
	orrs r1, r2			//turn on pins 0-6
	str r1, [r0, #MODER]//store in MODER for saving
	pop {r0,r1,r2,r3}
	// Student code goes above
	pop  {pc}

//===========================================================================
// Timer 6 Interrupt Service Routine  (Autotest 2)
// Parameters: none
// Return value: none
// Write the entire subroutine below
.global TIM6_DAC_IRQHandler
.type TIM6_DAC_IRQHandler , %function
TIM6_DAC_IRQHandler:
	push {r0-r4,lr}
	ldr r0, =TIM6		//location of TIM6
	ldr r1, [r0, #TIM_SR]
	ldr r2, =TIM_SR_UIF	//i think this is right part 2.2
	mvns r2, r2
	ands r1, r2
	str r1, [r0, #TIM_SR]

	//toggle bit
	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	movs r2, r1			//get current register set
	movs r3, #6			//set r3 as 6
	movs r4, #1			//set r4 as 1 for later
	lsrs r2, r2, r3		//left shift r2 for comparing
	lsls r4, r4, r3		//right shift 1 for changing

	cmp r2, 0x0
	beq changetoone		//logical statement 1
	bne changetozero	//logical statement 2

changetoone:
	orrs r1, r4			//changes 0 to 1
	b end

changetozero:
	bics r1,r4			//changes 1 to 0
	b end

end:
	str r1, [r0, #ODR]
	pop {r0-r4,pc}



//===========================================================================
// setup_tim6  (Autotest 3)
// Configure timer 6
// Parameters: none
// Return value: none
.global setup_tim6
setup_tim6:
	push {lr}
	// Student code goes below  //its on AB1NERa
	push {r0-r3}
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]
	ldr r2, =TIM6EN
	orrs r1, r2
	str r2, [r0, #APB1ENR]

	//PSC moves
	ldr r0, =TIM6
	ldr r1, =48000-1
	ldr r2, =500-1
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
	//ldr r1, [r0, r3]
	ldr r2, =1<<TIM6_DAC_IRQn
	//orrs r1, r2
	str r2, [r0, r3]
	pop {r0-r3}
	// Student code goes above
	pop  {pc}

.data
.global display
display: .byte 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07
.global history
history: .space 16
.global offset
offset: .byte 0
.text

//===========================================================================
// show_digit  (Autotest 4)
// Set up the Port C outputs to show the digit for the current
// value of the offset variable.
// Parameters: none
// Return value: none
// Write the entire subroutine below.
.global show_digit
show_digit:
	push {r0-r3,lr}
	ldr r0, =GPIOC		//will need ODR
	ldr r1, =display
	ldr r2, =offset
	ldrb r2, [r2]		//do i need this?
	movs r3, #7			//saves number 7
	ands r2, r3			//offset & 7
	ldrb r3, [r1, r2]
	lsls r2, r2, #8 	//off << 8
	orrs r2, r3
	//ldr r3, [r0, #ODR]	//load ODR
	//orrs r2, r3			//orrs to not lose any information already in there
	str r2, [r0, #ODR]  //store information in ODR

	pop {r0-r3,pc}

//===========================================================================
// get_cols  (Autotest 5)
// Return the current value of the PC8 - PC4.
// IS it PC or PB cause the lab manual says PB
// Parameters: none
// Return value: 4-bit result of columns active for the selected row
// Write the entire subroutine below.
.global get_cols
get_cols:
	push {r1,r2,lr}

	ldr r0, =GPIOB
	ldr r1, [r0, #IDR]
	lsrs r1, r1, #4
	movs r2, 0xf
	ands r1, r2
	movs r0, r1

	pop {r1,r2,pc}

//===========================================================================
// update_hist  (Autotest 6)
// Update the history byte entries for the current row.
// Parameters: r0: cols: 4-bit value read from matrix columns
// Return value: none
// Write the entire subroutine below.
.global update_hist
update_hist:
	push {r0-r7,lr}
	ldr r1, =history
	ldr r2, =offset
	ldrb r2, [r2]
	movs r3, #3
	ands r3, r2			//r3 is row = offset & 3
	movs r4, #0

forupdateloop:
	cmp r4, #4			//for i=0 i<4 i++
	bge endupdateloop

	//adds r2, r3, r4		//row + i
	movs r2, r3
	lsls r2, r2, #2		//multiply by 4 for byte location
	adds r2, r2, r4
	ldrb r5, [r1, r2]	//load data at data[4*row+i]
	lsls r5, r5, #1			//x << 1 = x
	movs r6, r0
	lsrs r6, r6, r4		//col >> i
	movs r7, #1
	ands r6, r7			//(cols >> i) & 1 = y
	adds r5, r6			//history[x] + y from above
	strb r5, [r1, r2]	//store in history[4*row+i]
	adds r4, r4, #1		//x += 1

	b forupdateloop		//continue loop

endupdateloop:
	pop {r0-r7,pc}		//run


//===========================================================================
// set_row  (Autotest 7)
// Set PB3 - PB0 to represent the row being scanned.
// Parameters: none
// Return value: none
// Write the entire subroutine below.
.global set_row
set_row:
	push {r0-r3,lr}
	ldr r0, =GPIOB
	ldr r1, =offset
	ldr r1, [r1]
	movs r3, #3
	ands r3, r1
	ldr r2, =0xf0000
	movs r1, #1
	lsls r1, r1, r3
	orrs r1, r2

	str r1, [r0, #BSRR]
	pop {r0-r3,pc}

//===========================================================================
// Timer 7 Interrupt Service Routine  (Autotest 8)
// Parameters: none
// Return value: none
// Write the entire subroutine below
.global TIM7_IRQHandler
.type TIM7_IRQHandler, %function
TIM7_IRQHandler:
	push {r0-r3,lr}

	ldr r0, =TIM7		//location of TIM6
	ldr r1, [r0, #TIM_SR]
	ldr r2, =TIM_SR_UIF	//i think this is right part 2.2
	mvns r2, r2
	ands r1, r2
	str r1, [r0, #TIM_SR]


	bl show_digit
	bl get_cols
	bl update_hist
	ldr r1, =offset
	ldr r2, [r1]
	adds r2, #1
	movs r3, 0x7
	ands r2, r3
	str r2, [r1]
	bl set_row

	pop {r0-r3,pc}


//===========================================================================
// setup_tim7  (Autotest 9)
// Configure Timer 7.
// Parameters: none
// Return value: none
.global setup_tim7
setup_tim7:
	push {lr}
	// Student code goes below
	push {r0-r3}
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]
	ldr r2, =TIM6EN
	bics r1, r2				//this should disable time6
	ldr r2, =TIM7EN
	orrs r1, r2
	str r2, [r0, #APB1ENR]

	//1kHz freq is the goal here
	ldr r0, =TIM7
	ldr r1, =4800-1
	ldr r2, =10-1
	str r1, [r0, #TIM_PSC]
	str r2, [r0, #TIM_ARR]

	//enable UIE bit
	ldr r2, [r0, #TIM_DIER]
	ldr r1, =TIM_DIER_UIE
	orrs r1, r2
	str r1, [r0, #TIM_DIER]

	ldr r2, [r0, #TIM_CR1]
	ldr r1, =TIM_CR1_CEN
	orrs r1, r2
	str r1, [r0, #TIM_CR1]

	//Enable the timer7 interrupt in the NVIC ISER
	ldr r0, =NVIC
	ldr r3, =ISER
	ldr r1, [r0, r3]
	ldr r2, =1<<TIM7_IRQn
	orrs r1, r2
	str r1, [r0, r3]
	pop {r0-r3}
	// Student code goes above
	pop  {pc}


//===========================================================================
// get_keypress  (Autotest 10)
// Wait for and return the number (0-15) of the ID of a button pressed.
// Parameters: none
// Return value: button ID
.global get_keypress
get_keypress:
	push {r1-r3,lr}
	// Student code goes below

forkeypress:
	wfi			//believe that this is asm volatile("wfi" : :)
	movs r0, #0
	movs r2, #0
	ldr r1, =offset
	ldrb r1, [r1]

	movs r3, #3
	ands r1, r3
ifkeypress:
	cmp r1, #0
	bne forkeypress

forround2:
	cmp r0, #16
	bge endkeypress

	ldr r1, =history
	ldrb r2, [r1, r0]
ifforkey:
	cmp r2, #1
	bne iteration

	pop {r1-r3,pc}

iteration:
	adds r0, r0, #1
	b forround2

endkeypress:
	b forkeypress


	//b forkeypress


	// Student code goes above
	pop  {pc}


//===========================================================================
// handle_key  (Autotest 11)
// Shift the symbols in the display to the left and add a new digit
// in the rightmost digit.
// ALSO: Create your "font" array just above.
// Parameters: ID of new button to display
// Return value: none

.global font
font: .byte 0x06, 0x5b, 0x4f, 0x77, 0x66, 0x6d, 0x7d, 0x7c, 0x07, 0x7f, 0x67, 0x39, 0x49, 0x3f, 0x76, 0x5e
.text
//.align 2

.global handle_key
handle_key:
	push {lr}
	// Student code goes below
	push {r0-r4}
	ldr r1, =0xf
	ands r0, r1		//key = key & 0xf
	movs r1, #0		//r1 = x
	ldr r2, =display

forhandle:
	cmp r1, #7
	bge endforhandle

	adds r3, r1, #1		//i + 1
	ldrb r4, [r2, r3]	//load display[i+1]
	//subs r1, r1, #1
	strb r4, [r2, r1]	//store in display[i]
	adds r1, #1		//increment i

	b forhandle

endforhandle:
	ldr r3, =font
	ldrb r4, [r3, r0]	//ldr font[key]
	strb r4, [r2, r1]	//str display[7]

	pop {r0-r4}
	// Student code goes above
	pop  {pc}

.global login
login: .string "kolb3"
.align 2

//===========================================================================
// main
// Already set up for you.
// It never returns.
.global main
main:
	bl  check_wiring
	//bl  autotest
	bl  enable_ports
	bl  setup_tim6
	bl  setup_tim7

endless_loop:
	bl   get_keypress
	bl   handle_key
	b    endless_loop
