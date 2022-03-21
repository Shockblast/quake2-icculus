id Software Quake2 3.19+Changes by Steven Fuller <relnev@icculus.org>

Be sure to install SDL 1.2 (http://www.libsdl.org).

These modifications are intended for Linux users, as I do not have have
access to other platforms.

To build fully optimized binaries: cd quake2 && make build_release

I'll post any updates I make at http://www.icculus.org/~relnev/

v0.0.4: [12/23/01]
-------
+ Mouse Wheel (SDL buttons 4 and 5).
+ Fixed bug with changing the sound options in game (using the menus).
+ Fixed Makefile to build both build_debug and build_release by default.

v0.0.3: [12/22/01]
-------
+ Fixed the texture wrapping with movies.
+ Enabled the OpenGL extensions under Linux.
+ Added support for GL_ARB_multitexture.

v0.0.2: [12/22/01]
-------
+ Added ref_sdlgl.so (SDL OpenGL Renderer).
+ v0.0.1 Bugfixes.

v0.0.1: [12/22/01]
-------
+ Updates to Linux Makefile (it was missing a few files).
+ Added ref_softsdl.so (Software SDL Renderer).
- OpenGL not yet supported.

Thanks:
-------
John Allensworth
