#ifndef _GL_HPP_
#define _GL_HPP_

#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <logging.hpp>

static const char glerrorstring[][21] = {
	"GL_INVALID_ENUM",				//	0x0500
	"GL_INVALID_VALUE",				//	0x0501
	"GL_INVALID_OPERATION",			//	0x0502
	"GL_STACK_OVERFLOW",			//	0x0503
	"GL_STACK_UNDERFLOW",			//	0x0504
	"GL_OUT_OF_MEMORY"				//	0x0505
	};


inline void gltErrorCheck()
{
	for ( GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError() )
		LOG(( "OpenGL error : %s\n", glerrorstring[ err - 0x0500 ] ));
}

inline void gltErrorCheck( const char * file, int line )
{
	for ( GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError() )
		LOG(( "%s:%i: OpenGL error : %s\n", file, line, glerrorstring[ err - 0x0500 ] ));
}

#define GLTERRORCHECK gltErrorCheck( __FILE__, __LINE__ )

class SetOrtho
{
	public:
		SetOrtho( long w, long h )
		{
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho( 0, w, h, 0, 0.0f, 1.0f );
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
		}
		~SetOrtho()
		{
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
};

class GltSetWindow
{
	private:
		//SetOrtho ortho;
	public:
		GltSetWindow( long w, long h )
			//~ : ortho( w, h )
		{
			GLTERRORCHECK;
			glViewport( 0, 0, w, h );
			
			GLTERRORCHECK;
			
			glMatrixMode(GL_MODELVIEW);
			//~ glPushMatrix();
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			//~ glPushMatrix();
			glLoadIdentity();
			GLTERRORCHECK;
			glOrtho( 0, w, h, 0, 0.0f, 1.0f );
			GLTERRORCHECK;
			
			//temp:
			glDisable( GL_LIGHTING );
			//~ glEnable( GL_DEPTH_TEST );
			//~ glDisable( GL_DEPTH_TEST );
			glEnable(GL_BLEND);
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glEnable( GL_COLOR_MATERIAL );
			
			//~ glShadeModel( GL_SMOOTH );
		
			//~ glEnable(GL_SCISSOR_TEST);
			//~ glScissor( 0, 0, w, h );
			
			GLTERRORCHECK;
		}
		~GltSetWindow()
		{
			//~ glMatrixMode(GL_PROJECTION);
			//~ glPopMatrix();
			//~ glMatrixMode(GL_MODELVIEW);
			//~ glPopMatrix();
		}
};

#endif //_GL_HPP_
