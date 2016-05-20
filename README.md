# X3D-68k
A 3D portal-rendering game engine for graphing calculators! Originally written just for the TI-68k line of graphing calculators, it is now being developed as a cross-platform library. Checkout the documentation on [the wiki](https://github.com/catastropher/X3D-68k/wiki) for information about the project!

To build and run, you will need cmake and libsdl2-dev

```
git clone https://github.com/catastropher/X3D-68k.git
cd X3D-68k
export X3D=$(pwd)
make config
make test-manual
```



## OS X 10.11.5

To build and run X3D on OS X, SDL (Not SDL2) must be installed from source
	https://www.libsdl.org/download-1.2.php

CD into the SDL source directory.

You will need to edit the file ${sdl_srcdir}/src/video/quartz/SDL_QuartzVideo.h 
to remove the line containing CGDirectPaletteRef as this type no longer exists
on this platform as of OS X 10.9.

Next you will need to use the patch file named sdl_x11sym.patch in the patch directory on 
the ${sdl_srcdir}/video/x11/SDL_x11sym.h file.

Then you can build and install SDL

```
mkdir build; cd build
make
make install
```

Once SDL is installed you can proceed with the normal build of X3D.
