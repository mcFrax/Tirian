#ifndef _WIDGETS_HPP_
#define _WIDGETS_HPP_

#include <widget.hpp>
#include <engine.hpp>
#include <string>
#include <vector>
#include <list>

#include <fontrenderer.hpp>

class Label : public Widget
{
	public:
		enum HorizontalTextPos{
			hpLeft, hpCenter, hpRight
		};
		enum VerticalTextPos{
			vpTop, vpCenter, vpBottom
		};
	private:
		std::string Caption;
		SDLFontRenderer * Font;
		VerticalTextPos vertTextPos;
		HorizontalTextPos horTextPos;
	protected:
		void onDraw();
	public:
		Label( const std::string& name, Widget* new_parent, long left, long top, long width, long height, SDLFontRenderer * font, const std::string& caption );
		
		const std::string& caption() const { return Caption; }
		void setCaption( const std::string& c ) { Caption = c; if ( isVisible() ) Engine::Redraw(); }
		
		VerticalTextPos getVertTextPos() { return vertTextPos; }
		HorizontalTextPos getHorTextPos() { return horTextPos; }
		void setVertTextPos( VerticalTextPos vp ) { vertTextPos = vp; if ( isVisible() ) Engine::Redraw(); }
		void setHorTextPos( HorizontalTextPos hp ) { horTextPos = hp; if ( isVisible() ) Engine::Redraw(); }
		
		SDLFontRenderer * getFont() { return Font; }
		void setFont( SDLFontRenderer * f ) { Font = f; if ( isVisible() ) Engine::Redraw(); }
};

class Button : public Widget
{
	private:
		Label* label;
	protected:
		void onEnabled();
		void onDraw();
		void onMotion( const MouseMotionEvent& event );
		void onButton( const MouseButtonEvent& event );
	public:
		Button( const std::string& name, Widget* new_parent, long left, long top, long width, long height );
		
		const std::string& caption() const { return label -> caption(); }
		void setCaption( const std::string& c ) { label -> setCaption( c ); }
		
		static const long defaultHeight;
};

class Image : public Widget
{
	protected:
		Texture tex;
		void onDraw();
	public:
		Image( const std::string& name, Widget* new_parent, long left, long top, long width, long height, const Texture& pat = Texture() );
		
		const Texture& texture() { return tex; }
		
		void setTexture( const Texture& pat );
		
		long realWidth() { return ( tex.width() && tex.height() ) ? ( (float(width())/height() < float(tex.width())/tex.height()) ? width() : tex.width() * height()/tex.height() ) : 0; }
		long realHeight() { return ( tex.width() && tex.height() ) ? ( (float(width())/height() < float(tex.width())/tex.height()) ? tex.height() * width()/tex.width() : height() ) : 0; }
};

class AnimatedImage : public Image
{
	private:
		std::vector<Texture> texes;
		Uint32 lastTime;
		Uint32 last;
		Uint32 Interval;
	protected:
		void onDraw();
	public:
		AnimatedImage( const std::string& name, Widget* new_parent, long left, long top, long width, long height, const Texture& pat = Texture(), Uint32 frames = 1, Uint32 interval = 0 );
		
		void setTexture( const Texture& pat, Uint32 frames = 1 );
		
		Uint32& interval() { return Interval; }
		const Uint32& interval() const { return Interval; }
		
		void reset();
};

class ScrollBar : public Widget
{
	friend class SeekBar;
	public:
		enum Direction
		{
			horizontal,
			vertical
		};
	private:
		class SeekBar : public Widget
		{
			public:
				void onMotion( const MouseMotionEvent& mme );
				void onButton( const MouseButtonEvent& mbe );
				void onDraw();
				ScrollBar* scroll;
				long mx, my;
				SeekBar( const std::string& name, Widget* new_parent, long left, long top, long width, long height, ScrollBar * scr );
		};
		class ArrowButton : public Button
		{
			protected:
				void onDraw();
			public:
				char dir; //L=0, R=1, U=2, D=3
				ArrowButton( const std::string& n, Widget* np, long l, long t, long w, long h, char d )
					:	Button( n, np, l, t, w, h ), dir( d ) { clickRepeat = 1; }
				
		};
		ArrowButton* but1;
		ArrowButton* but2;
		Direction dir;
		Uint32 length;
		Uint32 pos;
		Uint32 shortjump;
		Uint32 longjump;
		SeekBar* seekBar;
		static const long minSeekbarSize;
		void seekBarMoved();
		void setSeekBarSize();
		void but1cb();
		void but2cb();
		static void but1cbW( Widget *, const MouseButtonEvent& );
		static void but2cbW( Widget *, const MouseButtonEvent& );
	protected:
		void onResize();
		void onDraw();
		void onClick( const MouseButtonEvent& event );
	public:
		static const long defaultWidth;
		static const long defaultLongJump;
	public:
		ScrollBar( const std::string& name, Widget* new_parent, long left, long top, long width, long height, Direction direction, Uint32 length, Uint32 pos = 0, Uint32 shortjump = 1, Uint32 longjump = defaultLongJump );
		
		long clickInterval; //value <= 0 means no repeating, ddefault 0;
	
		long getLength() { return length; }
		long getPosition() { return pos; }
		long getShortJump() { return shortjump; }
		long getLongJump() { return longjump; }
		Direction getDirection() { return dir; }
		
		void setLength( Uint32 l );
		void setPosition( Uint32 p );
		void setShortJump( Uint32 j );
		void setLongJump( Uint32 j );
		void setDirection( const Direction d );
		
		Callback changeCallback;
};

class ScrollBox : public Widget
{
	private:
		ScrollBar * vertScroll;
		ScrollBar * horScroll;
		static void scrollcb( Widget * );
	protected:
		void resizeDockSpace( long, long, long, long ) {}
		void onDraw();
		void onResize();
		void onChildResized( Widget * );
		void onChildVisibled( Widget * w ) { onChildResized( (w -> isVisible()) ? w : 0 ); }
	public:
		ScrollBox( const std::string& name, Widget* new_parent, long left, long top, long width, long height );
		
		//~ long getScrollX();
		//~ long getScrollY();
		//~ 
		//~ void setScrollX();
		//~ void setScrollY();
};


#endif //_WIDGETS_HPP_
