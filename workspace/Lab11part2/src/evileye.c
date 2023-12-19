
#include "stm32f0xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "lcd.h"
#include <stdio.h> // for sprintf()
#define N 1000
#define RATE 20000
short int wavetable[N];

char display[8];	//7-segment display
char offset;		//for cycling through
char state;		//what state the game is currently in
char start;

//for the music
int volume = 2048;
int stepa = 0;
int offseta = 0;
int note = 0;

//song maybe idk we will see if this works
//ccggaaggffeeddcc


float song[20] = {261.63,261.63,392,392,440,440,392,392,349.23,349.23,329.63,329.63,293.66,293.66,261.63,261.63,0,0,0,0};


//timer 1	//shes at full speed atm
void setup_tim1(){
	//RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	//GPIOA->MODER |= 0xaa0000;
	//GPIOA->AFR[1] |= 0x2000;

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	TIM1->BDTR |= 0x8000;

	TIM1->PSC = 0;
	TIM1->ARR = 2399;
	TIM1->CCMR1 |= 0x6060;
	TIM1->CCMR2 |= 0x6860;
	TIM1->CCER |= 0x1111;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void setup_tim3(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = 47999;
	TIM3->ARR = 499;
	TIM3->DIER |= TIM_DIER_UIE;
	NVIC->ISER[0] |= 1<<TIM3_IRQn;
	TIM3->CR1 |= TIM_CR1_CEN;
}

void setup_tim6()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->CR1 |= TIM_CR1_CEN;
	TIM6->PSC = 479;
	TIM6->ARR = 4;
	TIM6->DIER |= TIM_DIER_UIE;
	NVIC->ISER[0] = 1<<TIM6_DAC_IRQn;
}

//timer 2 just for the srand function
void setup_tim2(){
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 1;
	//TIM2->ARR = 499;
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC->ISER[0] |= 1<<TIM2_IRQn;
	TIM2->CR1 |= TIM_CR1_CEN;
}

void setup_tim7()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
	TIM7->PSC = 4799;
	TIM7->ARR = 9;
	TIM7->DIER |= TIM_DIER_UIE;
	NVIC->ISER[0] = 1<<TIM7_IRQn;
	TIM7->CR1 |= TIM_CR1_CEN;
}

//consider making this faster, not insane but more than one move per second
void setup_tim17()
{
    // Set to invoke the ISR every 2 seconds.
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
	TIM17->CR1 |= TIM_CR1_CEN;
	TIM17->PSC = 47999;
	TIM17->ARR = 999;
	TIM17->DIER |= TIM_DIER_UIE;
	NVIC->ISER[0]=1<<TIM17_IRQn;
}

void setup_portb()
{
    // Enable the RCC clock to Port B.
    // Set PB0 - PB3 as outputs.
    // Set PB4 - PB7 as inputs, with pull down resistors.
    // Set output a '1' on PB2 and PB3.
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= 0xffff0000;
	GPIOB->MODER |= 0x55;
	GPIOB->PUPDR &= 0xffff00ff;
	GPIOB->PUPDR |= 0xaa00;
	GPIOB->BSRR = 0xf0000 | (1<<2) | (1<<3) | 1;
   // Also sets up port C too
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER &= 0xffc00000;
	GPIOC->MODER |= 0x155555;

}

void wait_no_press(void)
{
    // Wait for any of PB4 - PB7 to all be '0'.
    // Then wait another 10 msec.  [Use nano_wait(100000000)]
	if(~((GPIOB->IDR >> 4) & 1) && ~((GPIOB->IDR >> 5)  & 1) && ~((GPIOB->IDR >> 6) & 1) && ~((GPIOB->IDR >> 7)  & 1)){
		nano_wait(10000000);
	}
}

