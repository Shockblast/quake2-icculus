/*
** RW_SDL.C
**
** This file contains ALL Linux specific stuff having to do with the
** software refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** SWimp_EndFrame
** SWimp_Init
** SWimp_InitGraphics
** SWimp_SetPalette
** SWimp_Shutdown
** SWimp_SwitchFullscreen
*/

#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL/SDL.h>

#ifdef OPENGL
#include <GL/gl.h>
#endif

#ifdef OPENGL
#include "../ref_gl/gl_local.h"
#else
#include "../ref_soft/r_local.h"
#endif

#include "../client/keys.h"
#include "../linux/rw_linux.h"

/*****************************************************************************/

static qboolean                 X11_active = false;

static SDL_Surface *surface;
static int sdl_palettemode;

struct
{
	int key;
	int down;
} keyq[64];
int keyq_head=0;
int keyq_tail=0;

int config_notify=0;
int config_notify_width;
int config_notify_height;
						      
// Console variables that we need to access from this module

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

// this is inside the renderer shared lib, so these are called from vid_so

static qboolean        mouse_avail;
static int     mouse_buttonstate;
static int     mouse_oldbuttonstate;
static int   mouse_x, mouse_y;
static int	old_mouse_x, old_mouse_y;
static int		mx, my;
static float old_windowed_mouse;
static int p_mouse_x, p_mouse_y;

static cvar_t	*_windowed_mouse;
static cvar_t	*m_filter;
static cvar_t	*in_mouse;

static qboolean	mlooking;

// state struct passed in Init
static in_state_t	*in_state;

static cvar_t *sensitivity;
static cvar_t *lookstrafe;
static cvar_t *m_side;
static cvar_t *m_yaw;
static cvar_t *m_pitch;
static cvar_t *m_forward;
static cvar_t *freelook;

static void Force_CenterView_f (void)
{
	in_state->viewangles[PITCH] = 0;
}

static void RW_IN_MLookDown (void) 
{ 
	mlooking = true; 
}

static void RW_IN_MLookUp (void) 
{
	mlooking = false;
	in_state->IN_CenterView_fp ();
}

void RW_IN_Init(in_state_t *in_state_p)
{
	int mtype;
	int i;

	in_state = in_state_p;

	// mouse variables
	_windowed_mouse = ri.Cvar_Get ("_windowed_mouse", "0", CVAR_ARCHIVE);
	m_filter = ri.Cvar_Get ("m_filter", "0", 0);
    in_mouse = ri.Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);
	freelook = ri.Cvar_Get( "freelook", "0", 0 );
	lookstrafe = ri.Cvar_Get ("lookstrafe", "0", 0);
	sensitivity = ri.Cvar_Get ("sensitivity", "3", 0);
	m_pitch = ri.Cvar_Get ("m_pitch", "0.022", 0);
	m_yaw = ri.Cvar_Get ("m_yaw", "0.022", 0);
	m_forward = ri.Cvar_Get ("m_forward", "1", 0);
	m_side = ri.Cvar_Get ("m_side", "0.8", 0);

	ri.Cmd_AddCommand ("+mlook", RW_IN_MLookDown);
	ri.Cmd_AddCommand ("-mlook", RW_IN_MLookUp);

	ri.Cmd_AddCommand ("force_centerview", Force_CenterView_f);

	mouse_x = mouse_y = 0.0;
	mouse_avail = true;
}

void RW_IN_Shutdown(void)
{
	mouse_avail = false;
}

/*
===========
IN_Commands
===========
*/
void RW_IN_Commands (void)
{
	int i;
   
	if (!mouse_avail) 
		return;
   
	for (i=0 ; i<3 ; i++) {
		if ( (mouse_buttonstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) )
			in_state->Key_Event_fp (K_MOUSE1 + i, true);

		if ( !(mouse_buttonstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) )
			in_state->Key_Event_fp (K_MOUSE1 + i, false);
	}
	mouse_oldbuttonstate = mouse_buttonstate;
}

