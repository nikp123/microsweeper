#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/random.h>
#include <X11/Xlib.h>

// DO NOT CHANGE!!
#define NUM_TILES 16
#define TILE 16
#define AREA NUM_TILES*NUM_TILES
#define WS NUM_TILES*TILE
#define MINES 25

static Window win;
static Display *dpy;
static XEvent event;
static GC gc;

static unsigned char *field;
static unsigned short i, j, k, l;
static unsigned char a, b;

#define FIELD_SET  0b10000000
#define FLAG_SET   0b01000000
#define MINE_SET   0b00100000
#define NUMBER_SET 0b00001111

#define WHITE     0xffffffff
#define GRAY      0x80808080
#define BLACK     0x00000000

// bit structure of the field

// 1st bit - open/closed
// 2nd bit - flagged
// 3rd bit - mine
// 4rd bit - has mine near it
// 5-8 bit - mine count (1-8)

//void dump_field() {
//	for(j = 0; j < 16; j++) {
//		for(i = 0; i < 16; i++) {
//			if(field[i<<4|j]&MINE_SET) putchar('m');
//			else {
//				int lol = field[i<<4|j] & NUMBER_SET;
//				if(lol) printf("%d", lol);
//				else putchar(' ');
//			}
//		}
//		putchar('\n');
//	}
//}

char isAvail(char x, char y) {
	if(x<0 || x>15 || y<0 || y>15) return 0;

	// if current tile already activated, FAIL
	if(field[(x<<4)|y]&FIELD_SET) return 0;

	if(x>0) {
		a = field[((x-1)<<4)|y];
		if((a&FIELD_SET)&&!(a&0xf)) return 1;
	}
	if(x<15) {
		a = field[((x+1)<<4)|y];
		if((a&FIELD_SET)&&!(a&0xf)) return 1;
	}

	if(y>0) {
		a = field[(x<<4)|(y-1)];
		if((a&FIELD_SET)&&!(a&0xf)) return 1;
	}
	if(y<15) {
		a = field[(x<<4)|(y+1)];
		if((a&FIELD_SET)&&!(a&0xf)) return 1;
	}

	return 0;
}

void floodFill(char x, char y, char skip) {
	if(isAvail(x, y)||skip) {
		field[(x<<4)|y] |= FIELD_SET;
		floodFill(x-1, y, 0);
		floodFill(x+1, y, 0);
		floodFill(x, y-1, 0);
		floodFill(x, y+1, 0);
	}
}

void generate_field() {
	// neophodno, jer onda rand() daje iste rezultate

	// mine generator
	for(i = 0; i < MINES; i++) {
		retry_mine:
		getrandom(&a, 1, 0);
		if(field[a]&MINE_SET) goto retry_mine;
		field[a] = MINE_SET;
	}

	for(i = 0; i < 16; i++) {
		for(j = 0; j < 16; j++) {
			k = i<<4|j;
			if(field[k]&MINE_SET) continue;

			l = 0;
			k-=17;

			a = j<15;
			b = i<15;

			if((field[k] & MINE_SET) && j && i) l++;
			k++;
			if((field[k] & MINE_SET) && i     ) l++;
			k++;
			if((field[k] & MINE_SET) && i && a) l++;
			k+=14;
			if((field[k] & MINE_SET) && j     ) l++;
			k++;
			k++;
			if((field[k] & MINE_SET) && a     ) l++;
			k+=14;
			if((field[k] & MINE_SET) && b && j) l++;
			k++;
			if((field[k] & MINE_SET) && b     ) l++;
			k++;
			if((field[k] & MINE_SET) && b && a) l++;
			k-=17;
			field[k] |= l;
		}
	}
	//dump_field();
}

void main() {
	// don't bother error checking, wasting bytes
	dpy = XOpenDisplay(NULL);
	win = DefaultRootWindow(dpy); // recycle kids :)
	win = XCreateSimpleWindow(dpy, win, 0, 0, WS, WS, 0, 0, 0);
	gc =  XCreateGC(dpy, win, 0, 0);

	field = calloc(AREA, 1);

	generate_field();

	// assign window to the display - practically show it
	XMapWindow(dpy, win);
	// KeyPressMask
	XSelectInput(dpy, win, ExposureMask|ButtonPressMask);

	while(1) {
		XNextEvent(dpy, &event);
		switch(event.type) {
			case Expose: // redraw window
				XSetForeground(dpy, gc, BLACK);
				XFillRectangle(dpy, win, gc, 0, 0, WS, WS);
				for(i = 0; i < TILE; i++) {
					for(j = 0; j < TILE; j++) {
						a = i<<4; b = j<<4;
						char abc[2] = { 0, 0 };
						XSetForeground(dpy, gc, WHITE);
						if(!(field[a|j]>>7)) // FIELD_SET is magic for open
							XFillRectangle(dpy, win, gc, a, b, 16, 16);
						if(field[a|j]&FLAG_SET) {
							abc[0] = 'P';
							XSetForeground(dpy, gc, GRAY);
						} else {
							l = field[a|j]&NUMBER_SET;
							if(l) {
								abc[0] = '0' + l;
							}
						}
						if(abc[0]) XDrawString(dpy, win, gc, a + 5, b + 14, abc, 1);

						//XSetForeground(dpy, gc, GRAY);
						//XDrawRectangle(dpy, win, gc, a, b, 16, 16);
					}
				}
				break;
			case ButtonPress:
				// assembly optimizations :) 
				i = (event.xbutton.x&0xf0)|(event.xbutton.y>>4);

				switch(event.xbutton.button) {
					case 1: // left click
						if(field[i]&MINE_SET) return; // you ded
						field[i] &= ~FLAG_SET; // unset right click
						floodFill(event.xbutton.x>>4, event.xbutton.y>>4, 1);
						break;
					case 3: // right click
						if(!(field[i]&FIELD_SET))
							field[i]^=FLAG_SET;
						break;
				}

				j=AREA;
				for(i=0; i<255; i++) {
					if(field[i]&FIELD_SET) j--;
				}
				if(j == MINES) { 
					puts("u win");
					return;
				}

				event.type = Expose;
				XSendEvent(dpy, win, 0, 0, &event);
				break;
		}
	}
}

// start function that executes first
// MUST NOT REACH END, instead call _Exit(ret_val)
#ifdef SMOL
void _start() {
	main();
	_Exit(0);
}
#endif