char get_press(void)
{
    // If any of PB4 - PB7 are set,
    // wait another 10 msec for any bouncing to stop.
    // If '*' is pressed, return '*'.
    // If '#' is pressed, return '#'.
    // If '8' is pressed, return '8'.
    // Otherwise, don't return.
	if(((GPIOB->IDR >> 4) & 1) || ((GPIOB->IDR >> 5)  & 1) || ((GPIOB->IDR >> 6) & 1) || ((GPIOB->IDR >> 7)  & 1)){
			nano_wait(10000000);
		}
	for(;;){

		int select = ((GPIOB->IDR >> 4) & 0xf);
		if(select == 1){
			return '*';
		}
		else if(select == 2){
			return'8';
		}
		else if(select == 4){
			return '#';
		}
		else if(select == 8) {//whatever 0 is
			return 'A';
		}

	}

}
//MUSIC stuff
void init_wavetable(void)
{
	for(int i = 0; i < N; i++)
	    wavetable[i] = 32767 * sin(2 * M_PI * i / N);
}

void set_freq_a(float f)
{
	if(f == 0){
		offseta=0;
		stepa=0;
	}
	else{
		stepa = f * N / RATE * (1<<16);
	}
}

void setup_spi1()
{
    // use setup_spi1() from lab 8.
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~0x3fcff0;
	GPIOA->MODER |= 0x808a00;
	GPIOA->MODER |= 0X50;
	GPIOA->AFR[1] |=0x2000;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_BR;
	SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
	SPI1->CR2 = SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_SSOE | SPI_CR2_NSSP;
	SPI1->CR1 |= SPI_CR1_SPE;
}

struct Room {
    uint16_t north;
    uint16_t west;
    uint16_t south;
    uint16_t east;
};

