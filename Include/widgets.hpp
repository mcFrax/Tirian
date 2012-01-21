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
	private:
		std::string Caption;
		SDLFontRenderer * Font;
	protected:
		void onDraw();
	public:
		Label( const std::string& name, Widget* new_parent, long left, long top, long width, long height, SDLFontRenderer * font, const std::string& caption );
		
		const std::string& caption() const { return Caption; }
		void setCaption( const std::string& c ) { Caption = c; if ( isVisible() ) Engine::Redraw(); }
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
	public:
		enum FillType { original, stretch, fill, scale, tile };
	protected:
		Texture tex;
		void onDraw();
		FillType fillType;
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
	public:
		enum ScrollBarDirection
		{
			horizontal,
			vertical
		};
	private:
		Uint32 length;
		Uint32 pos;
		Uint32 shortjump;
		Uint32 longjump;
		ScrollBarDirection dir;
		static const Uint32 minSeekSize;
	protected:
		void onDraw();
	public:
		static const long defaultWidth;
		static const long defaultLongJump;
	public:
		ScrollBar( const std::string& name, Widget* new_parent, long left, long top, long width, long height, ScrollBarDirection direction, long length, long pos = 0, long shortjump = 1, long longjump = defaultLongJump );
	
		long getLength() { return length; }
		long getPosition() { return pos; }
		long getShortJump() { return shortjump; }
		long getLongJump() { return longjump; }
		ScrollBarDirection getDirection() { return dir; }
		
		void setLength( long l ) { length = l; if ( isVisible() ) Engine::Redraw(); }
		void setPosition( long p ) { pos = p; if ( isVisible() ) Engine::Redraw(); }
		void setShortJump( long j ) { shortjump = j; if ( isVisible() ) Engine::Redraw(); }
		void setLongJump( long j ) { longjump = j; if ( isVisible() ) Engine::Redraw(); }
		void setDirection( const ScrollBarDirection d ) { dir = d; if ( isVisible() ) Engine::Redraw(); }
};

//~ class List : public ClickableWidget, public KeyboardWidget
//~ {
	//~ private:
		//~ 
		//~ virtual void Click( const MouseButtonEvent& event );
	//~ protected:
	//~ public:
		//~ List( const std::string& name, DockWidget* new_parent, long left, long top, long width, long height );
//~ };


#endif //_WIDGETS_HPP_
