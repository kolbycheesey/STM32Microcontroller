.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global main
main:
	bl serial_init
	ldr r0,=greeting
	ldr r1,=login
	//ldr r0, [r0]
	//ldr r1, [r1]
	bl printf
	wfi
greeting:
	.string "Hello, %s.\n"
	.align 2 // Align anything after this
login:
	.string "kolb3"
	.align 2

