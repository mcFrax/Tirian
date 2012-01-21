#include <widgets.hpp>


static const float BUTTON_BORDER = 3.0f;

Label::Label( const std::string& name, Widget* new_parent, long left, long top, long width, long height, SDLFontRenderer * font, const std::string& caption )
	: Widget( name, new_parent, left, top, width, height ), Caption( caption ), Font( font )
{
}

void Label::onDraw()
{
	int textw, texth;
	TTF_SizeUTF8( Font -> getFont(), Caption.c_str(), &textw, &texth );
	
	if ( textw > width() ){
		float H = width()/textw * texth;
		Font -> render( Caption, 0, (height()-H)/2, H );
	} else {
		Font -> render( Caption, (width()-textw)/2, std::max( (height()-texth)/2, 0l ) );
	}
	
	glDisable(GL_TEXTURE_2D);
}

const long Button::defaultHeight = 16;

Button::Button( const std::string& name, Widget* new_parent, long left, long top, long width, long height )
	:	Widget( name, new_parent, left, top, width, height ),
		label( new Label( std::string(), this, BUTTON_BORDER, BUTTON_BORDER, width-BUTTON_BORDER*2, height-BUTTON_BORDER*2, Engine::defFont, name ) )
{
	label -> setEnabled( 0 );
	label -> anchors = AN_LEFT | AN_RIGHT | AN_TOP | AN_BOTTOM;
}

void Button::onEnabled()
{
	Engine::Redraw();
}


void Button::onDraw()
{
	drawButton( 0, 0, width(), height(), isEnabled(), !pressed );
}

void Button::onMotion( const MouseMotionEvent& e )
{
	#warning temp!
	Engine::Redraw();
}

void Button::onButton( const MouseButtonEvent& e )
{
	Engine::Redraw();
}

Image::Image( const std::string& name, Widget* new_parent, long left, long top, long width, long height, const Texture& pat )
	: Widget( name, new_parent, left, top, width, height ), tex( pat ), fillType( scale )
{
}

void Image::setTexture( const Texture& pat )
{
	tex = pat;
}

void Image::onDraw()
{	
	static float vertex[4][2];
	static float texCoord[4][2];
	
	long w = width(), h = height();
	
	if ( !tex ) return;
	glEnable( GL_TEXTURE_2D );
	tex.safeBind();
	
	glVertexPointer( 2, GL_FLOAT, 0, vertex );
	glTexCoordPointer( 2, GL_FLOAT, 0, texCoord );
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	texCoord[0][0] = tex.x1(); texCoord[0][1] = tex.y1();
	texCoord[1][0] = tex.x2(); texCoord[1][1] = tex.y1();
	texCoord[2][0] = tex.x2(); texCoord[2][1] = tex.y2();
	texCoord[3][0] = tex.x1(); texCoord[3][1] = tex.y2();
	
	if ( float(w)/h < float(tex.width())/tex.height() ){
		//skalujemy po szerokości
		vertex[0][0] = 0; vertex[0][1] = 0;
		vertex[1][0] = w; vertex[1][1] = 0;
		vertex[2][0] = w; vertex[2][1] = tex.height() * w/tex.width();
		vertex[3][0] = 0; vertex[3][1] = tex.height() * w/tex.width();
	} else {
		//skalujemy po wysokości
		vertex[0][0] = 0; vertex[0][1] = 0;
		vertex[1][0] = tex.width() * h/tex.height(); vertex[1][1] = 0;
		vertex[2][0] = tex.width() * h/tex.height(); vertex[2][1] = h;
		vertex[3][0] = 0; vertex[3][1] = h;
	}
	
	glColor4f( 1,1,1,1 );
	
	glDrawArrays( GL_QUADS, 0, 4 );
	
	glDisable( GL_TEXTURE_2D );
}

AnimatedImage::AnimatedImage( const std::string& name, Widget* new_parent, long left, long top, long width, long height, const Texture& pat, Uint32 frames, Uint32 interval )
	:	Image( name, new_parent, left, top, width, height ),
		lastTime( SDL_GetTicks() ), last(0), Interval( interval )
{
	setTexture( pat, frames );
}

void AnimatedImage::setTexture( const Texture& pat, Uint32 frames )
{
	if ( (!frames) || (!pat) ){
		texes.resize( 0 );
		tex.reset();
		return;
	}
		
	if ( frames >= 0 ){
		texes.resize( frames );
		for ( Uint32 i = 0; i < frames; ++i ){
			texes[i] = Texture( &pat, i*pat.width()/frames, 0, (i+1)*pat.width()/frames - i*pat.width()/frames, pat.height() );
		}
	} else {
		frames = -frames;
		texes.resize( frames );
		for ( Uint32 i = 0; i < frames; ++i ){
			texes[i] = Texture( &pat, 0, i*pat.height()/frames, pat.width(), (i+1)*pat.height()/frames - i*pat.height()/frames );
		}
	}
	tex = texes[0];
	last = 0;
	lastTime = SDL_GetTicks();
	
	Engine::Redraw();
}

void AnimatedImage::reset()
{
	lastTime = SDL_GetTicks();
	last = 0;
}

void AnimatedImage::onDraw()
{
	if ( texes.empty() ) return;
	if ( Interval ){
		Uint32 t = ( SDL_GetTicks() - lastTime ) / Interval;
		( last += t ) %= texes.size();
		lastTime += t*Interval;
	} else {
		last = 0;
		lastTime = SDL_GetTicks();
	}
	
	tex = texes[ last ];
	
	Image::onDraw();
	Engine::Redraw();
}
