#ifndef _ENGINE_HPP_
#define _ENGINE_HPP_

#include <interface.hpp>
#include <logging.hpp>
#include <except.hpp>
#include <fontrenderer.hpp>
#include <gl.hpp>

class Widget;
class Frame;
class Window;
class Engine;

typedef std::list<Frame*> FrameList;
//~ typedef std::list<Message*> MessageList;

class Engine
{
	private:
		Uint16 W;
		Uint16 H;
		static bool instanceExists;
		static bool quit;
		static bool redraw;
		static Window* window_ptr;
		static const SDL_VideoInfo* video;
	public:
		static SDLFontRenderer * defFont;
		static const char TirianVersion[];
		static const char DefaultIconPath[];
		static const char DefaultCaption[];
		static const char DefaultFontPath[];
		static const int DefaultFontHeight;
		
		static float backgroundColor[3];
		static float activeButtonColor[3];
		static float nonActiveButtonColor[3];
		static float bevelColorDiff;
		static float bevelWidth;
		static float buttonBevelWidth;
	private:
		void printSDLVersions();
		void printGLVersion();
	public:
		Engine( int argc, char** argv, Uint16 width, Uint16 height, const char* captionTitle, const char* captionIcon, const char* icon, Uint32 videoModeFlags = 0 );
		~Engine();
		
		//~ void MotionEvent( const SDL_MouseMotionEvent &e );
		//~ void ButtonEvent( const SDL_MouseButtonEvent &e );
		//~ void KeyEvent( const SDL_KeyboardEvent &e );
		
		void MainLoop();
		
		static void Quit() { quit = 1; }
		static bool isQuiting() { return quit; }
		
		static void Redraw() { redraw = 1; }
		
		static Window& window() { return *window_ptr; }
};


#endif //_ENGINE_HPP_
