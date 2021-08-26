#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/random.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// BUILD CONFIGS
#ifdef BUILD_FULL
	#define BUILD_QR
#endif
#ifdef BUILD_QR
	#define BUILD_2K
#endif
#ifdef BUILD_2K
	#define BUILD_MINIMAL
#endif

static Window win;
static Display *dpy;
static XEvent event;
static GC gc;

static unsigned char *field;
static unsigned short i, j, k, l;
static unsigned char a, b, c;

#define FIELD_SET  0b10000000
#define FLAG_SET   0b01000000
#define MINE_SET   0b00100000
#define NUMBER_SET 0b00001111

#ifdef BUILD_QR
	unsigned char TILE = 16;
	unsigned short WSX = 256, WSY = 256;
	Atom wm_delete_window;
	unsigned short constX, constY;
	#define XCOORD i*TILE + constX
	#define YCOORD j*TILE + constY
	#define WS WSX
#else
	// DO NOT CHANGE!!
	#define TILE 16
	#define WS NUM_TILES*TILE
	#define XCOORD a
	#define YCOORD b
#endif

#define MINES     25
#define AREA      256
#define NUM_TILES 16

#ifdef BUILD_QR
	#define BLUE      colors[0]
	#define GREEN     colors[1]
	#define RED       colors[2]
	#define DARK_BLUE colors[3]
	#define DARK_RED  colors[4]
	#define CYAN      colors[5]
	#define BLACK     colors[6]
	#define GRAY      colors[7]
	
	#define WHITE     colors[8]
	#define BG_COL    colors[9]
	
	unsigned int colors[] = {
		0xff0100fe,
		0xff017f01,
		0xfffe0000,
		0xff010080,
		0xff810102,
		0xff008081,
		0xff000000,
		0xff808080,
		0xffffffff,
		0xffc0c0c0
	};
#else
	#define BG_COL 0x00000000
	#define WHITE  0xffffffff
	#define RED    0xffff0000
	#define GRAY   0x80808080
#endif

#define GAME_END  0xff

// bit structure of the field

// 1st bit - open/closed
// 2nd bit - flagged
// 3rd bit - mine
// 5-8 bit - mine count (1-8)

#ifdef BUILD_FULL
void dump_field() {
	for(j = 0; j < 16; j++) {
		for(i = 0; i < 16; i++) {
			if(field[i<<4|j]&MINE_SET) putchar('m');
			else {
				int lol = field[i<<4|j] & NUMBER_SET;
				if(lol) printf("%d", lol);
				else putchar(' ');
			}
		}
		putchar('\n');
	}
}
#endif

char isAvail(char x, char y) {
	if((x|y)&0xF0) return 0;

	// if current tile already activated, FAIL
	if(field[(x<<4)|y]&FIELD_SET) return 0;

	if((x|y)&0xF0) return 0;
	
	a = field[((x-1)<<4)|y];
	if((a&FIELD_SET)&&!(a&NUMBER_SET)) return 1;

	a = field[((x+1)<<4)|y];
	if((a&FIELD_SET)&&!(a&NUMBER_SET)) return 1;

	a = field[(x<<4)|(y-1)];
	if((a&FIELD_SET)&&!(a&NUMBER_SET)) return 1;

	a = field[(x<<4)|(y+1)];
	if((a&FIELD_SET)&&!(a&NUMBER_SET)) return 1;

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
	#ifdef BUILD_FULL
		dump_field();
	#endif
}