// +--+--+--+--+--+--+--+--+--+--+--+--+--+       N
// | 1  2  3  4  5  6  7  8  9 10 11 12 13|       ^
// |  +--+  +--+  +--+  +--+  +--+  +--+  |   W < + > E
// |14|  |15|  |16|  |17|  |18|  |19|  |20|       v
// |  +--+  +--+  +--+  +--+  +--+  +--+  |       S
// |21 22 23 24 25 26 27 28 29 30 31 32 33|
// |  +--+  +--+  +--+  +--+  +--+  +--+  |
// |34|  |35|  |36|  |37|  |38|  |39|  |40|
// |  +  +  +--+  +--+  +  +  +--+  +--+  |
// |41|  |42 43 44 45 46|  |47 48 49 50 51|
// |  +  +--+--+  +--+  +  +  +--+  +--+  |
// |52|        |53|  |54|  |55|  |56|  |57|
// |--+--+--+--+  +  +  +--+  +--+  +  +  |
// |  |58 59 60 61|  |62 63 64 65 66|  |67|
// |--+--+  +--+--+  +  +--+  +--+--+  +  |
// |68|  |69|        |70|  |71|        |72|
// |  +--+  +--+--+--+--+--+  +--+--+--+--|
// |73 74 75 76 77 78|  |79 80 81 82 84|  |
// |  +--+  +--+  +--+--+--+  +--+--+--+--|
// |85|  |86|  |87|  |88|  |89|        |90|
// |--+  +  +  +  +  +  +--+  +--+--+--+  |
// |     |91|  |92|  |93 94 95 96 97 98 99|
// +--------------------------------------+
// 90 is the win condition
const struct Room rooms[] = {
        [1] = { .east=2, .south=14 },
        [2] = { .west=1, .east=3 },
        [3] = { .west=2, .east=4, .south=15 },
        [4] = { .west=3, .east=5 },
        [5] = { .west=4, .east=6, .south=16 },
        [6] = { .west=5, .east=7 },
        [7] = { .west=6, .east=8, .south=17 },
        [8] = { .west=7, .east=9 },
        [9] = { .west=8, .east=10, .south=18 },
        [10] = { .west=9, .east=11 },
        [11] = { .west=10, .east=12, .south=19 },
        [12] = { .west=11, .east=13 },
        [13] = { .west=12, .south=20 },

        [14] = { .north=1, .south=21 },
        [15] = { .north=3, .south=23 },
        [16] = { .north=5, .south=25 },
        [17] = { .north=7, .south=27 },
        [18] = { .north=9, .south=29 },
        [19] = { .north=11, .south=31 },
        [20] = { .north=13, .south=33 },

        [21] = { .north=14, .east=22, .south=34 },
        [22] = { .west=21, .east=23 },
        [23] = { .west=22, .north=15, .east=24, .south=35 },
        [24] = { .west=23, .east=25 },
        [25] = { .west=24, .north=16, .east=26, .south=36 },
        [26] = { .west=25, .east=27 },
        [27] = { .west=26, .north=17, .east=28, .south=37 },
        [28] = { .west=27, .east=29 },
        [29] = { .west=28, .north=18, .east=30, .south=38 },
        [30] = { .west=29, .east=31 },
        [31] = { .west=30, .north=19, .east=32, .south=39 },
        [32] = { .west=31, .east=33 },
        [33] = { .west=32, .north=20, .south=40 },

	//added maze
	[34] = { .north=21, .south=41 },
	[35] = { .north=23, .south=42 },
	[36] = { .north=25, .south=44 },
	[37] = { .north=27, .south=46 },
	[38] = { .north=29, .south=47 },
	[39] = { .north=31, .south=49 },
	[40] = { .north=33, .south=51 },

	[41] = { .north=34, .south=52 },
	[42] = { .north=35, .east=43 },
	[43] = { .west=42, .east=44 },
	[44] = { .west=43, .north=36, .east=45, .south=53 },
	[45] = { .west=44, .east=46 },
	[46] = { .north=37, .south=54 },
        [47] = { .north=38, .east=48, .south=55 },
        [48] = { .east=49, .west=48 },
        [49] = { .west=48, .north=39, .east=50, .south=56 },
        [50] = { .west=49, .east=51 },
        [51] = { .north=40, .east=50, .south=57 },

	[52] = { .north=41 },
        [53] = { .north=44, .south=61 },
        [54] = { .north=46, .south=62 },
        [55] = { .north=47, .south=64 },
        [56] = { .north=49, .south=66 },
        [57] = { .north=51, .south=67 },

	[58] = { .east=59 },
        [59] = { .west=58, .east=60, .south=69 },
        [60] = { .west=59, .east=61 },
        [61] = { .west=60, .north=53 },
        [62] = { .north=54, .east=63, .south=70 },
        [63] = { .west=62, .east=64 },
        [64] = { .west=63, .north=55, .east=65, .south=71 },
        [65] = { .west=64, .east=66 },
        [66] = { .west=64, .north=56 },
	[67] = { .south=57, .south=72 },

	[68] = { .south=73 },
        [69] = { .north=59, .south=75 },
        [70] = { .north=62 },
        [71] = { .north=64, .south=80 },
        [72] = { .north=67 },

	[73] = { .north=68, .east=74, .south=85 },
        [74] = { .west=73, .east=75 },
        [75] = { .west=74, .north=69, .east=74, .south=86 },
        [76] = { .west=75, .east=77 },
        [77] = { .west=76, .east=78, .south=87 },
        [78] = { .west=77 },
        [79] = { .south=80 },
        [80] = { .west=79, .north=71, .east=81, .south=89 },
        [81] = { .west=80, .west=82 },
        [82] = { .west=81, .east=84 },
        //[83] = { .north=9, .south=29 },
        [84] = { .west=82 },

	[85] = { .north=73 },
        [86] = { .north=75, .south=91 },
        [87] = { .north=77, .south=92 },
        [88] = { .south=93 },
        [89] = { .north=80, .south=95 },
        [90] = { .south=99 },

	[91] = { .north=91 },
        [92] = { .north=87 },
        [93] = { .north=93, .east=94 },
        [94] = { .west=93, .east=95 },
        [95] = { .west=94, .north=89, .east=96 },
        [96] = { .west=95, .east=97 },
        [97] = { .west=96, .east=98 },
        [98] = { .west=97, .north=99 },
	[99] = { .west=98, .north=90 },
};

