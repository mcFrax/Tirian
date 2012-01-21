#include <widget.hpp>
#include <engine.hpp>
#include <cstring>

static const TAnchors defAnch = AN_LEFT | AN_TOP;
static const Uint32 doubleClickTime = 320;

Widget* Widget::keyFocus = 0;
Widget* Widget::mouseFocus = 0;
Widget* Widget::underMouse = 0;
Widget::DrawBevelledRectFunc Widget::drawBevelledRect = &Widget::defaultDrawBevelledRect;
Widget::DrawButtonFunc Widget::drawButton = &Widget::defaultDrawButton;

Widget::Widget( const std::string& name )
	:	Name(name), Visible(0), Enabled(0), Parent( 0 ), listIt(0),
		x( 0 ), y( 0 ), w( 0 ), h( 0 ), anchors( defAnch ),
		pressed( 0 ), underPression( 0 ), lastClicked( 0 ),
		motionCallback(0), mouseEnterCallback( 0 ), mouseEscapeCallback( 0 ),
		buttonDownCallback(0), buttonUpCallback(0),
		clickCallback(0), doubleClickCallback(0), keyDownCallback(0),
		keyUpCallback(0), /*keyPressedCallback(0),*/ resizeCallback(0),
		drawCallback(0)
{
	dockSpace( 0, 0, 0, 0 );
	place( 0, 0, 0, 0, 0 );
}

Widget::Widget( const std::string& name, Widget* new_parent, long left, long top, long width, long height )
	:	Name(name), Visible(1), Enabled(1), Parent( 0 ), listIt(0),
		x( 0 ), y( 0 ), w( width ), h( height ), anchors( defAnch ),
		pressed( 0 ), underPression( 0 ), lastClicked( 0 ),
		motionCallback(0), mouseEnterCallback( 0 ), mouseEscapeCallback( 0 ),
		buttonDownCallback(0), buttonUpCallback(0),
		clickCallback(0), doubleClickCallback(0), keyDownCallback(0),
		keyUpCallback(0), /*keyPressedCallback(0),*/ resizeCallback(0),
		drawCallback(0)
{
	dockSpace( 0, 0, width, height );
	place( new_parent, left, top, width, height );
}

Widget::~Widget()
{
	unparent();
	
	for ( WidgetList::iterator i = children.begin(), j; i != children.end(); i = j ){
		j = i; ++j;
		delete (*i);
	}
}

void Widget::unparent()
{
	if ( Parent ){
		Parent -> children.erase( listIt );
		Parent -> onChildResized( 0 );
		Parent = 0;
	}
}

void Widget::place( Widget* new_parent, long left, long top, long width, long height )
{
	unparent();
	Parent = new_parent;
	if ( Parent ){
		Parent -> children.push_front( this );
		listIt = Parent -> children.begin();
	}
	resize( left, top, width, height );
}

void Widget::setVisible( bool new_value )
{
	if ( Visible == new_value ) return;
	Visible = new_value;
	onVisible();
	if ( Parent ) Parent -> onChildVisible( this );
	if ( Visible ) Engine::Redraw();
}

void Widget::setEnabled( bool new_value )
{
	if ( Enabled == new_value ) return;
	Enabled = new_value;
	onEnabled();
}

void Widget::resizeDockSpace( long l, long t, long wi, long hi )
{
	dockSpace( 0, 0, std::max( wi, 0l ), std::max( hi, 0l ) );
}

void Widget::resize( long l, long t, long wi, long hi )
{
	long diffx = -dockSpace.width;
	long diffy = -dockSpace.height;
	
	resizeDockSpace( l, t, wi, hi );
	
	diffx += dockSpace.width;
	diffy += dockSpace.height;
	
	x = l; y = t; w = std::max( wi, 0l ); h = std::max( hi, 0l );
	onResize();
	
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i ){
		int ix = (*i) -> left(), iy = (*i) -> top(), iw = (*i) -> width(), ih = (*i) -> height();
		bool c = 0;
		if ( (*i) -> anchors & AN_RIGHT ){
			c = 1;
			if ( (*i) -> anchors & AN_LEFT )
				iw += diffx;
			else
				ix += diffx;
		}
		if ( (*i) -> anchors & AN_BOTTOM ){
			c = 1;
			if ( (*i) -> anchors & AN_TOP )
				ih += diffy;
			else
				iy += diffy;
		}
		if ( c ) (*i) -> resize( ix, iy, iw, ih );
	}
	
	if ( Parent ) Parent -> onChildResized( this );
	if ( Visible ) Engine::Redraw();
}

