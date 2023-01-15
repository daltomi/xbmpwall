### XBmpWall (xbmpwall)

X11 bitmap (.xbm) file manager for `xsetroot`.

It shows a bitmap preview and allows you to place it as a wallpaper.

- Inspired by: `https://github.com/dkeg/bitmap-walls.git`

- Generates a script in `~/.xbmpwall.sh` to place the wallpaper at the beginning of your session with all the information required by `xsetroot`




<img src="https://github.com/daltomi/xbmpwall/raw/master/preview02.png"/>

<img src="https://github.com/daltomi/xbmpwall/raw/master/preview03.png"/>


---

#### Build

* Requirements:
  +  `C11, POSIX.1-2008`
  + `libX11`
  + `libXaw (X11 Athena Widget library)`
  + `autotools` (*)
  + `xsetroot`
  + `sh`

* Process:

  + `autoreconf -fi` (*)

  + debug:
    ```bash
      $ ./configure CFLAGS="-O2 -g -Wall -Wextra" CPPFLAGS="-DDEBUG" && make
    ```
  + release:
    ```bash
      $ ./configure CFLAGS="-O3" CPPFLAGS="-DNDEBUG" && make
    ```


(*) Only if you build from GIT.

_Note: Please, if you want to help find bugs, compile in Debug mode._


#### User manual

Recommended to download the bitmaps collection from: `https://github.com/dkeg/bitmap-walls.git`


- Open `xbmpwall` indicating the file or files of bitmaps (* .xbm), for example:

```bash
$ xbmpwall $(pwd)/arches.xbm $(pwd)/balls.xbm

$ xbmpwall ~/bitmap-walls/{patterns/*.xbm,bw-bgnd/*.xbm}

```

_Note: the path to the file must be absolute_

- Each time you select a bitmap or a color(background or foreground), `xsetroot` is executed to place the wallpaper.

- To change between the color selection:

  - press :keyboard: key `Space` to change selection mode:`foreground color` or `background color` then

  - if **cursor** of mouse is :arrow_up: the selection mode is: `foreground color`

  - if **cursor** of mouse is :arrow_down: the selection mode is: `background color`


When the program finishes, the file `~/.xbmpwall.sh` will be saved automatically with the indications for `xsetroot`.
This script in 'sh' has the executable attribute.


#### X11 resources

For example, you can modify some properties in `~/.Xresources`:

```bash
XBmpWall*font: -*-terminus-bold-*-*-*-12-*-*-*-*-*-*-*
XBmpWall*.background: #ECE9D8
XBmpWall*foreground: red

```


#### FAQ

- What is a `background` color?

    Corresponds to the white color of the XBM file format.

- What is a `foreground` color?

    Corresponds to the black color of the XBM file format.
