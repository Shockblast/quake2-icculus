id Software Quake2 3.19+Changes by Steven Fuller <relnev@icculus.org>

Be sure to install SDL 1.2 (http://www.libsdl.org).

These modifications are intended for Linux users, as I do not have have
access to other platforms.

To build fully optimized binaries: cd quake2 && make build_release
NOTE: gcc 2.96 is reported to not work with build_release (it dies on
game/game/p_client.c).  Use build_debug instead.

To install:
(builddir is either debugi386 or releasei386)
copy <builddir>/gamei386.so to <installdir>/baseq3/
copy <builddir>/ref_*.so to <path in /etc/quake2>
copy <builddir>/quake2 to <installdir>/

To run:
cd <installdir> && ./quake2

NOTE: Savegames will most likely not work across different versions or
builds (this is due to how savegames were stored).

Be sure to edit /etc/quake2/ if needed (this file contains <installdir>,
or where the ref_*.so were copied).

Commonly used commands:
cd_nocd 0               // disable CD audio
s_initsound 0           // disable sound
_windowed_mouse 0       // disable mouse-grabbing
gl_ext_multitexture 0   // disable OpenGL Multitexturing (requires a
                           vid_restart)
vid_ref <driver>        // select a video driver (softx is the original
                           X11-only, softsdl is SDL software, sdlgl is 
                           SDL OpenGL)
vid_fullscreen 0        // disable fullscreen mode


I'll post any updates I make at http://www.icculus.org/~relnev/

Questions:
----------
Should /etc/quake2.conf support be removed?
What's the best way of handling international keyboards with SDL?

TODO:
-----
Suggestions, anyone?

v0.0.5: [12/23/01]
-------
+ Better SDL de/initialization (fixes crashes for some people).
+ Removed trailing '\r's from files; removed a few files.

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
Matti Valtonen