void Widget::motion( const MouseMotionEvent& e )
{
	#define LEFT ((*i) -> left() + dockSpace.offX)
	#define TOP ((*i) -> top() + dockSpace.offY)
	bool overChild = 0;
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i )
		if ( LEFT <= e.x && TOP <= e.y && LEFT+(*i)->width() > e.x && TOP+(*i)->height() > e.y )
			if ( (*i) -> Visible && (*i) -> Enabled ){
				overChild = 1;
				if ( mouseFocus != this ){
					(*i) -> motion( MouseMotionEvent( e, LEFT, TOP ) );
					return;
				}
			}
	#undef LEFT
	#undef TOP
	
	onMotion( e );
	if ( motionCallback ) motionCallback( *this, e );
	
	bool b = underPression;
	underPression = ( e.x >= 0 && e.y >= 0 && e.x < w && e.y < h ) && pressed && !overChild;
	if ( b != underPression ) Engine::Redraw();
	
	if ( underMouse != this ){
		underMouse -> onMouseEscape( e );
		if ( underMouse -> mouseEscapeCallback ) underMouse -> mouseEscapeCallback( *underMouse, e );
		underMouse = this;
		onMouseEnter( e );
		if ( mouseEnterCallback ) mouseEnterCallback( *this, e );
	}
}


void Widget::button( const MouseButtonEvent& e )
{
	#define LEFT ((*i) -> left() + dockSpace.offX)
	#define TOP ((*i) -> top() + dockSpace.offY)
	bool overChild = 0;
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i )
		if ( LEFT <= e.x && TOP <= e.y && LEFT+(*i)->width() > e.x && TOP+(*i)->height() > e.y )
			if ( (*i) -> Visible && (*i) -> Enabled ){
				overChild = 1;
				if ( mouseFocus != this ){
					(*i) -> button( MouseButtonEvent( e, LEFT, TOP ) );
					return;
				}
			}
	#undef LEFT
	#undef TOP
	
	if ( e.state == SDL_RELEASED || !overChild ){
		onButton( e );
		if ( e.state == SDL_RELEASED ){
			if ( buttonUpCallback ) buttonUpCallback( *this, e );
		} else {
			if ( buttonDownCallback ) buttonDownCallback( *this, e );
		}
	}
	
	if ( e.button == SDL_BUTTON_LEFT ){
		if( e.state == SDL_PRESSED ){
			pressed = 1;
			underPression = 1;
			mouseFocus = this;
			Engine::Redraw();
		} else {
			if ( pressed ){
				mouseFocus = 0;
				Engine::Redraw();
				if ( e.x >= 0 && e.y >= 0 && e.x < w && e.y < h ){
					onClick( e );
					if ( clickCallback ) clickCallback( *this, e );
					if ( SDL_GetTicks() - lastClicked <= doubleClickTime ){
						onDoubleClick( e );
						if ( doubleClickCallback ) doubleClickCallback( *this, e );
						lastClicked = 0;
					} else {
						lastClicked = SDL_GetTicks();
					}
				}
			}
			pressed = 0;
			underPression = 0;
		}
	}
}

void Widget::key( const SDL_KeyboardEvent& e )
{
	if ( e.state == SDL_PRESSED ){
		if ( keyDownCallback ) keyDownCallback( *this, e.keysym );
	} else {
		if ( keyUpCallback ) keyUpCallback( *this, e.keysym );
		//~ if ( onKeyPressed ) (*onKeyPressed)( *this, e.keysym );
	}
}

