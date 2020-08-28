# microsweeper
Trying to fit a minesweeper game into as little space as possible (Linux X11)

# Why?
[This video got my interest](https://www.youtube.com/watch?v=ExwqNreocpg)

# How?
By stripping everything except absolutely necessary

## No, I meant how to actually do it!?
Get tcc with i386 cross-compilation (if you're on x86_64, which you probably should by now).

Install xz, GCC, Make and Xlib on your Linux PC.

Get and compile: https://www.pouet.net/prod.php?which=85558

Then copy the ``./vondehi`` executable into this directory.

There are multiple flavours to choose from:
```
minimal - smallest one at 1900 bytes
2k - 2048 bytes
full - all features available
```

Once you've picked your favourite, run: ``make TARGET=$one_of_the_above``

And you should get your ``microsweeper`` executable that you can use by now.

## Quick disclaimer
This as mentioned above, uses vondehi which relies on the ``xzcat`` decompressor being on the system.
This means that it isn't fully "self-reliant". But this shouldn't be a issue for most since it's part
of the binutils, which you can find on every Linux distro pretty much.

# why is the code so goddamn awful?

because smol code != best code
