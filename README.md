# microsweeper
Trying to fit a minesweeper game into as little space as possible (Linux X11)

# Why?
[This video got my interest](https://www.youtube.com/watch?v=ExwqNreocpg)

# How?
By stripping everything except absolutely necessary

## No, I meant how to actually do it!?
Get tcc with i386 cross-compilation (if you're on x86_64, which you probably should by now).

Install X11-devel or Xlib on your Linux PC.

Run this: ``tcc microsweeper.c -o microsweeper -m32 -L/usr/lib32 -nostdlib -lc -lX11 -DSMOL``

You should get 5340 binary, which can be ``strip``-ped down to 5332.

UPX can further shrink it down to 4412. But that's not enought.

### WHEN IS IT ENOUGH!?

When I get it on a QR code potentially.

Just like on the video by MattKC.

# why is the code so goddamn awful?

because smol code != best code