void Widget::draw( long l, long t )
{
	onDraw();
	if ( drawCallback ) drawCallback( *this );
	
	#define LEFT ((*i) -> left() + dockSpace.offX)
	#define TOP ((*i) -> top() + dockSpace.offY)
	for ( WidgetList::reverse_iterator i = children.rbegin(); i != children.rend(); ++i ){
		if ( !(*i) -> Visible ) continue;
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef( LEFT, TOP, 0.0f );
		glScissor( l+LEFT, Engine::window().height() - ( t+TOP+(*i) -> height() ), (*i) -> width(), (*i) -> height() );
		(*i) -> draw( l+LEFT, t+TOP );
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
	#undef LEFT
	#undef TOP
}

void Widget::defaultDrawBevelledRect( const float l, const float t, const float w, const float h, const bool up )
{
	float vertex[8][2] =
	{
		{ l,	t },
		{ l+w,	t },
		{ l+w,	t+h },
		{ l,	t+h },
		{ l+Engine::bevelWidth,		t+Engine::bevelWidth },
		{ l+w-Engine::bevelWidth,	t+Engine::bevelWidth },
		{ l+w-Engine::bevelWidth,	t+h-Engine::bevelWidth },
		{ l+Engine::bevelWidth,		t+h-Engine::bevelWidth }
	};
	
	static const Uint8 indices[20] = { 0, 1, 5, 4, 3, 0, 4, 7, 4, 5, 6, 7, 1, 2, 6, 5, 2, 3, 7, 6 };
	
	float * col = Engine::backgroundColor;
	float dark[3] =
	{
		col[0] - Engine::bevelColorDiff,
		col[1] - Engine::bevelColorDiff,
		col[2] - Engine::bevelColorDiff
	};
	float bright[3] =
	{
		col[0] + Engine::bevelColorDiff,
		col[1] + Engine::bevelColorDiff,
		col[2] + Engine::bevelColorDiff
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	glVertexPointer( 2, GL_FLOAT, 0, vertex );
	
	if ( up ){
		glColor3f( bright[0], bright[1], bright[2] );
	} else {
		glColor3f( dark[0], dark[1], dark[2] );
	}
	
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices );
	
	glColor3f( col[0], col[1], col[2] );
	
	//~ glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+8 );
	
	if ( up ){
		glColor3f( bright[0], bright[1], bright[2] );
	} else {
		glColor3f( dark[0], dark[1], dark[2] );
	}
	
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+12 );
}

struct Color3f
{
	float r, g, b;
	Color3f() : r(0), g(0), b(0) {}
	Color3f( float * ptr ) : r(ptr[0]), g(ptr[1]), b(ptr[2]) {}
	Color3f( float * ptr, const float diff ) : r(ptr[0]+diff), g(ptr[1]+diff), b(ptr[2]+diff) {}
	Color3f( float R, float G, float B ) : r(R), g(G), b(B) {}
	void set() { glColor3fv( &r ); }
	void copyTo( float * ptr ) { memcpy( ptr, this, sizeof( Color3f ) ); }
	operator float*() { return &r; }
};

struct Color4f
{
	float r, g, b, a;
	Color4f() : r(0), g(0), b(0), a(1) {}
	Color4f( float * ptr ) : r(ptr[0]), g(ptr[1]), b(ptr[2]), a(1) {}
	Color4f( float * ptr, const float diff ) : r(ptr[0]+diff), g(ptr[1]+diff), b(ptr[2]+diff), a(1) {}
	Color4f( float R, float G, float B ) : r(R), g(G), b(B), a(1) {}
	void set() { glColor4fv( &r ); }
	void copyTo( float * ptr ) { memcpy( ptr, this, sizeof( Color3f ) ); }
	operator float*() { return &r; }
};

void Widget::defaultDrawButton( const float l, const float t, const float w, const float h, const bool enabled, const bool up )
{
	Color4f col( enabled ? Engine::activeButtonColor : Engine::nonActiveButtonColor );
	Color4f lt( col, up ? +Engine::bevelColorDiff : -Engine::bevelColorDiff );
	Color4f br( col, up ? -Engine::bevelColorDiff : +Engine::bevelColorDiff );
	
	//~ Color3f ColorArray[8] = { lt, lt, br, lt, col, col, col, col };
	Color4f ColorArray[8] = { lt, lt, br, lt, col, col, col, col };
	
	float vertex[8][2] =
	{
		{ l,	t },
		{ l+w,	t },
		{ l+w,	t+h },
		{ l,	t+h },
		{ l+Engine::buttonBevelWidth,		t+Engine::buttonBevelWidth },
		{ l+w-Engine::buttonBevelWidth,	t+Engine::buttonBevelWidth },
		{ l+w-Engine::buttonBevelWidth,	t+h-Engine::buttonBevelWidth },
		{ l+Engine::buttonBevelWidth,		t+h-Engine::buttonBevelWidth }
	};
	
	static const Uint8 indices[20] = { 0, 1, 5, 4, 3, 0, 4, 7, 4, 5, 6, 7, 1, 2, 6, 5, 2, 3, 7, 6 };
	
	glColorPointer( 4, GL_FLOAT, 0, &ColorArray[0].r );
	glVertexPointer( 2, GL_FLOAT, 0, vertex );
	//~ glColorPointer( 3, GL_FLOAT, 0, &ColorArray[0].r );

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	#warning WTF!?!
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices );
	
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices );
	
	ColorArray[1] = br; ColorArray[3] = br;
	
	//~ glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+8 );
	//~ 
	//~ glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+12 );
	glDrawElements( GL_QUADS, 12, GL_UNSIGNED_BYTE, indices+8 );
	
	glDisableClientState(GL_COLOR_ARRAY);
}

