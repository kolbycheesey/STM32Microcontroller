.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.data
.align 4
//global variables here
.global value
.global source
.global str

value: 	.word 0 //int value
source: .word 1, 2, 2, 4, 5, 9, 12, 8, 9, 10, 11
str: 	.asciz "HELLO, 01234 WorlD! 56789+-"


.text
.global intsub
intsub:
	//code for intsub function here
		ldr r0, =source
		push {r4, r5}		//have to push initially
		ldr r5, =value		//saves on ldrs
		movs r4, #0 		//for int i = 0


for1: //for loop
		cmp r4, #10
		bge endfor1

if1:  //if((i & 2) == 2) bit wise and checks to see if the 2s spot is covered by a 1 or 0 if true continues otherwise goes to else
		movs r2, #2			//puts 2 in r2
		tst r4, r2			//checks if 2s spot is a 1 or 0 if 0 then not equal should run to else 1
		beq else1			//its either this or eq

	//math statements
		lsls r2, r4, #2 	//position
		adds r3, r2, #4		//i+1
		ldr r3, [r0, r3]	//r3 = i+1
		ldr r2, [r0, r2]	//r2 is now i
		subs r3, r2			//math r3- r2 saved in r3
		ldr r1, [r5]		//load current value
		adds r1, r3			//add value sum and new value
		str r1, [r5]		//store new summed value at value


		b increment1

else1:
		lsls r2, r4, #2		//position in source
		ldr r3, [r0, r2]	//load into r3
		ldr r1, [r5]		//loads current summed value
		adds r1, r3			//math
		str r1, [r5]


		b increment1

increment1:
		adds r4, #1
		b for1

endfor1:
		pop {r4,r5}
		bx lr


.global charsub
charsub:
	//code for charsub function here
		ldr r0, =str
		movs r1, #0			//intialize i


for2:	//as long as str[x] isnt null terminator x++
		ldrb r2, [r0, r1]  	//loads str[x]
		cmp r2, 0x0			//is str[x] null or not
		beq endfor2			//if it is end this loop

if2:		//if 'A' <= str[x] <= 'Z'
		cmp r2, 0x41
		blt increment2
		cmp r2, 0x5a
		bgt increment2

		//stuff and things
		movs r2, 0x3d
		strb r2, [r0, r1]
		b increment2

increment2:
		adds r1, #1
		b for2


endfor2:
		bx lr



.global login
login: .string "kolb3"
.align 2
.global main
main:
		bl autotest
		bl intsub
		bl charsub
		bkpt
