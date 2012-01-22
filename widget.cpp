#include <widget.hpp>
#include <engine.hpp>
#include <cstring>

static const Widget::Anchors defAnch = Widget::anLeft | Widget::anTop;
static const Uint32 doubleClickTime = 320;

unsigned long Widget::nextID = 1;
std::map<unsigned long, Widget*> Widget::IDmap;

Widget* Widget::keyFocus = 0;
Widget* Widget::mouseFocus = 0;
Widget* Widget::underMouse = 0;
Widget::DrawBevelledRectFunc Widget::drawBevelledRect = &Widget::defaultDrawBevelledRect;
Widget::DrawButtonFunc Widget::drawButton = &Widget::defaultDrawButton;


Widget::Widget( const std::string& name, Widget* new_parent, long X, long Y, long W, long H )
	:	Name(name), Visible(1), Enabled(1), Parent( this ), listIt(0),
		x( 0 ), y( 0 ), w( W ), h( H ), absCoords( 0 ),
		anchors( defAnch ), info( 0 ), ID( nextID++ ),
		pressed( 0 ), underPression( 0 ), lastClicked( 0 ), clickRepeat( 0 ),
		motionCallback(0), mouseEnterCallback( 0 ), mouseEscapeCallback( 0 ),
		buttonDownCallback(0), buttonUpCallback(0),
		clickCallback(0), doubleClickCallback(0), keyDownCallback(0),
		keyUpCallback(0), /*keyPressedCallback(0),*/ resizeCallback(0)
{
	//~ LOG(( "Creating %s\n", name.c_str() ));
	IDmap[ ID ] = this;
	if ( !new_parent ) throw MKRE( "Widget without parent!" );
	dockSpace( 0, 0, W, H );
	place( new_parent, X, Y, W, H );
	//~ LOG(( "%s constructed\n", name.c_str() ));
}

Widget* Widget::getPointer( unsigned long id )
{
	std::map<unsigned long, Widget*>::iterator it = IDmap.find( id );
	return ( it == IDmap.end() ) ? 0 : (*it).second;
}

Widget::~Widget()
{
	unparent();
	
	for ( WidgetList::iterator i = children.begin(), j; i != children.end(); i = j ){
		j = i; ++j;
		delete (*i);
	}
	
	IDmap.erase( IDmap.find( ID ) );
}

void Widget::unparent()
{
	if ( Parent != this ){
		Parent -> children.erase( listIt );
		Parent -> onChildResized( 0 );
		Parent = this;
	}
}

void Widget::place( Widget* new_parent, long left, long top, long width, long height )
{
	unparent();
	absCoords = 0;
	Parent = new_parent;
	//~ LOG(( "placing %s on %s\n", Name.c_str(), new_parent -> Name.c_str() ));
	if ( Parent != this ){
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
	if ( Parent != this ) Parent -> onChildVisible( this );
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

void Widget::move( long l, long t )
{
	x = l; y = t;
	onMove();
	
	if ( Parent != this ) Parent -> onChildResized( this );
	if ( Visible ) Engine::Redraw();
}

void Widget::resize( long l, long t, long wi, long hi )
{
	long diffx = -dockSpace.width;
	long diffy = -dockSpace.height;
	
	resizeDockSpace( l, t, wi, hi );
	
	diffx += dockSpace.width;
	diffy += dockSpace.height;
	
	long absDiffX = wi - w;
	long absDiffY = hi - h;
	
	x = l; y = t; w = std::max( wi, 0l ); h = std::max( hi, 0l );
	onResize();
	
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i ){
		int ix = (*i) -> left(), iy = (*i) -> top(), iw = (*i) -> width(), ih = (*i) -> height();
		bool c = 0;
		register long dx = ( (*i) -> absCoords ) ? absDiffX : diffx;
		register long dy = ( (*i) -> absCoords ) ? absDiffY : diffy;
		if ( (*i) -> anchors & anRight ){
			c = 1;
			if ( (*i) -> anchors & anLeft )
				iw += dx;
			else
				ix += dx;
		}
		if ( (*i) -> anchors & anBottom ){
			c = 1;
			if ( (*i) -> anchors & anTop )
				ih += dy;
			else
				iy += dy;
		}
		if ( c ) (*i) -> resize( ix, iy, iw, ih );
	}
	
	if ( Parent != this ) Parent -> onChildResized( this );
	if ( Visible ) Engine::Redraw();
}

void Widget::motion( const MouseMotionEvent& e )
{
	bool overChild = 0;
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i ){
		register long LEFT = ((*i) -> absCoords) ? (*i) -> left() :((*i) -> left() + dockSpace.offX);
		register long TOP = ((*i) -> absCoords) ? (*i) -> top() : ((*i) -> top() + dockSpace.offY);
		if ( LEFT <= e.x && TOP <= e.y && LEFT+(*i)->width() > e.x && TOP+(*i)->height() > e.y )
			if ( (*i) -> Visible && (*i) -> Enabled ){
				overChild = 1;
				if ( mouseFocus != this ){
					(*i) -> motion( MouseMotionEvent( e, LEFT, TOP ) );
					return;
				}
			}
	}
	
	onMotion( e );
	if ( motionCallback ) motionCallback( this, e );
	
	bool b = underPression;
	underPression = ( e.x >= 0 && e.y >= 0 && e.x < w && e.y < h ) && pressed && !overChild;
	if ( b != underPression ) Engine::Redraw();
	
	if ( underMouse != this ){
		underMouse -> onMouseEscape( e );
		if ( underMouse -> mouseEscapeCallback ) underMouse -> mouseEscapeCallback( underMouse, e );
		underMouse = this;
		onMouseEnter( e );
		if ( mouseEnterCallback ) mouseEnterCallback( this, e );
	}
}