#if 0
void Widget::defaultDrawButton( const float l, const float t, const float w, const float h, const bool enabled, const bool up )
{	
	float * col = enabled ? Engine::activeButtonColor : Engine::nonActiveButtonColor;
	float dark[3] =
	{
		col[0] - Engine::bevelColorDiff,
		col[1] - Engine::bevelColorDiff,
		col[2] - Engine::bevelColorDiff
	};
	Color3f dark
	float bright[3] =
	{
		col[0] + Engine::bevelColorDiff,
		col[1] + Engine::bevelColorDiff,
		col[2] + Engine::bevelColorDiff
	};
	
	float *lt, *br;
	if ( up ){
		lt = bright;
		br = dark;
	} else {
		lt = dark;
		br = bright;
	}
	
	float vertex[8][2] =
	{
		{ l,	t },
		{ l+w,	t },
		{ l+w,	t+h },
		{ l,	t+h },
		{ l+Engine::buttonBevelWidth,		t+Engine::buttonBevelWidth },
		{ l+w-Engine::buttonBevelWidth,	t+Engine::buttonBevelWidth },
		{ l+w-Engine::buttonBevelWidth,	t+h-Engine::buttonBevelWidth },
		{ l+Engine::buttonBevelWidth,		t+h-Engine::buttonBevelWidth }
	};
	
	float color[8][3] =
	{
		{ lt[0], lt[1], lt[2] },
		{ lt[0], lt[1], lt[2] },
		{ br[0], br[1], br[2] },
		{ lt[0], lt[1], lt[2] },
		{ col[0], col[1], col[2] },
		{ col[0], col[1], col[2] },
		{ col[0], col[1], col[2] },
		{ col[0], col[1], col[2] }
	};
	
	static const Uint8 indices[20] = { 0, 1, 5, 4, 3, 0, 4, 7, 4, 5, 6, 7, 1, 2, 6, 5, 2, 3, 7, 6 };

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glVertexPointer( 2, GL_FLOAT, 0, vertex );
	glColorPointer( 3, GL_FLOAT, 0, &color[0][0] );
	
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices );
	
	memcpy( &color[1][0], br, 3*sizeof( float ) );
	memcpy( &color[3][0], br, 3*sizeof( float ) );
	
	glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+8 );
	
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+12 );
	
	//~ glDisableClientState(GL_VERTEX_ARRAY);
	//~ glDisableClientState(GL_COLOR_ARRAY);
}
#endif



Frame::Frame( const std::string& name )
	:	Widget( name, &Engine::window(), 0, 0, Engine::window().width(), Engine::window().height() )
{
	anchors = AN_LEFT | AN_RIGHT | AN_TOP | AN_BOTTOM;
	setVisible( 0 );
}

void Frame::onDraw()
{
	glClear( GL_COLOR_BUFFER_BIT );
}


Window::Window( long width, long height )
	: Widget( std::string( "Main Window" ), 0, 0, 0, width, height ), idleCallback( 0 )
{
	underMouse = this;
}

void Window::draw()
{
	GltSetWindow attr( width(), height() );
	
	glClearColor( Engine::backgroundColor[0], Engine::backgroundColor[1], Engine::backgroundColor[2], 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	Widget::draw( 0l, 0l );
	
	glFinish();
	
	SDL_GL_SwapBuffers();
}

void Window::resize( const SDL_ResizeEvent& e )
{
	Widget::resize( 0, 0, e.w, e.h );
}

void Window::idle()
{
	if ( idleCallback ) idleCallback( *this );
}