/*
===========
IN_Move
===========
*/
void RW_IN_Move (usercmd_t *cmd)
{
	if (!mouse_avail)
		return;
   
	if (m_filter->value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	} else {
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	if (!mouse_x && !mouse_y)
		return;

	mouse_x *= sensitivity->value;
	mouse_y *= sensitivity->value;

// add mouse X/Y movement to cmd
	if ( (*in_state->in_strafe_state & 1) || 
		(lookstrafe->value && mlooking ))
		cmd->sidemove += m_side->value * mouse_x;
	else
		in_state->viewangles[YAW] -= m_yaw->value * mouse_x;

	if ( (mlooking || freelook->value) && 
		!(*in_state->in_strafe_state & 1))
	{
		in_state->viewangles[PITCH] += m_pitch->value * mouse_y;
	}
	else
	{
		cmd->forwardmove -= m_forward->value * mouse_y;
	}
	mx = my = 0;
}

void RW_IN_Frame (void)
{
}

void RW_IN_Activate(void)
{
}

/*****************************************************************************/


// ========================================================================
// Tragic death handler
// ========================================================================

void TragicDeath(int signal_num)
{
	/* SDL_Quit(); */
	Sys_Error("This death brought to you by the number %d\n", signal_num);
}

int XLateKey(unsigned int keysym)
{
	int key;
	
	key = 0;
	switch(keysym) {
		case SDLK_KP9:			key = K_KP_PGUP; break;
		case SDLK_PAGEUP:		key = K_PGUP; break;
		
		case SDLK_KP3:			key = K_KP_PGDN; break;
		case SDLK_PAGEDOWN:		key = K_PGDN; break;
		
		case SDLK_KP7:			key = K_KP_HOME; break;
		case SDLK_HOME:			key = K_HOME; break;
		
		case SDLK_KP1:			key = K_KP_END; break;
		case SDLK_END:			key = K_END; break;
		
		case SDLK_KP4:			key = K_KP_LEFTARROW; break;
		case SDLK_LEFT:			key = K_LEFTARROW; break;
		
		case SDLK_KP6:			key = K_KP_RIGHTARROW; break;
		case SDLK_RIGHT:		key = K_RIGHTARROW; break;
		
		case SDLK_KP2:			key = K_KP_DOWNARROW; break;
		case SDLK_DOWN:			key = K_DOWNARROW; break;
		
		case SDLK_KP8:			key = K_KP_UPARROW; break;
		case SDLK_UP:			key = K_UPARROW; break;
		
		case SDLK_ESCAPE:		key = K_ESCAPE; break;
		
		case SDLK_KP_ENTER:		key = K_KP_ENTER; break;
		case SDLK_RETURN:		key = K_ENTER; break;
		
		case SDLK_TAB:			key = K_TAB; break;
		
		case SDLK_F1:			key = K_F1; break;
		case SDLK_F2:			key = K_F2; break;
		case SDLK_F3:			key = K_F3; break;
		case SDLK_F4:			key = K_F4; break;
		case SDLK_F5:			key = K_F5; break;
		case SDLK_F6:			key = K_F6; break;
		case SDLK_F7:			key = K_F7; break;
		case SDLK_F8:			key = K_F8; break;
		case SDLK_F9:			key = K_F9; break;
		case SDLK_F10:			key = K_F10; break;
		case SDLK_F11:			key = K_F11; break;
		case SDLK_F12:			key = K_F12; break;
		
		case SDLK_BACKSPACE:		key = K_BACKSPACE; break;
		
		case SDLK_KP_PERIOD:		key = K_KP_DEL; break;
		case SDLK_DELETE:		key = K_DEL; break;
		
		case SDLK_PAUSE:		key = K_PAUSE; break;
		
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:		key = K_SHIFT; break;
		
		case SDLK_LCTRL:
		case SDLK_RCTRL:		key = K_CTRL; break;
		
		case SDLK_LMETA:
		case SDLK_RMETA:
		case SDLK_LALT:
		case SDLK_RALT:			key = K_ALT; break;

		case SDLK_KP5:			key = K_KP_5; break;
		
		case SDLK_INSERT:		key = K_INS; break;
		case SDLK_KP0:			key = K_KP_INS; break;
		
		case SDLK_KP_MULTIPLY:		key = '*'; break;
		case SDLK_KP_PLUS:		key = K_KP_PLUS; break;
		case SDLK_KP_MINUS:		key = K_KP_MINUS; break;
		case SDLK_KP_DIVIDE:		key = K_KP_SLASH; break;
		
		default: /* assuming that the other sdl keys are mapped to ascii */
			if (keysym < 128)
				key = keysym;
			break;
	}
	
	return key;		
}

void GetEvent(SDL_Event *event)
{
	unsigned int bstate;
	
	switch(event->type) {
	case SDL_KEYDOWN:
		keyq[keyq_head].key = XLateKey(event->key.keysym.sym);
		keyq[keyq_head].down = true;
		keyq_head = (keyq_head + 1) & 63;
		break;
	case SDL_KEYUP:
		keyq[keyq_head].key = XLateKey(event->key.keysym.sym);
		keyq[keyq_head].down = false;
		keyq_head = (keyq_head + 1) & 63;
		break;
#if 0
	case MotionNotify:
		if (_windowed_mouse->value) {
			mx += ((int)x_event.xmotion.x - (int)(vid.width/2));
			my += ((int)x_event.xmotion.y - (int)(vid.height/2));

			/* move the mouse to the window center again */
			XSelectInput(x_disp,x_win, STD_EVENT_MASK & ~PointerMotionMask);
			XWarpPointer(x_disp,None,x_win,0,0,0,0, 
				(vid.width/2),(vid.height/2));
			XSelectInput(x_disp,x_win, STD_EVENT_MASK);
		} else {
			mx = ((int)x_event.xmotion.x - (int)p_mouse_x);
			my = ((int)x_event.xmotion.y - (int)p_mouse_y);
			p_mouse_x=x_event.xmotion.x;
			p_mouse_y=x_event.xmotion.y;
		}
		break;

	case ButtonPress:
		b=-1;
		if (x_event.xbutton.button == 1)
			b = 0;
		else if (x_event.xbutton.button == 2)
			b = 2;
		else if (x_event.xbutton.button == 3)
			b = 1;
		if (b>=0)
			mouse_buttonstate |= 1<<b;
		break;

	case ButtonRelease:
		b=-1;
		if (x_event.xbutton.button == 1)
			b = 0;
		else if (x_event.xbutton.button == 2)
			b = 2;
		else if (x_event.xbutton.button == 3)
			b = 1;
		if (b>=0)
			mouse_buttonstate &= ~(1<<b);
		break;
#endif	
	}

}

/*****************************************************************************/

/*
** SWimp_Init
**
** This routine is responsible for initializing the implementation
** specific stuff in a software rendering subsystem.
*/
int SWimp_Init( void *hInstance, void *wndProc )
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "DEBUG: SDL Init failed: %s\n", SDL_GetError());
		return false;
	}
	
	atexit(SDL_Quit);
	
