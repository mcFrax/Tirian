#ifndef _FONTRENDERER_HPP_
#define _FONTRENDERER_HPP_

#include <except.hpp>
#include <map>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <texture.hpp>

/*!
 * \file fontrenderer.hpp
 * \brief Implementacja klasy SDLFontRenderer
 */


class FontRenderer
{
    protected:
		int h;
		int ls;
		TTF_Font * font;
		bool blend;
		bool mipmaps;
		float color[4];
		FontRenderer();
	public:
		virtual ~FontRenderer() {}
		virtual void set( TTF_Font * _font, const bool useMipmaps,
                const bool blending ) = 0;
		void setBlending( bool blending ) { blend = blending; }
		virtual void render( std::string s, float x, float y, float height ) = 0;
		void render( std::string s, float x, float y ) { render( s, x, y, h ); }
		int height() { return h; }
		int lineSkip() { return ls; }
		TTF_Font * getFont() { return font; }
};

//! Klasa do renderowania fontów przez OpenGL z wykorzystaniem SDL
/*!
 * Udostępnia łatwe renderowanie czcionek TrueType (TTF) z użyciem OpenGL'a, przez UTF-8.
 */
class SDLFontRenderer : public FontRenderer
{
	private:
		std::map<char, Texture> tex;
		Texture * get( char );
		Texture * make( char );
		struct vert { float x, y; };
	public:
		SDLFontRenderer();
		SDLFontRenderer( TTF_Font * _font, const bool useMipmaps, const bool blending );
		void set( TTF_Font * _font, const bool useMipmaps, const bool blending );
		void setGLflags();
		void render( std::string s, float x, float y );
		void render( std::string s, float x, float y, float height );
};

#endif //_FONTRENDERER_HPP_