//grabbed this from lab 7 need to add some items on
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
	['e'] = 0x79,
	['L'] = 0x38,
	['U'] = 0x3e,
	['p'] = 0x73,
	['a'] = 0x77,
	['!'] = 0x86,
};


typedef enum { NORTH, WEST, SOUTH, EAST } dir_t;

uint16_t me_loc;
dir_t me_dir;
uint16_t eye_loc;
dir_t eye_dir;

dir_t left_of(dir_t dir)
{
    switch(dir) {
    case NORTH: return WEST;
    case WEST:  return SOUTH;
    case SOUTH: return EAST;
    case EAST:  return NORTH;
    default:    return NORTH;
    }
}

dir_t right_of(dir_t dir)
{
    switch(dir) {
    case NORTH: return EAST;
    case WEST:  return NORTH;
    case SOUTH: return WEST;
    case EAST:  return SOUTH;
    default: return NORTH;
    }
}

dir_t opposite_of(dir_t dir)
{
    switch(dir) {
    case NORTH: return SOUTH;
    case WEST:  return EAST;
    case SOUTH: return NORTH;
    case EAST:  return WEST;
    default: return NORTH;
    }

}

uint16_t move_from(uint16_t loc, dir_t dir)
{
    if (loc == 0)
        return 0;
    if (loc >= sizeof rooms / sizeof rooms[0])
        return 0;
    struct Room r = rooms[loc];
    switch(dir) {
    case NORTH: return r.north;
    case WEST:  return r.west;
    case SOUTH: return r.south;
    case EAST:  return r.east;
    default: return 0;
    }
}

const char *dir_name(dir_t dir)
{
    switch(dir) {
    case NORTH: return "north";
    case WEST:  return "west";
    case SOUTH: return "south";
    case EAST:  return "east";
    default:    return "?????";
    }
}

void update_info(void)
{
    char line[30];
    sprintf(line, "me loc:  %2d", me_loc);
    LCD_DrawString(60,200,  WHITE, BLACK, line, 16, 0);
    sprintf(line, "me dir:  %-5s", dir_name(me_dir));
    LCD_DrawString(60,220,  WHITE, BLACK, line, 16, 0);
    sprintf(line, "eye loc: %2d", eye_loc);
    LCD_DrawString(60,240,  WHITE, BLACK, line, 16, 0);
    sprintf(line, "eye dir: %-5s", dir_name(eye_dir));
    LCD_DrawString(60,260,  WHITE, BLACK, line, 16, 0);
}

