#ifndef _ENGINE_HPP_
#define _ENGINE_HPP_

#include <interface.hpp>
#include <logging.hpp>
#include <except.hpp>
#include <fontrenderer.hpp>
#include <gl.hpp>
#include <queue>
#include <memory>

class Widget;
class Frame;
class Window;
class Engine;

typedef std::list<Frame*> FrameList;
//~ typedef std::list<Message*> MessageList;


class Engine
{
	public:
		class Event
		{
			protected:
				Event( ulong execTime ) : executionTime( execTime ) {}
			public:
				const Uint32 executionTime;
				virtual ~Event() {}
				virtual void execute() = 0;
		};
	private:
		class ECmp{
			public:
			bool operator()( const Event* a, const Event* b )
				{ return a->executionTime > b->executionTime; }
		};
	private:
		static Uint16 W;
		static Uint16 H;
		static Uint16 windowW;
		static Uint16 windowH;
		static Uint16 screenW;
		static Uint16 screenH;
		static bool instanceExists;
		static bool quit;
		static bool redraw;
		static Window* window_ptr;
		static std::priority_queue<Event*, std::vector<Event*>, ECmp> eventQueue;
		static Uint32 videoModeFlags;
		static const SDL_VideoInfo* video;
	public:
		static SDLFontRenderer * defFont;
		static const char TirianVersion[];
		static const char DefaultIconPath[];
		static const char DefaultCaption[];
		static const char DefaultFontPath[];
		static const int DefaultFontTextureHeight;
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
		
		void MainLoop();
		
		static void pushEvent( Event* ptr ) { eventQueue.push( ptr ); }
		
		static void Quit() { quit = 1; }
		static bool isQuiting() { return quit; }
		
		static void Redraw() { redraw = 1; }
		
		static Uint32 getVideoModeFlags() { return videoModeFlags; }
		static void setVideoModeFlags( Uint32 flags );
		static Window& window() { return *window_ptr; }
};


#endif //_ENGINE_HPP_