void Widget::button( const MouseButtonEvent& e )
{
	bool overChild = 0;
	for ( WidgetList::iterator i = children.begin(); i != children.end(); ++i ){
		register long LEFT = ((*i) -> absCoords) ? (*i) -> left() :((*i) -> left() + dockSpace.offX);
		register long TOP = ((*i) -> absCoords) ? (*i) -> top() : ((*i) -> top() + dockSpace.offY);
		if ( LEFT <= e.x && TOP <= e.y && LEFT+(*i)->width() > e.x && TOP+(*i)->height() > e.y )
			if ( (*i) -> Visible && (*i) -> Enabled ){
				overChild = 1;
				if ( mouseFocus != this ){
					(*i) -> button( MouseButtonEvent( e, LEFT, TOP ) );
					return;
				}
			}
	}
	#undef LEFT
	#undef TOP
	
	if ( e.state == SDL_RELEASED || !overChild ){
		onButton( e );
		if ( e.state == SDL_RELEASED ){
			if ( buttonUpCallback ) buttonUpCallback( this, e );
		} else {
			if ( buttonDownCallback ) buttonDownCallback( this, e );
		}
	}
	
	if ( e.button == SDL_BUTTON_LEFT ){
		if( e.state == SDL_PRESSED ){
			pressed = 1;
			underPression = 1;
			mouseFocus = this;
			if ( clickRepeat ){
				Engine::pushEvent( new ClickingEvent( SDL_GetTicks()+SDL_DEFAULT_REPEAT_DELAY, ID ) );
			}
			Engine::Redraw();
		} else {
			if ( pressed ){
				mouseFocus = 0;
				Engine::Redraw();
				if ( e.x >= 0 && e.y >= 0 && e.x < w && e.y < h ){
					onClick( e );
					if ( clickCallback ) clickCallback( this, e );
					if ( SDL_GetTicks() - lastClicked <= doubleClickTime ){
						onDoubleClick( e );
						if ( doubleClickCallback ) doubleClickCallback( this, e );
						//~ lastClicked = 0;
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

void Widget::ClickingEvent::execute()
{
	register Widget* ptr = Widget::getPointer( scr );
	if ( !ptr ) return;
	ptr -> clickEvent( executionTime );
}

void Widget::clickEvent( const Uint32 t )
{
	if ( !clickRepeat || !underPression ) return;
	SDL_MouseButtonEvent e;
	e.state = SDL_PRESSED;
	e.x = Mouse::getX(); e.y = Mouse::getY();
	e.button = SDL_BUTTON_LEFT;
	onClick( e );
	if ( clickCallback ) clickCallback( this, e );
	Engine::pushEvent( new ClickingEvent( t + SDL_DEFAULT_REPEAT_INTERVAL, ID ) );
}

void Widget::key( const SDL_KeyboardEvent& e )
{
	if ( e.state == SDL_PRESSED ){
		if ( keyDownCallback ) keyDownCallback( this, e.keysym );
	} else {
		if ( keyUpCallback ) keyUpCallback( this, e.keysym );
		//~ if ( onKeyPressed ) (*onKeyPressed)( this, e.keysym );
	}
}

void Widget::setAbsCoordsOf( Widget* w, bool abs )
{
	if ( w -> Parent != this ) throw RuntimeError( "Illegal use of Widget::setAbsCoordsOf()" ); // alternatywnie - return;
	w -> absCoords = abs;
}

void Widget::draw( long l, long t )
{
	for ( GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError() )
		LOG(( "%s:%i: OpenGL error : %s\n( %s )\n", __FILE__, __LINE__, glerrorstring[ err - 0x0500 ], name().c_str() ));
		
	onDraw();
	
	for ( WidgetList::reverse_iterator i = children.rbegin(); i != children.rend(); ++i ){
		register long LEFT = ((*i) -> absCoords) ? (*i) -> left() :((*i) -> left() + dockSpace.offX);
		register long TOP = ((*i) -> absCoords) ? (*i) -> top() : ((*i) -> top() + dockSpace.offY);
		if ( !(*i) -> Visible ) continue;
	GLTERRORCHECK;
		long x1 = std::max( 0l, LEFT ), y1 = std::max( 0l, TOP );
		long w1 = std::min( LEFT+(*i) -> width(), width() ) - x1, h1 = std::min( TOP+(*i) -> height(), height() ) - y1;
		if ( w1 <= 0 || h1 <= 0 ) continue;
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef( LEFT, TOP, 0.0f );
		glScissor( l+x1, t+y1, w1, h1 );
		//~ glScissor( l+LEFT, Engine::window().height() - ( t+TOP+(*i) -> height() ), (*i) -> width(), (*i) -> height() );
		(*i) -> draw( l+LEFT, t+TOP );
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
	GLTERRORCHECK;
	}
}

void Widget::defaultDrawBevelledRect( const float l, const float t, const float w, const float h, const bool up )
{
	GLTERRORCHECK;
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
	
	//~ glColor3f( col[0], col[1], col[2] );
	//~ glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+8 );
	
	if ( up ){
		glColor3f( bright[0], bright[1], bright[2] );
	} else {
		glColor3f( dark[0], dark[1], dark[2] );
	}
	
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+12 );
	GLTERRORCHECK;
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
	GLTERRORCHECK;
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
	
	#warning burdel się zrobił trochę
	glEnable( GL_COLOR_MATERIAL );
	glDisable( GL_LIGHTING );
	glDisable( GL_CULL_FACE );
	
	glColorPointer( 4, GL_FLOAT, 0, &ColorArray[0].r );
	glVertexPointer( 2, GL_FLOAT, 0, vertex );
	//~ glColorPointer( 3, GL_FLOAT, 0, &ColorArray[0].r );

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	#warning WTF!?!
	glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, indices );
	
	glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices );
	
	ColorArray[1] = br; ColorArray[3] = br;
	
	//~ glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, indices+8 );
	//~ 
	//~ glDrawElements( GL_QUADS, 8, GL_UNSIGNED_BYTE, indices+12 );
	glDrawElements( GL_QUADS, 12, GL_UNSIGNED_BYTE, indices+8 );
	
	glDisableClientState(GL_COLOR_ARRAY);
	GLTERRORCHECK;
}


Frame::Frame( const std::string& name )
	:	Widget( name, &Engine::window(), 0, 0, Engine::window().width(), Engine::window().height() )
{
	anchors = Widget::anLeft | Widget::anRight | Widget::anTop | Widget::anBottom;
	setVisible( 0 );
}

void Frame::onDraw()
{
	glClear( GL_COLOR_BUFFER_BIT );
}


Window::Window( long width, long height )
	: Widget( std::string( "Main Window" ), this, 0, 0, width, height ), idleCallback( 0 )
{
	underMouse = this;
}

void Window::draw()
{
	GltSetWindow attr( width(), height() );
	
	GLTERRORCHECK;
	glClearColor( Engine::backgroundColor[0], Engine::backgroundColor[1], Engine::backgroundColor[2], 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	GLTERRORCHECK;
	
	Widget::draw( 0l, 0l );
	GLTERRORCHECK;
	
	glFinish();
	
	SDL_GL_SwapBuffers();
}

void Window::resize( const SDL_ResizeEvent& e )
{
	Widget::resize( 0, 0, e.w, e.h );
}

void Window::idle()
{
	if ( idleCallback ) idleCallback( this );
}