// Draw the components of the view at a distance.
// This subroutine recursively draws the things at a further
// distance in the center.
void view(int loc, dir_t dir, int dist)
{
    struct Room r = rooms[loc];
    int left=0;
    int right=0;
    int straight=0;
    int win_loc = 90;

    if(start != 'y') {
	    LCD_Clear(BLACK);
	    LCD_DrawString(50,80, GREEN, BLACK, "Press A to start", 16, 0);
	    //add the output of You Lose to the led 7 seg
	    state = 's';
	    //me_loc = 0;
	    //eye_loc = 0;
	    return;
    }
    //might need this
    /*
     * else{
     * 		state = '0';
     * 	}
     */

    //Lose Condition
    if (dist == 0 && loc == eye_loc && start == 'y') {
        LCD_Clear(BLACK);
        LCD_DrawString(50,80, GREEN, BLACK, "Captured by the eye!", 16, 0);
        //add the output of You Lose to the led 7 seg
        state = 'l';
        me_loc = 0;
        eye_loc = 0;
        return;
    }
    //Win Condition
    else if(loc == win_loc){
	LCD_Clear(BLACK);
	LCD_DrawString(50,80,GREEN,BLACK, "You have escaped!",16,0);
	//Output You Win to the led 7 seg
	state = 'w';
	me_loc = 0;
	eye_loc = 0;	//this might need to be different so it doesnt activate both win and lose
	return;
    }


    switch(dir) {
    case NORTH:
        left = r.west;
        right = r.east;
        straight = r.north;
        break;
    case WEST:
        left = r.south;
        right = r.north;
        straight = r.west;
        break;
    case SOUTH:
        left = r.east;
        right = r.west;
        straight = r.south;
        break;
    case EAST:
        left = r.north;
        right = r.south;
        straight = r.east;
        break;
    }

    // Compute outer and inner rectangles with perspective.
    float depth = 1.0; // Make this larger to make things look longer.
    struct xy { uint16_t x; uint16_t y; };
    struct xy otl = { 0,0 };
    struct xy obl = { 0,160 };
    struct xy otr = { 240,0 };
    struct xy obr = { 240,160 };

    struct xy center = { 120,80 };

    struct xy itl,ibl,itr,ibr;
    float distpow;
    distpow = depth * pow(2.0,dist+0.5);
    itl.x = center.x - abs(otl.x-center.x)/distpow;
    itl.y = center.y - abs(otl.y-center.y)/distpow;
    ibl.x = center.x - abs(obl.x-center.x)/distpow;
    ibl.y = center.y + abs(obl.y-center.y)/distpow;
    itr.x = center.x + abs(otr.x-center.x)/distpow;
    itr.y = center.y - abs(otr.y-center.y)/distpow;
    ibr.x = center.x + abs(obr.x-center.x)/distpow;
    ibr.y = center.y + abs(obr.y-center.y)/distpow;

    if (dist != 0) {
        distpow = depth * pow(2.0,dist-0.5);
        otl.x = center.x - abs(otl.x-center.x)/distpow;
        otl.y = center.y - abs(otl.y-center.y)/distpow;
        obl.x = center.x - abs(obl.x-center.x)/distpow;
        obl.y = center.y + abs(obl.y-center.y)/distpow;
        otr.x = center.x + abs(otr.x-center.x)/distpow;
        otr.y = center.y - abs(otr.y-center.y)/distpow;
        obr.x = center.x + abs(obr.x-center.x)/distpow;
        obr.y = center.y + abs(obr.y-center.y)/distpow;
    }

    // This is the top-level.  Draw a gray box and update the information.
    if (dist==0) {
        LCD_DrawFillRectangle(otl.x,otl.y,obr.x,obr.y, GRAY);
        update_info();
    }

    // Draw a view like this:      Or maybe this:
    //
    // \                    /
    //  \                  /
    //   \________________/       ____ ________________ ____
    //   |                |           |                |
    //   |                |           |                |
    //   |                |           |                |
    //   |                |           |                |
    //   |________________|       ____|________________|____
    //  /                  \      (straight          (straight
    // /                    \       across)            across)

    // Always draw the two left and right wall segment markers.
    LCD_DrawLine(itl.x,itl.y,ibl.x,ibl.y, BLACK);
    LCD_DrawLine(itr.x,itr.y,ibr.x,ibr.y, BLACK);
    if (straight==0) {
        // Straight ahead is blocked.  Draw lines on floor/ceiling.
        LCD_DrawLine(itl.x,itl.y,itr.x,itr.y, BLACK);
        LCD_DrawLine(ibl.x,ibl.y,ibr.x,ibr.y, BLACK);
    }
    if (left) {
        // Open to left.  Draw straight lines at floor and ceiling.
        LCD_DrawLine(otl.x, itl.y, itl.x,itl.y, BLACK);
        LCD_DrawLine(obl.x, ibl.y, ibl.x,ibl.y, BLACK);
    } else {
        // Closed to left.  Draw left wall.
        LCD_DrawLine(otl.x, otl.y, itl.x,itl.y, BLACK);
        LCD_DrawLine(obl.x, obl.y, ibl.x,ibl.y, BLACK);
    }
    if (right) {
        // Open to right.  Draw straight lines at floor and ceiling.
        LCD_DrawLine(itr.x, itr.y, otr.x,itl.y, BLACK);
        LCD_DrawLine(ibr.x, ibr.y, obr.x,ibl.y, BLACK);
    } else {
        // Closed to right.  Draw right wall.
        LCD_DrawLine(itr.x, itr.y, otr.x,otl.y, BLACK);
        LCD_DrawLine(ibr.x, ibr.y, obr.x,obl.y, BLACK);
    }

    // If straight ahead is open, recurse to draw the further distance.
    if (straight)
        view(straight, dir, dist+1);

    if (eye_loc == loc) {
        // A square straight ahead in view is the location of the eye,
        // so the eye is visible.  Draw it.
        int radius = 100 / (depth * pow(2.0, dist));
        if (radius >= 4) {
            // Draw the white circle and a black border.
            LCD_Circle(center.x, center.y, radius, 1, WHITE);
            LCD_Circle(center.x, center.y, radius, 0, BLACK);
            if (eye_dir == opposite_of(me_dir)) {
                // The eye is looking right at me.
                LCD_Circle(center.x, center.y, radius/2, 1, GREEN);
                LCD_Circle(center.x, center.y, radius/4, 1, BLACK);
            } else if (eye_dir == left_of(me_dir)) {
                // Draw the iris and pupil on the left.
                for(int i=radius*3/4; i<radius; i++) {
                    // so difficult.
                    int dist = sqrt(pow(radius,2) - pow(i,2));
                    if (i < radius * 7/8) {
                        LCD_DrawLine(center.x-i, center.y-dist,
                                     center.x-i, center.y+dist, GREEN);
                    } else {
                        LCD_DrawLine(center.x-i, center.y-dist,
                                     center.x-i, center.y+dist, BLACK);
                    }
                }
            } else if (eye_dir == right_of(me_dir)) {
                // Draw the iris and pupil on the right.
                for(int i=radius*3/4; i<radius; i++) {
                    // so difficult.
                    int dist = sqrt(pow(radius,2) - pow(i,2));
                    if (i < radius * 7/8) {
                        LCD_DrawLine(center.x+i, center.y-dist,
                                     center.x+i, center.y+dist, GREEN);
                    } else {
                        LCD_DrawLine(center.x+i, center.y-dist,
                                     center.x+i, center.y+dist, BLACK);
                    }
                }
            }
        }
    }
}