// catch signals so i can turn on auto-repeat
#if 0
 	{
		struct sigaction sa;
		sigaction(SIGINT, 0, &sa);
		sa.sa_handler = TragicDeath;
		sigaction(SIGINT, &sa, 0);
		sigaction(SIGTERM, &sa, 0);
	}
#endif

	return true;
}

int GLimp_Init( void *hInstance, void *wndProc )
{
	return SWimp_Init(hInstance, wndProc);
}

/*
** SWimp_InitGraphics
**
** This initializes the software refresh's implementation specific
** graphics subsystem.  In the case of Windows it creates DIB or
** DDRAW surfaces.
**
** The necessary width and height parameters are grabbed from
** vid.width and vid.height.
*/
#ifndef OPENGL
static qboolean SWimp_InitGraphics( qboolean fullscreen )
{
	const SDL_VideoInfo *vinfo;

	srandom(getpid());

	// free resources in use
	SWimp_Shutdown ();

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (vid.width, vid.height);

/* 
	Okay, I am going to query SDL for the "best" pixel format.
	If the depth is not 8, use SetPalette with logical pal, 
	else use SetColors.
	
	Hopefully this works all the time.
*/
	vinfo = SDL_GetVideoInfo();
	sdl_palettemode = (vinfo->vfmt->BitsPerPixel == 8) ? (SDL_PHYSPAL|SDL_LOGPAL) : SDL_LOGPAL;
	
// check for command-line window size
	if ((surface = SDL_SetVideoMode(vid.width, vid.height, 8, /*SDL_DOUBLEBUF|*/SDL_SWSURFACE|SDL_HWPALETTE)) == NULL) {
		return false;
	}

	if (fullscreen)
		SDL_WM_ToggleFullScreen(surface);
	
	SDL_WM_SetCaption("Quake II", "Quake II");

	SDL_ShowCursor(0);

	vid.rowbytes = surface->pitch;
	vid.buffer = surface->pixels;

	X11_active = true;

	return true;
}
#else
static qboolean GLimp_InitGraphics( qboolean fullscreen )
{
	srandom(getpid());

	// free resources in use
	GLimp_Shutdown ();

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (vid.width, vid.height);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	if ((surface = SDL_SetVideoMode(vid.width, vid.height, 0, SDL_OPENGL)) == NULL) {
		fprintf(stderr, "(OpenGL) SDL SetVideoMode failed: %s\n", SDL_GetError());
		return false;
	}

	if (fullscreen)
		SDL_WM_ToggleFullScreen(surface);
	
	SDL_WM_SetCaption("Quake II", "Quake II");

	SDL_ShowCursor(0);

	X11_active = true;

	return true;
}
#endif

#ifdef OPENGL
void GLimp_BeginFrame( float camera_seperation )
{
}
#endif

