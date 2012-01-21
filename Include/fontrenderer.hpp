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


//! Klasa do renderowania fontów przez OpenGL z wykorzystaniem SDL
/*!
 * Udostępnia łatwe renderowanie czcionek TrueType (TTF) z użyciem OpenGL'a, przez UTF-8.
 */
class SDLFontRenderer
{
	private:
		//~ std::map<wchar_t, Texture> tex;
		std::map<char, Texture> tex;
		int h;
		int ls;
		bool blend;
		bool mipmaps;
		TTF_Font * font;
		float color[4];
		//~ Texture& get( wchar_t );
		//~ Texture& make( wchar_t );
		Texture * get( char );
		Texture * make( char );
		struct vert { float x, y; };
	public:
		SDLFontRenderer();
		SDLFontRenderer( TTF_Font * _font, const bool useMipmaps, const bool blending );
		void set( TTF_Font * _font, const bool useMipmaps, const bool blending );
		void setBlending( bool blending ) { blend = blending; }
		void setGLflags();
		//~ void render( std::wstring s, float x, float y, float height );
		void render( std::string s, float x, float y );
		void render( std::string s, float x, float y, float height );
		//~ void render( std::string s, float x, float y, float height );
		int height() { return h; }
		int lineSkip() { return ls; }
		TTF_Font * getFont() { return font; }
};

#endif //_FONTRENDERER_HPP_
