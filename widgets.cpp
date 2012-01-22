#include <widgets.hpp>

static const float BUTTON_BORDER = 3.0f;

Label::Label( const std::string& name, Widget* new_parent, long left, long top, long width, long height, SDLFontRenderer * font, const std::string& caption )
	: Widget( name, new_parent, left, top, width, height ), Caption( caption ), Font( font ), vertTextPos( vpTop ), horTextPos( hpLeft )
{
}

void Label::onDraw()
{
	int textw, texth;
	TTF_SizeUTF8( Font -> getFont(), Caption.c_str(), &textw, &texth );
	
	float W = float(textw)*Engine::DefaultFontHeight/texth, H = Engine::DefaultFontHeight;
	//~ LOG(( "%f, %f\n", W, H ));
	if ( W > width() ){
		H = width()/textw * texth;
		W = width();
	}
	
	float l = 0, t = 0;
	if ( vertTextPos != vpTop ) t = height()-H;
	if ( vertTextPos == vpCenter ) t /= 2;
	if ( horTextPos != hpLeft ) l = width()-W;
	if ( horTextPos == hpCenter ) l /= 2;
	
	Font -> render( Caption, l, t, H );
	
	glDisable(GL_TEXTURE_2D);
}

Button::Button( const std::string& name, Widget* new_parent, long left, long top, long width, long height )
	:	Widget( name, new_parent, left, top, width, height ),
		label( new Label( std::string(), this, BUTTON_BORDER, BUTTON_BORDER, width-BUTTON_BORDER*2, height-BUTTON_BORDER*2, Engine::defFont, name ) )
{
	label -> setEnabled( 0 );
	label -> anchors = Widget::anLeft | Widget::anRight | Widget::anTop | Widget::anBottom;
	label -> setVertTextPos( Label::vpCenter );
	label -> setHorTextPos( Label::hpCenter );
}

const long Button::defaultHeight = 24;

void Button::onEnabled()
{
	Engine::Redraw();
}


