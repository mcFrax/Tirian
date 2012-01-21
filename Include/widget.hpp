#ifndef _WIDGET_HPP_
#define _WIDGET_HPP_

#include <list>
#include <string>

#include <texture.hpp>
#include <logging.hpp>
#include <except.hpp>
#include <interface.hpp>
#include <gl.hpp>


typedef int TAnchors;

class Widget;
class Window;
class Engine;

struct MouseMotionEvent;
struct MouseButtonEvent;


class Widget
{
	friend class Engine;
	friend class Window;
	//typedefs
	public:
		typedef std::list<Widget*> WidgetList;
		typedef void (*Callback)( Widget& );
		typedef void (*MotionCallback)( Widget&, const MouseMotionEvent& );
		typedef void (*ButtonCallback)( Widget&, const MouseButtonEvent& );
		typedef void (*KeyCallback)( Widget&, const SDL_keysym& );
		typedef void (*DrawBevelledRectFunc)( const float, const float, const float, const float, const bool );
		typedef void (*DrawButtonFunc)( const float, const float, const float, const float, const bool, const bool );
	private:
	//Podstawowe pola:
		const std::string Name;
		bool Visible;
		bool Enabled;
		Widget * Parent;
		WidgetList::iterator listIt;
		long x, y, w, h;
	public:
		TAnchors anchors;
	//Pola do dokowania:
	private:
		WidgetList children;
	protected:
		//współrzędne pola dokowania
		struct {
			long offX, offY, width, height;
			void operator()( long a, long b, long c, long d ) { offX=a; offY=b; width=c; height=d; }
		} dockSpace;
	//Pola obsługi kliknięć:
	protected:
		bool pressed;
		bool underPression;
		Uint32 lastClicked;
	//Pola statyczne focusów:
	private:
		static Widget* keyFocus;
		static Widget* mouseFocus;
		static Widget* underMouse;	//!< Ostatni widget, który odebrał jakikolwiek event myszy
	public:
		static Widget* getKeyFocus() { return keyFocus; }
		static Widget* getMouseFocus() { return mouseFocus; }
		static Widget* getUnderMouse() { return underMouse; }
	//pola i funkcje statyczne - uniwersalne metody rysowania
	public:
		static DrawBevelledRectFunc drawBevelledRect;
		static DrawButtonFunc drawButton;
		static void defaultDrawBevelledRect( const float left, const float top, const float width, const float height, const bool up );
		static void defaultDrawButton( const float left, const float top, const float width, const float height, const bool enabled, const bool up );
	//Konstruktory:
	protected:
		Widget( const std::string& name );
		Widget( const std::string& name, Widget* new_parent, long left, long top, long width, long height );
	//Wenętrzna obsługa eventów i hierarchii:
	private:
		void motion( const MouseMotionEvent& event );
		void button( const MouseButtonEvent& event );
		
		void key( const SDL_KeyboardEvent& event );
		
		void unparent();
	protected:
		void draw( long l, long t );
		virtual void resizeDockSpace( long l, long t, long w, long h );	//!< Wywoływane przez resize \u przed zmianą wielkości, argumenty są przekazane bezpośrednio.
	//Reimplementowalna bsługa eventów:
	protected:
		virtual void onMotion( const MouseMotionEvent& event ) {}
		virtual void onButton( const MouseButtonEvent& event ) {}
		
		virtual void onMouseEnter( const MouseMotionEvent& event ) {}
		virtual void onMouseEscape( const MouseMotionEvent& event ) {}
		
		virtual void onClick( const MouseButtonEvent& event ) {}
		virtual void onDoubleClick( const MouseButtonEvent& event ) {}
		
		virtual void onKey( const SDL_KeyboardEvent& event ) {}
		
		virtual void onResize() {}
		
		virtual void onVisible() {}
		virtual void onEnabled() {}
		
		virtual void onDraw() {}
		
		virtual void onChildResized( Widget* ) {}	//!<Argument 0 oznacza, że dziecko się odczepiło.
		virtual void onChildVisible( Widget* ) {}
	//Metody publiczne
	public:
		virtual ~Widget();
		
		void place( Widget* new_parent, long left, long top, long width, long height );
		
		bool isVisible() const { return Visible; }
		bool isEnabled() const { return Enabled; }
		void setVisible( bool new_value );
		void setEnabled( bool new_value );
		
		long left() const { return x; }
		long top() const { return y; }
		long width() const { return w; }
		long height() const { return h; }
		long absLeft() const;
		long absTop() const;
		
		void resize( long left, long right, long width, long height );
		
		const std::string& name() const { return Name; }
		
		MotionCallback motionCallback;
		MotionCallback mouseEnterCallback;
		MotionCallback mouseEscapeCallback;
		
		ButtonCallback buttonDownCallback;
		ButtonCallback buttonUpCallback;
		
		ButtonCallback clickCallback;
		ButtonCallback doubleClickCallback;
		
		KeyCallback keyDownCallback;
		KeyCallback keyUpCallback;
		//~ KeyCallback keyPressedCallback;
		
		Callback resizeCallback;
		Callback drawCallback;
		
		int tag;
};

class Frame : public Widget
{
	protected:
		virtual void onDraw();
	public:
		Frame( const std::string& );
		
		Callback onIdle;
};

class Window : protected Widget
{
	friend class Frame;
	friend class Engine;
	private:
	public:
		Window( long width, long height );
		
		Widget::width;
		Widget::height;
		
		void draw();
		
		void resize( const SDL_ResizeEvent& event );
		void idle();
		
		Widget::keyDownCallback;
		Widget::keyUpCallback;
		
		Callback idleCallback;
};

enum enumAnchors
{
	AN_LEFT = 1,
	AN_RIGHT = 2,
	AN_TOP = 4,
	AN_BOTTOM = 8
};

struct MouseMotionEvent
{
	Uint8 state;
	Sint32 x, y; 
	Sint16 xrel, yrel;
	MouseMotionEvent( const MouseMotionEvent& e, Uint16 offX, Uint16 offY )
	{
		*this = e;
		x -= offX; y -= offY;
	}
	MouseMotionEvent( const SDL_MouseMotionEvent& e )
		: state(e.state), x(e.x), y(e.y), xrel(e.xrel), yrel(e.yrel) {}
};

struct MouseButtonEvent
{  
	Uint8 state;
	Uint8 button;
	Sint32 x, y;
	MouseButtonEvent( const MouseButtonEvent& e, Uint16 offX, Uint16 offY )
	{
		*this = e;
		x -= offX; y -= offY;
	}
	MouseButtonEvent( const SDL_MouseButtonEvent& e )
		: state(e.state), button(e.button), x(e.x), y(e.y) {}
};

inline long Widget::absLeft() const
{
	long left = 0;
	for ( const Widget * i = this; i != 0; i = i -> Parent )
		left += i -> left();
	return left;
}

inline long Widget::absTop() const
{
	long top = 0;
	for ( const Widget * i = this; i != 0; i = i -> Parent )
		top += i -> top();
	return top;
}


#endif //_WIDGET_HPP_