void TIM17_IRQHandler(void)
{
    TIM17->SR &= ~TIM_SR_UIF;
    if (eye_loc == 0)
        return;
    if (move_from(eye_loc, eye_dir))
        eye_loc = move_from(eye_loc, eye_dir);
    else {
        // Didn't move.  Change direction.
        if (random() % 2)
            eye_dir = left_of(eye_dir);
        else
            eye_dir = right_of(eye_dir);
    }
    view(me_loc, me_dir, 0);
}

//change so that everything is setup correctly
void show_message(){
	int off = offset & 7;
	GPIOC->ODR = (off << 8) | display[off];
}

/*void TIM1_CC_IRQHandler(void){
	TIM1->SR &= ~TIM_SR_UIF;
}*/

void TIM2_IRQHandler(void){
	TIM2->SR &= ~TIM_SR_UIF;
}

void TIM3_IRQHandler(void){
	TIM3->SR &= ~TIM_SR_UIF;
	set_freq_a(song[note]);
	if(note >= 19){
		note = 0;
	}
	else {
		note++;
	}
}

void TIM6_DAC_IRQHandler(void){
	int sample = 0;
	TIM6->SR &= ~TIM_SR_UIF;
	set_freq_a(song[note]);
	offseta += stepa;

	if(offseta >= N<<16)
	   offseta -= N<<16;

	sample += wavetable [offseta >> 16];

	sample = ((sample * volume)>>17) + 1200;

	if(sample > 4095){
		sample = 4095; }

	if(sample < 0){
	    sample = 0;}

	TIM1->CCR4 = sample;
}