void Button::onDraw()
{
	drawButton( 0, 0, width(), height(), isEnabled(), !underPression );
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
	: Widget( name, new_parent, left, top, width, height ), tex( pat )
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

const long ScrollBar::minSeekbarSize = 3;
const long ScrollBar::defaultWidth = 16;
const long ScrollBar::defaultLongJump = 10;

ScrollBar::SeekBar::SeekBar( const std::string& n, Widget* np, long l, long t, long w, long h, ScrollBar * scr )
	:	Widget( n, np, l, t, w, h ), scroll( scr )
{
	clickRepeat = 1;
}

template <class T1, class T2>
static T1 min( T1 a, T2 b ) { return a<b?a:b; }

template <class T1, class T2>
static T1 max( T1 a, T2 b ) { return a>b?a:b; }

void ScrollBar::SeekBar::onMotion( const MouseMotionEvent& e )
{
	if ( !pressed ) return;
	
	long l = left();
	long t = top();
	
	if ( scroll -> dir == ScrollBar::vertical ){
		t = min( max( t + e.y - my, scroll -> width() ), scroll -> height() - scroll -> width() - height() );
	} else {
		l = min( max( l + e.x - mx, scroll -> height() ), scroll -> width() - scroll -> height() - width() );
	}
	resize( l, t, width(), height() );
	scroll -> seekBarMoved();
}

void ScrollBar::SeekBar::onButton( const MouseButtonEvent& e )
{
	if ( e.state == SDL_PRESSED && e.button == SDL_BUTTON_LEFT ){
		mx = e.x;
		my = e.y;
	}
}

void ScrollBar::SeekBar::onDraw()
{
	GLTERRORCHECK;
	drawButton( 0, 0, width(), height(), scroll -> isEnabled(), !pressed );
}

void ScrollBar::ArrowButton::onDraw()
{
	GLTERRORCHECK;
	drawButton( 0, 0, width(), height(), isEnabled(), !underPression );
	glColor3f( 0, 0, 0 );
	glBegin( GL_TRIANGLES );
	if ( dir < 2 ){ //poziom
		float x1 = ( dir == 1 ) ? width()*3.0f/8 : width()*5.0f/8;
		float y1 = height()/4.0f;
		glVertex2f( x1, y1 );
		glVertex2f( x1, height()-y1 );
		glVertex2f( width()-x1, height()/2.0f );
	} else {	//pion
		float x1 = width()/4.0f;
		float y1 = ( dir == 3 ) ? height()*3.0f/8 : height()*5.0f/8;
		glVertex2f( x1, y1 );
		glVertex2f( width()-x1, y1 );
		glVertex2f( width()/2.0f, height()-y1 );
	}
	glEnd();
}

ScrollBar::ScrollBar( const std::string& n, Widget* np, long l, long t, long w, long h, Direction d, Uint32 len, Uint32 p, Uint32 shj, Uint32 lgj )
	:	Widget( n, np, l, t, w, h ), dir( d ), length( len ), pos( p ),
		shortjump( shj ), longjump( lgj ), clickInterval( 0 ), changeCallback( 0 )
{
	if ( !length ) length = 1;
	if ( pos > length ) pos = length;
	int sbl, sbt, sbw, sbh;
	if ( dir == vertical ){
		sbw = w;
		sbh = (h - 2*w)*longjump/( length+longjump );
		if ( sbh < minSeekbarSize ) sbh = minSeekbarSize;
		if ( sbh > h - 2*w ) sbh = h - 2*w;
		sbl = 0;
		sbt = w+(h - 2*w - sbh)*pos/( length+longjump );
		
		but1 = new ArrowButton( (name() + " -> button1").c_str(), this, 0, 0, w, w, 2 );
		but2 = new ArrowButton( (name() + " -> button2").c_str(), this, 0, h-w, w, w, 3 );
	} else {
		sbw = (w - 2*h)*longjump/( length+longjump );
		if ( sbw < minSeekbarSize ) sbw = minSeekbarSize;
		if ( sbh > w - 2*h ) sbh = w - 2*h;
		sbh = h;
		sbl = h+(w - 2*h - sbw)*pos/( length+longjump );
		sbt = 0;
		
		but1 = new ArrowButton( (name() + " -> button1").c_str(), this, 0, 0, h, h, 0 );
		but2 = new ArrowButton( (name() + " -> button2").c_str(), this, w-h, 0, h, h, 1 );
	}
	
	but1 -> info = but2 -> info = this;
	but1 -> clickCallback = but1cbW;
	but2 -> clickCallback = but2cbW;
	
	seekBar = new SeekBar( name() + " -> seekBar", this, sbl, sbt, sbw, sbh, this );
	seekBar -> anchors |= ( dir == vertical ) ? Widget::anRight : Widget::anBottom;
	
	clickRepeat = 1;
}

void ScrollBar::setLength( Uint32 l )
{
	length = l;
	if ( pos > l ) setPosition( l );
	setSeekBarSize();
	if ( isVisible() ) Engine::Redraw();
}

void ScrollBar::setPosition( Uint32 p )
{
	pos = std::max( Uint32(0), Uint32(std::min( p, length ) ) );
	setSeekBarSize();
	if ( changeCallback ) changeCallback( this );
	if ( isVisible() ) Engine::Redraw();
}

void ScrollBar::setShortJump( Uint32 j )
{
	shortjump = j;
	if ( isVisible() ) Engine::Redraw();
}

void ScrollBar::setLongJump( Uint32 j )
{
	longjump = j;
	setSeekBarSize();
	if ( isVisible() ) Engine::Redraw();
}

void ScrollBar::setDirection( const Direction d )
{
	dir = d;
	if ( dir == vertical ){
		but1 -> dir = 2;
		but2 -> dir = 3;
		but1 -> resize( 0, 0, width(), width() );
		but2 -> resize( 0, height()-width(), width(), width() );
	} else {
		but1 -> dir = 0;
		but2 -> dir = 1;
		but1 -> resize( 0, 0, height(), height() );
		but2 -> resize( width()-height(), 0, height(), height() );
	}
	setSeekBarSize();
	if ( isVisible() ) Engine::Redraw();
}

void ScrollBar::seekBarMoved()
{
	if ( dir == vertical )
		if ( height() - 2*width() - seekBar -> height() )
			pos = length * ( seekBar -> top() - width() ) / ( height() - 2*width() - seekBar -> height() );
		else
			pos = 0;
	else
		if ( width() - 2*height() - seekBar -> width() ) 
			pos = length * ( seekBar -> left() - height() ) / ( width() - 2*height() - seekBar -> width() );
		else
			pos = 0;
	//~ setSeekBarSize();
	if ( changeCallback ) changeCallback( this );
}

void ScrollBar::setSeekBarSize()
{
	if ( dir == vertical ){
		long tracklen = height()-2*width();
		long sbl = ( tracklen ) * longjump / ( length+longjump );
		seekBar -> resize( 0, width()+( tracklen-sbl )*pos/( length+longjump ), width(), sbl );
	} else {
		long tracklen = width()-2*height();
		long sbl = ( tracklen ) * longjump / ( length+longjump );
		seekBar -> resize( height()+( tracklen-sbl )*pos/( length+longjump ), 0, sbl, height() );
	}
}

inline void ScrollBar::but1cb()
{
	pos = max( 0, signed(pos)-signed(shortjump) );
	setSeekBarSize();
	if ( changeCallback ) changeCallback( this );
}

inline void ScrollBar::but2cb()
{
	pos = min( length, pos+shortjump );
	setSeekBarSize();
	if ( changeCallback ) changeCallback( this );
}

void ScrollBar::but1cbW( Widget * s, const MouseButtonEvent& e )
{
	reinterpret_cast<ScrollBar*>( s -> info ) -> but1cb();
}

void ScrollBar::but2cbW( Widget * s, const MouseButtonEvent& e )
{
	reinterpret_cast<ScrollBar*>( s -> info ) -> but2cb();
}

void ScrollBar::onClick( const MouseButtonEvent& e )
{
	if ( dir == vertical ){
		if ( e.y > seekBar -> top() )
			setPosition( pos + longjump );
		else
			setPosition( pos - longjump );
	} else {
		if ( e.x > seekBar -> left() )
			setPosition( pos + longjump );
		else
			setPosition( pos - longjump );
	}
}

void ScrollBar::onResize()
{
	setSeekBarSize();
}

void ScrollBar::onDraw()
{
	GLTERRORCHECK;
	#warning zoptymalizuj
	glColor3f( 0.5, 0.5, 0.5 );
	glBegin( GL_QUADS );
		glVertex2f( 0, 0 );
		glVertex2f( width(), 0 );
		glVertex2f( width(), height() );
		glVertex2f( 0, height() );
	glEnd();
	GLTERRORCHECK;
}


ScrollBox::ScrollBox( const std::string& n, Widget* np, long l, long t, long w, long h )
	:	Widget( n, np, l, t, w, h ), vertScroll( 0 ), horScroll( 0 )
{
	vertScroll = new ScrollBar( n+"->vScr", this, 0, 0,
			ScrollBar::defaultWidth, h-ScrollBar::defaultWidth, ScrollBar::vertical, h, 0, 10, h );
	horScroll = new ScrollBar( n+"->hScr", this, 0, 0,
			w-ScrollBar::defaultWidth, ScrollBar::defaultWidth, ScrollBar::horizontal, w, 0, 10, w );
	
	setAbsCoordsOf( vertScroll, 1 );
	setAbsCoordsOf( horScroll, 1 );
	vertScroll -> move( w - ScrollBar::defaultWidth, 0 );
	horScroll -> move( 0, h - ScrollBar::defaultWidth );
	
	vertScroll -> anchors = Widget::anTop | Widget::anRight | Widget::anBottom;
	horScroll -> anchors = Widget::anLeft | Widget::anRight | Widget::anBottom;
	
	horScroll -> changeCallback = vertScroll -> changeCallback = scrollcb;
	horScroll -> info = vertScroll -> info = this;
	LOG(( "%li %li\n", horScroll -> left(), horScroll -> top() ));
}

void ScrollBox::scrollcb( Widget * w )
{
	ScrollBox * sb = reinterpret_cast<ScrollBox*>( w -> info );
	sb -> dockSpace( -sb -> horScroll -> getPosition(), -sb -> vertScroll -> getPosition(), sb -> dockSpace.width, sb -> dockSpace.height );
	if ( sb -> isVisible() ) Engine::Redraw();
}
	
void ScrollBox::onDraw()
{
	Widget::drawBevelledRect( 0, 0, width(), height(), 0 );
}
	
void ScrollBox::onResize()
{
	vertScroll -> setLongJump( height() );
	horScroll -> setLongJump( width() );
	onChildResized( 0 );
}

void ScrollBox::onChildResized( Widget * w )
{
	ScrollBar * v = 0, * h = 0;
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ){
		if ( *i == vertScroll ) v = static_cast<ScrollBar*>(*i); 
		else if ( *i == horScroll ) h = static_cast<ScrollBar*>(*i);
		else { ++i; continue; }
		WidgetList::iterator j = i; ++j;
		children.erase( i );
		i = j;
	}
	if ( v ) children.push_front( v );
	if ( h ) children.push_front( h );
	long minL = dockSpace.width, minT = dockSpace.height, maxR = 0, maxB = 0;
	#define LEFT ((*i) -> left())
	#define TOP ((*i) -> top())
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i )
		if ( (*i) != vertScroll && (*i) != horScroll && (*i) -> isVisible() ){
			minL = std::min( minL, LEFT );
			minT = std::min( minT, TOP );
			maxR = std::max( maxR, LEFT+(*i) -> width() );
			maxB = std::max( maxB, TOP+(*i) -> height() );
		}
	
	//~ if ( minL != 0 || minT != 0 )
		//~ for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i )
			//~ if ( (*i) != vertScroll && (*i) != horScroll ){
				//~ (*i) -> move( LEFT-minL, TOP-minT );
			//~ }
	
	#undef LEFT
	#undef TOP
	
	//~ dockSpace( dockSpace.offX, dockSpace.offY, maxR-minL, maxB-minT );
	dockSpace( dockSpace.offX, dockSpace.offY, maxR, maxB );
	//~ LOG(( "%s dockSpace : %li %li %li %li\n", name().c_str(), dockSpace.offX, dockSpace.offY, dockSpace.width, dockSpace.height ));
	long spw = width() - ScrollBar::defaultWidth;
	long sph = height() - ScrollBar::defaultWidth;
	if ( vertScroll ){
		vertScroll -> setLength( std::max( 1l, dockSpace.height-sph ) );
		vertScroll -> setLongJump( std::min( vertScroll -> getLength(), sph ) );
	}
	if ( horScroll ){
		horScroll -> setLength( std::max( 1l, dockSpace.width-spw ) );
		horScroll -> setLongJump( std::min( horScroll -> getLength(), spw ) );
	}
	
	
	if ( isVisible() ) Engine::Redraw();
}
