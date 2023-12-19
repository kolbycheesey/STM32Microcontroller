.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global main
main:
	bl autotest
	bkpt
	;movs r0, #0
	;movs r1, #0
;loop:
	;adds r0, #1
	;subs r1, #1
	;b loop
.global login
login:
	.asciz "kolb3"
.align 2
