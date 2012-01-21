#include <main.hpp>

#include <engine.hpp>
#include <widgets>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Template"
#define WINDOW_TITLE_SHORT "Template"
#define WINDOW_ICON "Template.png"
#define WINDOW_SDL_FLAGS 0 /*| SDL_FULLSCREEN*/


int main( int argc, char ** argv )
{
	Engine engine( argc, argv, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, WINDOW_TITLE_SHORT, WINDOW_ICON, WINDOW_SDL_FLAGS );
	
	engine.MainLoop();
	
	return 0;
}
