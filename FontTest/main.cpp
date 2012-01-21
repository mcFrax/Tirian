#include <main.hpp>

#include <engine.hpp>
#include <widgets>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Template"
#define WINDOW_TITLE_SHORT "Template"
#define WINDOW_ICON "Template.png"
#define WINDOW_SDL_FLAGS 0 /*| SDL_FULLSCREEN*/

#include <fontrenderer.hpp>

class Label2 : public Widget
{
	private:
		std::string Caption;
		SDLFontRenderer * Font;
	protected:
		void onDraw();
	public:
		Label2( const std::string& name, Widget* new_parent, long left, long top, long width, long height, SDLFontRenderer * font, const std::string& caption );
		
		const std::string& caption() const { return Caption; }
		void setCaption( const std::string& c ) { Caption = c; if ( isVisible() ) Engine::Redraw(); }
};

Label2::Label2( const std::string& name, Widget* new_parent, long left, long top, long width, long height, SDLFontRenderer * font, const std::string& caption )
	: Widget( name, new_parent, left, top, width, height ), Caption( caption ), Font( font )
{
}

void Label2::onDraw()
{
	glActiveTexture( GL_TEXTURE1 );
	glDisable(GL_TEXTURE_2D);
	
	glActiveTexture( GL_TEXTURE0 );
	glEnable(GL_TEXTURE_2D);
	
	glDisable( GL_FOG );
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glColorMaterial( GL_BACK, GL_AMBIENT );
	glEnable( GL_COLOR_MATERIAL );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gltErrorCheck();
	
	int textw, texth;
	TTF_SizeUTF8( Font -> getFont(), Caption.c_str(), &textw, &texth );
	SDL_Color color;
	SDL_Surface * txt = TTF_RenderUTF8_Blended( Font -> getFont(), Caption.c_str(), color );
	Texture tex( txt, GL_ALPHA, 0, 0 );
	SDL_FreeSurface( txt );
	tex.safeBind();
	
	glBegin( GL_QUADS );
		glTexCoord2f( tex.x1(), tex.y1() );
		glVertex2f( (width()-textw)/2, std::max( (height()-texth)/2, 0l ) );
		glTexCoord2f( tex.x2(), tex.y1() );
		glVertex2f( (width()-textw)/2+textw, std::max( (height()-texth)/2, 0l ) );
		glTexCoord2f( tex.x2(), tex.y2() );
		glVertex2f( (width()-textw)/2+textw, std::max( (height()-texth)/2, 0l )+texth );
		glTexCoord2f( tex.x1(), tex.y2() );
		glVertex2f( (width()-textw)/2, std::max( (height()-texth)/2, 0l )+texth );
	glEnd();
	
	//~ Font -> render( Caption, (width()-textw)/2, std::max( (height()-texth)/2, 0l ) );
	
	glDisable(GL_TEXTURE_2D);
}

#define ILE 100

Label *l1[ILE];
Label2 *l2[ILE];

uint t = 0;
uint frames = 0;
uint lastPrint = 0;

void reset()
{
	lastPrint = t = SDL_GetTicks();
	frames = 0;
	printf( "reset\n" );
}

void ccb1( Widget& sender, const MouseButtonEvent& mbe )
{
	for ( int i = 0; i < ILE; ++i ){
		l1[i] -> setVisible( 1 );
		l2[i] -> setVisible( 0 );
	}
	reset();
}

void ccb2( Widget& sender, const MouseButtonEvent& mbe )
{
	for ( int i = 0; i < ILE; ++i ){
		l1[i] -> setVisible( 0 );
		l2[i] -> setVisible( 1 );
	}
	reset();
}

static const uint D = 2000;
void icb( Widget& sender )
{
	if ( SDL_GetTicks() - lastPrint >= D ){
		lastPrint += D;
		printf( "FPS = %f\n", frames*1000.0f / (SDL_GetTicks() - t) );
	}
	++frames;
	Engine::Redraw();
}

int main( int argc, char ** argv )
{
	Engine engine( argc, argv, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, WINDOW_TITLE_SHORT, WINDOW_ICON, WINDOW_SDL_FLAGS );
	
	Frame *f = new Frame( "F" );
	f -> setVisible( 1 );
	
	Engine::window().idleCallback = icb;
	
	for ( int i = 0; i < ILE; ++i ){
		l1[i] = new Label( "L", f, 10, 10, 900, 20, Engine::defFont, "AbcdefghijklmnopqrstuvwxyzAbcdefghijklmnopqrstuvwxyzAbcdefghijklmnopqrstuvwxyzAbcdefghijklmnopqrstuvwxyz" );
		l2[i] = new Label2( "L2", f, 10, 40, 900, 20, Engine::defFont, "AbcdefghijklmnopqrstuvwxyzAbcdefghijklmnopqrstuvwxyzAbcdefghijklmnopqrstuvwxyzAbcdefghijklmnopqrstuvwxyz" );
	}
	
	( new Button( "1", f, 10, 70, 100, 24 ) ) -> clickCallback = ccb1;
	( new Button( "2", f, 110, 70, 100, 24 ) ) -> clickCallback = ccb2;
	
	t = SDL_GetTicks();
	engine.MainLoop();
	
	return 0;
}