/*
** SWimp_EndFrame
**
** This does an implementation specific copy from the backbuffer to the
** front buffer.  In the Win32 case it uses BitBlt or BltFast depending
** on whether we're using DIB sections/GDI or DDRAW.
*/

#ifndef OPENGL
void SWimp_EndFrame (void)
{
	/* SDL_Flip(surface); */
	SDL_UpdateRect(surface, 0, 0, 0, 0);
}
#else
void GLimp_EndFrame (void)
{
	SDL_GL_SwapBuffers();
}
#endif

/*
** SWimp_SetMode
*/
#ifndef OPENGL
rserr_t SWimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	rserr_t retval = rserr_ok;

	ri.Con_Printf (PRINT_ALL, "setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( pwidth, pheight, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	ri.Con_Printf( PRINT_ALL, " %d %d\n", *pwidth, *pheight);

	if ( !SWimp_InitGraphics( fullscreen ) ) {
		// failed to set a valid mode in windowed mode
		return rserr_invalid_mode;
	}

	R_GammaCorrectAndSetPalette( ( const unsigned char * ) d_8to24table );

	return retval;
}
#else
int GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	ri.Con_Printf (PRINT_ALL, "setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( pwidth, pheight, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	ri.Con_Printf( PRINT_ALL, " %d %d\n", *pwidth, *pheight);

	if ( !GLimp_InitGraphics( fullscreen ) ) {
		// failed to set a valid mode in windowed mode
		return rserr_invalid_mode;
	}

	return rserr_ok;
}
#endif

/*
** SWimp_SetPalette
**
** System specific palette setting routine.  A NULL palette means
** to use the existing palette.  The palette is expected to be in
** a padded 4-byte xRGB format.
*/
#ifndef OPENGL
void SWimp_SetPalette( const unsigned char *palette )
{
	SDL_Color colors[256];
	
	int i;

	if (!X11_active)
		return;

	if ( !palette )
	        palette = ( const unsigned char * ) sw_state.currentpalette;
 
	for (i = 0; i < 256; i++) {
		colors[i].r = palette[i*4+0];
		colors[i].g = palette[i*4+1];
		colors[i].b = palette[i*4+2];
	}

	SDL_SetPalette(surface, sdl_palettemode, colors, 0, 256);
}
#endif

/*
** SWimp_Shutdown
**
** System specific graphics subsystem shutdown routine.  Destroys
** DIBs or DDRAW surfaces as appropriate.
*/

void SWimp_Shutdown( void )
{
	if (surface)
		SDL_FreeSurface(surface);
	surface == NULL;

	/* SDL_Quit(); */
	
	X11_active = false;
}

void GLimp_Shutdown( void )
{
	SWimp_Shutdown();
}

/*
** SWimp_AppActivate
*/
void SWimp_AppActivate( qboolean active )
{
}

void GLimp_AppActivate( qboolean active )
{
}

//===============================================================================

/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{

	int r;
	unsigned long addr;
	int psize = getpagesize();

	addr = (startaddr & ~(psize-1)) - psize;

//	fprintf(stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr,
//			addr, startaddr+length, length);

	r = mprotect((char*)addr, length + startaddr - addr + psize, 7);

	if (r < 0)
    		Sys_Error("Protection change failed\n");

}

/*****************************************************************************/
/* KEYBOARD                                                                  */
/*****************************************************************************/

Key_Event_fp_t Key_Event_fp;

void KBD_Init(Key_Event_fp_t fp)
{
	Key_Event_fp = fp;
}

void KBD_Update(void)
{
	SDL_Event event;
	
// get events from x server
	if (X11_active)
	{
		int bstate;
		
		while (SDL_PollEvent(&event))
			GetEvent(&event);

	
	SDL_GetRelativeMouseState(&mx, &my);
	
	mouse_buttonstate = 0;
	bstate = SDL_GetMouseState(NULL, NULL);
	if (SDL_BUTTON(1) & bstate)
		mouse_buttonstate |= (1 << 0);
	if (SDL_BUTTON(2) & bstate)
		mouse_buttonstate |= (1 << 1);
	if (SDL_BUTTON(3) & bstate)
		mouse_buttonstate |= (1 << 2);
	
	if (old_windowed_mouse != _windowed_mouse->value) {
		old_windowed_mouse = _windowed_mouse->value;

		if (!_windowed_mouse->value) {
			/* ungrab the pointer */
			SDL_WM_GrabInput(SDL_GRAB_OFF);
		} else {
			/* grab the pointer */
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
	}			
		while (keyq_head != keyq_tail)
		{
			Key_Event_fp(keyq[keyq_tail].key, keyq[keyq_tail].down);
			keyq_tail = (keyq_tail + 1) & 63;
		}
	}
}

void KBD_Close(void)
{
}