void main() {
	// don't bother error checking, wasting bytes
	dpy = XOpenDisplay(NULL);
	win = DefaultRootWindow(dpy); // recycle kids :)
	win = XCreateSimpleWindow(dpy, win, 0, 0, WS, WS, 0, 0, 0);
	gc =  XCreateGC(dpy, win, 0, 0);

	field = calloc(AREA, 1);

	// assign window to the display - practically show it
	XMapWindow(dpy, win);
	// KeyPressMask
	XSelectInput(dpy, win, StructureNotifyMask|ExposureMask|ButtonPressMask);

	#ifdef BUILD_QR
		wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
		XSetWMProtocols(dpy, win, &wm_delete_window, 1);
	#endif

reset_game:
#ifdef BUILD_2K
	c = 0;
	XStoreName(dpy, win, "Microsweeper");
#endif
	generate_field();

	while(1) {
		XNextEvent(dpy, &event);
		switch(event.type) {
			case Expose: // redraw window
				XSetForeground(dpy, gc, BG_COL);
				#ifdef BUILD_QR
					XFillRectangle(dpy, win, gc, 0, 0, WSX, WSY);
				#else
					XFillRectangle(dpy, win, gc, 0, 0, WS, WS);
				#endif
				for(i = 0; i < NUM_TILES; i++) {
					for(j = 0; j < NUM_TILES; j++) {
						a = i<<4; b = j<<4;
						char abc[2] = { 0, 0 };
						XSetForeground(dpy, gc, WHITE);
						if(!(field[a|j]>>7))
							XFillRectangle(dpy, win, gc, XCOORD, YCOORD, TILE, TILE);
						if(field[a|j]&FLAG_SET) {
							abc[0] = 'P';
							#ifdef BUILD_2K
								if(!(field[a|j]&FIELD_SET)) XSetForeground(dpy, gc, RED);
							#else
								XSetForeground(dpy, gc, GRAY);
							#endif
						} else {
							l = field[a|j]&NUMBER_SET;
							if(l) {
								abc[0] = '0' + l;
								#ifdef BUILD_QR
									if(field[a|j]&FIELD_SET) XSetForeground(dpy, gc, colors[l-1]);
								#endif
							}
						}
						if(abc[0]) XDrawString(dpy, win, gc, XCOORD + 5, YCOORD + 14, abc, 1);

						#ifdef BUILD_2K
							XSetForeground(dpy, gc, GRAY);
							if((field[a|j]&MINE_SET) && (field[a|j]&FIELD_SET)) {
								#ifdef BUILD_QR
									XSetForeground(dpy, gc, RED);
								#endif
								XFillRectangle(dpy, win, gc, XCOORD, YCOORD, TILE, TILE);
							}
							XDrawRectangle(dpy, win, gc, XCOORD, YCOORD, TILE, TILE);
						#endif
					}
				}
				break;
			case ButtonPress:
				#ifdef BUILD_QR
					i = (((event.xbutton.x-constX)/TILE) << 4) | ((event.xbutton.y-constY)/TILE);
				#else
					// assembly optimizations :) 
					i = (event.xbutton.x&0xf0)|(event.xbutton.y>>4);
				#endif

				event.type = Expose;
				XSendEvent(dpy, win, 0, 0, &event);
				switch(event.xbutton.button) {
					case 1: // left click
						#ifdef BUILD_2K
						if(c) {
							//c=0;
							for(i=0; i<256; i++) field[i] = 0;
							goto reset_game;
						}
						#endif
						if(field[i]&MINE_SET) {
							#ifdef BUILD_QR
								for(i=0; i<256; i++)
									if(field[i]&MINE_SET) field[i] |= FIELD_SET;
							#endif
							#ifdef BUILD_2K
								XStoreName(dpy, win, "You lost. L/R click - restart/quit.");
								c = GAME_END;
								field[i] |= FIELD_SET;
								break;
							#else
								return; // you ded
							#endif
						}
						field[i] &= ~FLAG_SET; // unset right click
						#ifdef BUILD_QR
							floodFill((event.xbutton.x-constX)/TILE, (event.xbutton.y-constY)/TILE, 1);
						#else
							floodFill(event.xbutton.x>>4, event.xbutton.y>>4, 1);
						#endif
						break;
					case 3: // right click
						#ifdef BUILD_2K
						if(c) return;
						#endif
						if(!(field[i]&FIELD_SET))
							field[i]^=FLAG_SET;
						break;
				}

				j=AREA;
				for(i=0; i<255; i++) {
					if(field[i]&FIELD_SET) j--;
				}
				if(j == MINES) { 
					#ifdef BUILD_2K
						XStoreName(dpy, win, "You won. L/R click - restart/quit.");
						c = GAME_END;
					#else
						puts("U won");
						return;
					#endif
				}
				break;
			#ifdef BUILD_QR
				case ClientMessage:
					// user has quit, halt execution
					if((Atom)event.xclient.data.l[0] == wm_delete_window) return;
					break;
				case ConfigureNotify:
					WSX = event.xconfigure.width;
					WSY = event.xconfigure.height;
					if(WSX<256) {
						WSX = 256;
						XResizeWindow(dpy, win, WSX, WSY);
					}
					if(WSY<256) {
						WSY = 256;
						XResizeWindow(dpy, win, WSX, WSY);
					}
					i = WSX>WSY ? WSY : WSX;
					TILE = i/NUM_TILES;
					constX = (WSX-TILE*NUM_TILES)/2;
					constY = (WSY-TILE*NUM_TILES)/2;
					break;
			#endif
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

