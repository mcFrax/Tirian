#ifndef _MONO_FONTRENDERER_HPP_
#define _MONO_FONTRENDERER_HPP_

#include <fontrenderer.hpp>

//! Klasa do renderowania fontów przez OpenGL z wykorzystaniem SDL
/*!
 * Udostępnia łatwe renderowanie czcionek TrueType (TTF) z użyciem OpenGL'a, przez UTF-8.
 */
class MonoFontRenderer : public FontRenderer
{
	private:
		std::map<char, Texture> texmap;
		uint displayList;
		Texture tex;
		Texture * get( char );
		Texture * make( char );
		struct vert { float x, y; };
		void createDisplays();
		void render_special( std::string s );
		void render_standard( std::string s );
	public:
		MonoFontRenderer();
		MonoFontRenderer( TTF_Font * _font, const bool useMipmaps, const bool blending );
		~MonoFontRenderer();
		void set( TTF_Font * _font, const bool useMipmaps, const bool blending );
		void setGLflags();
		void render( std::string s, float x, float y );
		void render( std::string s, float x, float y, float height );
};

#endif //_MONO_FONTRENDERER_HPP_
