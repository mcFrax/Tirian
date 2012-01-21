#include <engine.hpp>
#include <widget.hpp>

bool Engine::instanceExists = 0;
bool Engine::quit = 0;
bool Engine::redraw = 0;
Window* Engine::window_ptr = 0;
SDLFontRenderer * Engine::defFont = 0;
const SDL_VideoInfo* Engine::video = 0;

const char Engine::TirianVersion[] = "0.2.0 alpha";
const char Engine::DefaultIconPath[] = "/home/frax/Programowanie/GalViewer/Tirian.png";
const char Engine::DefaultCaption[] = "Tirian application";
const char Engine::DefaultFontPath[] = "/usr/share/fonts/truetype/freefont/FreeSans.ttf";
const int Engine::DefaultFontHeight = 12;

float Engine::backgroundColor[3] = { 0x4C/255.0f, 0x40/255.0f, 0x39/255.0f };//{ 0.5f, 0.5f, 0.5f };	//#4C4039
float Engine::activeButtonColor[3] = { 0x4C/255.0f, 0x40/255.0f, 0x39/255.0f };//{ 0.5f, 0.5f, 0.5f };
float Engine::nonActiveButtonColor[3] = { 0.56f, 0.5f, 0.5f };	//{ 0.8f, 0.8f, 0.8f };
float Engine::bevelColorDiff = 0.13;
float Engine::bevelWidth = 1;
float Engine::buttonBevelWidth = 4.8;

void Engine::printSDLVersions()
{
	const SDL_version *link_version;
	link_version = SDL_Linked_Version();
	printf( "SDL %d.%d.%d\n", 
			link_version->major,
			link_version->minor,
			link_version->patch);
	link_version=TTF_Linked_Version();
	printf( "SDL_ttf %d.%d.%d\n", 
			link_version->major,
			link_version->minor,
			link_version->patch);
}

void Engine::printGLVersion()
{
	printf( "OpenGL version: %s\n", glGetString( GL_VERSION ) );
}

Engine::Engine( int argc, char** argv, Uint16 width, Uint16 height, const char* captionTitle, const char* captionIcon, const char* icon, Uint32 flags )
{
	TRY
		if ( instanceExists )
			throw MKLE( "One instance of Engine already exists." );
		
		instanceExists = 1;
		redraw = 0;
		
		LOG(( "Tirian %s\n", TirianVersion ));
		
		LOG(( "Initializing engine...\n" ));
		
		printSDLVersions();
		
		if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 ) {
			throw MKRE( std::string( "Couldn't initialize SDL :\n" ) += SDL_GetError() );
		}
		
		LOG(( "SDL initialized\n" ));
		
		TTF_Init();
		
		LOG(( "SDL_ttf initialized\n" ));
				
		defFont = new SDLFontRenderer( TTF_OpenFont( DefaultFontPath, DefaultFontHeight ), 1, 1 );
		
		LOG(( "Default font loaded from \"%s\"\n", DefaultFontPath ));
		
		SDL_Surface * ico = (icon) ? IMG_Load( icon ) : 0;
		
		if ( ico ){
			SDL_WM_SetIcon( ico, 0);
			LOG(( "Icon loaded from \"%s\".\n", icon ));
		} else {
			LOG(( "Couldn't load icon from \"%s\". Trying default...\n", icon ));
			ico = IMG_Load( DefaultIconPath );
			if ( ico ){
				SDL_WM_SetIcon( ico, 0);
				LOG(( "Default icon loaded from \"%s\".\n", DefaultIconPath ));
			} else {
				LOG(( "Couldn't load default icon.\n" ));
			}
		}
		
		SDL_WM_SetCaption( 
				captionTitle ? captionTitle : DefaultCaption,
				captionIcon ? captionIcon : DefaultCaption );

		video = SDL_GetVideoInfo();
		
		if( video == 0 ) {
			throw MKRE( std::string( "Couldn't get video information :\n" ) += SDL_GetError() );
		}
		
		if ( flags & SDL_FULLSCREEN ){
			width = video -> current_w;
			height = video -> current_h;
		}
		
		W = width;
		H = height;
		
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		if( SDL_SetVideoMode( W, H, video->vfmt->BitsPerPixel, SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_RESIZABLE | flags ) == 0 ) {
			throw MKRE( std::string( "Couldn't set video mode :\n" ) += SDL_GetError() );
		}
		
		LOG(( "Video mode set\n" ));
		
		printGLVersion();
		
		#warning "Do napisania -> inicjalizacja atrybutów OpenGl'a"
		
		glEnable( GL_COLOR_MATERIAL );
		
		gltErrorCheck();
		
		window_ptr = new Window( W, H );
		
		LOG(( "Initialization complete.\n" ));
	CATCH
}

Engine::~Engine()
{
	LOG(( "Engine exiting...\n" ));
	
	TTF_Quit();
	SDL_Quit();
	instanceExists = 0;
	delete window_ptr;
	delete defFont;
	
	LOG(( "Engine killed.\n" ));
}

void Engine::MainLoop()
{
	TRY
		LOG(( "Starting main loop...\n" ));
		
		Redraw();
		
		for ( unsigned long loops = 0; !quit; ++loops ){
			SDL_Event event;
			while( SDL_PollEvent( &event ) ){
				switch( event.type ) {
					case SDL_KEYDOWN :
					case SDL_KEYUP :
						if ( Widget::getKeyFocus() )
							Widget::getKeyFocus() -> key( event.key );
						else
							window_ptr -> key( event.key );
						break;
					case SDL_MOUSEBUTTONDOWN :
					case SDL_MOUSEBUTTONUP :
						Mouse::refresh( event.button.x, event.button.y );
						if ( Widget::getMouseFocus() )
							Widget::getMouseFocus() -> button( MouseButtonEvent( event.button, Widget::getMouseFocus() -> absLeft(), Widget::getMouseFocus() -> absTop() ) );
						else
							window_ptr -> button( MouseButtonEvent( event.button, 0, 0 ) );
						break;
					case SDL_MOUSEMOTION :
						Mouse::refresh( event.motion.x, event.motion.y );
						if ( Widget::getMouseFocus() )
							Widget::getMouseFocus() -> motion( MouseMotionEvent( event.motion, Widget::getMouseFocus() -> absLeft(), Widget::getMouseFocus() -> absTop() ) );
						else
							window_ptr -> motion( MouseMotionEvent( event.motion, 0, 0 ) );
						break;
					case SDL_VIDEORESIZE :
						W = event.resize.w;
						H = event.resize.h;
						SDL_SetVideoMode( W, H, video->vfmt->BitsPerPixel, SDL_OPENGL | SDL_RESIZABLE );
						window_ptr -> resize( event.resize );
						break;
					case SDL_VIDEOEXPOSE :
						Redraw();
						break;
					case SDL_QUIT :
						Quit();
					default :
						break;
				}
			}
			
			if ( redraw ){
				redraw = 0;
				window_ptr -> draw();
			}
			
			window_ptr -> idle();
		}
		
		LOG(( "Main loop left.\n" ));
		
	CATCH
}