void TIM7_IRQHandler(void){
	TIM7->SR &= ~TIM_SR_UIF;
	//GPIOC->ODR //this will be where i setup everything for each of the LEDs
	if(state == 'l'){	//lose
		display[0] = font['4'];
		display[1] = font['0'];
		display[2] = font['U'];
		display[3] = font[' '];
		display[4] = font['L'];
		display[5] = font['0'];
		display[6] = font['5'];
		display[7] = font['e'];
	}
	else if(state == 's'){
		display[0] = font['p'];
		display[1] = font['r'];
		display[2] = font['e'];
		display[3] = font['5'];
		display[4] = font['5'];
		display[5] = font[' '];
		display[6] = font['a'];
		display[7] = font[' '];
	}
	else if(state == 'w'){	//win //using double u for w
		display[0] = font['4'];
		display[1] = font['0'];
		display[2] = font['U'];
		display[3] = font[' '];
		display[4] = font['U'];
		display[5] = font['U'];
		display[6] = font['0'];
		display[7] = font['n'];
	}
	else{	//running state
		display[0] = font['5'];
		display[1] = font['U'];
		display[2] = font['C'];
		display[3] = font['C'];
		display[4] = font['e'];
		display[5] = font['5'];
		display[6] = font['5'];
		display[7] = font['!'];
	}
	show_message();
	offset = (offset + 1) & 0x7;
}

int main(void)
{
    setup_portb();
    setup_spi1();
    LCD_Init();
    LCD_Clear(BLACK);

    me_dir = EAST;
    me_loc = 1;
    eye_dir = WEST;
    eye_loc = 13;

    //setup for time 2
    /*
     * Setup display to tell the player to select a button to set number in this time so it can be imported to tim 17
     * need to figure out how to set up the screen for that if i should do it within tim 2 or if i should do it
     * somewhere else, also use the 1kHz that we used in lab 5 seems to work well and I just need win and lose
     *
     */
    setup_tim2();	//for rand function;
    //here is to hoping the sound works i can get an output on waveforms but cant find a speaker to wire up or my 3.5mm
    setup_tim3();

    /*
     * This is the stuff for the music
     */
    init_wavetable();
    set_freq_a(0);
    setup_tim1();
    setup_tim6();

    //setup for timer 17
    setup_tim17();

    //setup timers for music try to get meglovania in or some other epic boss music
    /*Timer got setup i have output on ad2 but cant test if i have sound or not
     * Look at lab 7 PWM setup for help on this
     */
    setup_tim7();	//7-seg LED function
    /*
     *
     * If you are testing this and need to make it to the end from start go south 2 east 2 south 2 east 4 south 2 east 2 south 4 east 4 north 1
     * S 2
     * E 2
     * S 2
     * E 4
     * S 2
     * E 2
     * S 4
     * E 4
     * N 1
     *
     */


    for(; me_loc != 0;) {
        // Turn off the update interrupt so we do not have two things
        // calling view() at the same time.

	while(start != 'y') {
		char intial = get_press();
		if(intial == 'A'){
			start = 'y';
			//randy = TIM2->CNT;
			srand(TIM2->CNT);	//this should change up the game so it is different every time
		}

	}
	state = '0';

        NVIC->ICER[0] = 1<<TIM17_IRQn; // disable interrupt
        view(me_loc, me_dir, 0);
        NVIC->ISER[0] = 1<<TIM17_IRQn; // re-enable interrupt
        wait_no_press();
        char button = get_press();
        if (button == '*') {
            // Turn left.
            me_dir = left_of(me_dir);
        } else if (button == '#') {
            // Turn right.
            me_dir = right_of(me_dir);
        } else if (button == '8') {
            // Move forward.
            if (move_from(me_loc, me_dir))
                me_loc = move_from(me_loc, me_dir);
        }
    }
}
