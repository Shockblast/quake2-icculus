/*
** QGL_WIN.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake2 you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/
#include <float.h>
#include "../ref_gl/gl_local.h"
#include "glw_solaris.h"

#include <GL/glx.h>

/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.
*/
void QGL_Shutdown( void )
{
  printf( "shutting down qgl\n" );
}

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to
** the appropriate GL stuff.  In Windows this means doing a
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
**
*/

qboolean QGL_Init( const char *dllname )
{
  printf( "initializing qgl\n" );

  return true;
}

void GLimp_EnableLogging( qboolean enable )
{
  if( enable ) {
    printf( "OpenGL logging not available\n" );
  }
}


void GLimp_LogNewFrame( void )
{
	fprintf( glw_state.log_fp, "*** R_BeginFrame ***\n" );
}


void *qwglGetProcAddress(char *symbol)
{
  return NULL;
}
