.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global login
login: .string "kolb3"
hello_str: .string "Hello, %s!\n"
.align 2
.global hello
hello:
	push {lr}
	ldr r0,=hello_str
	ldr r1,=login
	//ldr r0, [r0]
	//ldr r1, [r1]
	bl printf
	pop  {pc}

showmult2_str: .string "%d * %d = %d\n"
.align 2
.global showmult2
showmult2:
	push {lr}
	push {r0-r3}
	movs r3, r1
	muls r3, r0
	movs r2, r1
	movs r1, r0
	ldr r0, =showmult2_str
	bl printf
	pop {r0-r3}
	pop  {pc}

// Add the rest of the subroutines here

showmult3_str: .string "%d * %d * %d =%d\n"
.align 2
.global showmult3
showmult3:
	push {r0-r4,lr}
	//movs r4, r0
	movs r3, r2
	movs r2, r1
	movs r1, r0
	muls r0, r2
	muls r0, r3
	sub sp, #4
	str r0, [sp, #0]
	ldr r0, =showmult3_str
	bl printf
	add sp, #4
	pop {r0-r4,pc}

listing_str: .string "%s %05d %s %d students in %s, %d\n"
.align 2
.global listing
listing:
	push {r0-r5,lr}
	//movs r7, r0
	ldr r5, [sp, #4]
	ldr r4, [sp, #0]
	sub sp, #12
	//movs r0, r5
	str r5, [sp, #8]
	//sub sp, #4
	str r4, [sp, #4]
	//sub sp, #4
	str r3, [sp, #0]
	movs r3, r2			//ldr[r2]
	movs r2, r1
	//ldr r6, [r7]
	movs r1, r0			//ldr[r1]

	ldr r0,=listing_str
	bl printf

	add sp, #12
	pop {r0-r5,pc}

//tmp: .space 400	//100 words
.align 2
.global trivial
trivial:
	push {r1-r5,lr}
	sub sp, #400		//int tmp 100
	mov r5, sp

	movs r1, #4
	movs r2, #0
fortrivial:
	cmp r2, #100
	bge endfortrivial	//make sure r2 is less than 100
	movs r3, r1
	muls r3, r2

	adds r2, r2, #1			//x + 1
	str r2, [r5, r3]		//tmp[x] = x+1

	b fortrivial

endfortrivial:
	cmp r0, #100		//compare n to 100
	blo endiftrivial	//if n less than go to end statement
iftrivial:
	movs r0, #100
	subs r0, r0, #1
endiftrivial:
	lsls r2, r0, #2		//n * 4 for bit location of tmp[n]
	//movs r2, #4
	//muls r2, r0
	ldr r0, [r5, r2] 	//should load tmp[n]

	add sp, #400
	pop {r1-r5,pc}

.align 2
.global reverse_puts
reverse_puts:
	push {r1-r7,lr}
	movs r4, r0			//hold onto s if we need later
	bl strlen

	adds r1, r0, #4 	//strlen(s) + 4  //r0 is len
	movs r3, #3			//r3 = 3
	mvns r3, r3			//bitwise not of 3
	ands r3, r1			//r1 = r1 & ~3 r3 is now nelen

	mov r7, sp			//mov r7 to sp
	subs r7, r3			//sub bit size of r3 from r7
	mov sp, r7			//extra stuff
	mov r7, sp
	movs r1, #0			//double dip make 0 and use as x later
	strb r1, [r7, r0]	//store 0 in buffer[len]

forreverseputs:
	cmp r1, r0
	bcs endforreverseputs
	subs r2, r0, #1		//len - 1
	subs r2, r1			//len - 1 - x
	ldrb r6, [r4, r1]	//load s[x]						//need strb and ldrd
	strb r6, [r7, r2]	//str s[x] at buffer[len - 1 - x]
	adds r1, #1			//x += 1

	b forreverseputs

endforreverseputs:
	movs r0, r7			//movs buffer location to r0
	movs r6, r3
	bl puts				//puts(buffer)

	add sp, r6

	pop {r1-r7,pc}

.align 2
.global sumsq
sumsq:
	push {r1-r7,lr}
	sub sp, #400
	mov r5, sp
ifsumsq1:
	cmp r0, #100	//is a >= 100
	bcc ifsumsq2
	movs r0, #99	//make a 99 if so
ifsumsq2:
	cmp r1, #100	//is b >= 100
	bcc next
	movs r1, #99	//make b 99

next:
	movs r3, #1		//step = 1

ifsumsq3:
	cmp r0, r1		//is a == b
	bne else		//if not move to else
	movs r3, #0		//step = 0
else:
	cmp r0,r1		//a > b?
	bls next2
	movs r3, #0
	subs r3, r3, #1	//step = -1
next2:
	movs r4, #0		//x = 0
forsumsq:
	cmp r4, #100	//is x < 100
	bge inbetween

	push {r3,r4}
	lsls r3, r4, #2	//bit location of x
	muls r4, r4		//x^2
	str r4, [r5,r3]	//store x in r5 at bit location r3
	pop {r3,r4}		//move r3 and r4 back to normal values
	adds r4, #1		//x += 1
	b forsumsq

inbetween:
	movs r6, #0		//sum = 0
	movs r4, r0		//x = a

for2sumsq:
	lsls r7, r4, #2	//bit location of a
	ldr r7, [r5, r7]//ld r7 into r7
	adds r6, r7	//sum += tmp[x]

ifsumsq4:
	cmp r4, r1		//x == b?
	bne next3
	movs r0, r6		//make sum location r0
	add sp, #400
	pop {r1-r7,pc}	//return

next3:
	adds r4, r3		//x+=a
	b for2sumsq

.align 2
.global strlen
strlen:
	push {r1,r2,lr}
	movs r2, #0
loop:
	ldrb r1, [r0,r2]
	cmp r1, #0
	beq exit
	adds r2, #1
	b loop
exit:
	mov r0, r2
	pop {r1,r2,pc}
