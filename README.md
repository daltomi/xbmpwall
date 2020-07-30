### XBmpWall (xbmpwall)
  
X11 bitmap (.xbm) file manager for `xsetroot`.

It shows a bitmap preview and allows you to place it as a wallpaper.

- Inspired by: `https://github.com/dkeg/bitmap-walls.git`

- Generates a script in bash:`~/.xbmpwall.sh` to place the wallpaper at the beginning of your session with all the information required by `xsetroot`




<img src="https://git.disroot.org/daltomi/xbmpwall/raw/branch/master/preview02.png"/>

<img src="https://git.disroot.org/daltomi/xbmpwall/raw/branch/master/preview03.png"/>



---

#### Build

* Requirements:
	+ `libXaw (X11 Athena Widget library)`
	+ `libX11`
	+ `GCC GNU99 Compil.`

* Process:
	+ Debug : `make debug`
	+ Release: `make release`

* External Applications:
	+ `xsetroot`

_Please, if you want to help find bugs, compile in Debug mode._


#### User manual

Recommended to download the bitmaps collection from: `https://github.com/dkeg/bitmap-walls.git`


- Open `xbmpwall` indicating the file or files of bitmaps (* .xbm), for example:

```
./xbmpwall arches.xbm balls.xbm

./xbmpwall ~/bitmap-walls/{patterns/*.xbm,bw-bgnd/*.xbm}

```

- Each time you select a bitmap or a color(background or foreground), `xsetroot` is executed to place the wallpaper.

- To change between the color selection: 

	- press :keyboard: key `Space` to change selection mode:`foreground color` or `background color` then

	- if **cursor** of mouse is :arrow_up: the selection mode is: `foreground color`

	- if **cursor** of mouse is :arrow_down: the selection mode is: `background color`


When the program finishes, the file `~/.xbmpwall.sh` will be saved automatically with the indications for `xsetroot`.
This script in bash has the executable attribute.


#### FAQ

- What is a `background` color?

    Corresponds to the white color of the XBM file format.

- What is a `foreground` color?

    Corresponds to the black color of the XBM file format.
