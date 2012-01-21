#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <memory>

#include <except.hpp>
#include <auto_array_ptr.hpp>
#include <logging.hpp>

#include <gl.hpp>
#include <SDL/SDL_image.h>


/*!
 * \file texture.hpp
 * \brief Implementacja klasy Texture
 */


//! Klasa przechowująca dane o teksturze
/*!
 * Umożliwia ładowanie tekstur, dostęp do części tekstury, usuwanie tekstur z pamięci, rysowanie sprite'ów.
 * Zlicza ilość obiektów odwołujących się do jednej tekstury na zasadzie smart_ptr i automatycznie zwalnia nieużywane tekstury.
 * Udostępnia opcję dynamicznego ładowania - jeżeli tekstura jest załadowana z pliku, można ją wyrzucić z RAMu na czas, w którym nie jest potrzebna.
 */
class Texture
{
	private:
		unsigned int W, H;
		float _x1, _x2, _y1, _y2;
		struct TexCounter
		{
			struct TC
			{
				int count;
				int needers;
				GLuint n;
				std::string path;
				bool mipmaps;
				TC( GLuint name, std::string p, bool _mipmaps ) : count( 1 ), needers( 1 ), n( name ), path( p ), mipmaps( _mipmaps ) {}
			} *tc;
			bool need;
			void del()					{ delete tc; tc = 0; };
			bool empty() const			{ return tc->n == 0 && tc->path.empty(); }
			void decreaseNeeders();
			void increaseNeeders();
			void operator =  ( TC * t )	{ tc = t; need = 1; }
			TC * operator -> ()			{ return tc; }
			const TC * operator -> () const	{ return tc; }
			void operator ++ ()			{ tc->count++; if ( need ) tc->needers++; }
			void operator -- ()			{ tc->count--; decreaseNeeders(); }
			operator bool() const		{ return tc; }
		} counter;
		void createCounter( GLuint n, std::string path, bool _mipmaps );
		void load();
	public:
		Texture( bool log = 0 );											//!< Pusty konstruktor
		Texture( int w, int h, GLenum pixelFormat, GLenum textureFormat, int offset, unsigned char * data, bool buildMipmaps = 1, bool log = 0 );	//!< Konstruktor ładujący z bufora pamięci
		Texture( const char *path, bool buildMipmaps = 1, bool log = 0 );	//!< Konstruktor ładujący z pliku
		Texture( const Texture &pattern );									//!< Konstruktor kopiujący
		Texture( const Texture *tex, unsigned int offsetx, unsigned int offsety, unsigned int width, unsigned int height, bool log = 0 );	//!< Konstruktor fragmentu
		Texture( SDL_Surface * pattern, GLenum pixelFormat, bool buildMipmaps = 1, bool log = 0 );	//!< Konstruktor ładujący z SDL_Surface
		~Texture();											//!< Destruktor
		
		bool logging;										//!< Określa, czy mają być wypisywane logi. Domyślnie 0.
		
		void reset();										//!< Czyści pola obiektu.
		
		const Texture rect( unsigned int offsetx, unsigned int offsety, unsigned int width, unsigned int height ) const;	//!< Pobiera fragment tekstury
		void loadFromFile( const char *path, bool buildMipmaps = 1 );	//!< Ładuje z pliku, używając SDL_image.
		void loadFromBuffer( int w, int h, GLenum pixelFormat, GLenum textureFormat, int offset, unsigned char * data, bool buildMipmaps = 1 );	//!< Ładuje z bufora pamięci.
		void loadFromSDL_Surface( SDL_Surface * pattern, GLenum textureFormat, bool buildMipmaps = 1 );	//!< Ładuje z SDL_Surface.
		void safeBind();													//!< Binduje teksturę bezpiecznie, tj. nigdy nie powoduje segfaulta
			//!< \see bind() \see fastBind() \see free() \see reload()
		void bind();														//!< Binduje teksturę, w razie potrzeby uprzednio ją przeładowując. Powoduje segfault jeżeli tekstura jest pusta (tj. counter == 0).
			//!< \see fastBind() \see safeBind() \see free() \see reload()
		void fastBind() const { glBindTexture( GL_TEXTURE_2D, counter -> n ); }	//!< Wrapper do glBindTexture. Binduje teksturę szybko, powoduje segfault jeżeli tekstura jest pusta (tj. counter == 0), nie ładuje dynamicznej tekstury.
			void free() { counter.decreaseNeeders(); }							//!< Zwalnia teksturę, tj. jeżeli tekstura nie jest używana przez żaden obiekt, wyrzuca ją z pamięci operacyjnej. Nie czyści obiektu, teksturę można nadal zbindować przez bind() lub safeBind() oraz załadować ponownie przez reload(). Aby permanentnie wyczyścić teksturę, użyj reset().
			//!< \see bind() \see fastBind() \see safeBind() \see reload()
		void reload() { if ( counter && !counter -> n && !counter -> path.empty() ) load(); }	//!< Przeładowuje teksturę z pliku, jeśli została wcześniej zwolniona.
				
		void draw( int x, int y ) const;							//!< Rysuje jako sprite'a
			//!< \warning Jeszcze nie zaimplementowane!
		
		const GLuint name() const { return counter ? counter -> n : 0; }
		//! Pobranie szerokości
		const unsigned int& width() const { return W; }	//!< \return szerokość tekstury w tekselach
		//! Pobranie wysokości
		const unsigned int& height() const { return H; }	//!< \return wysokość tekstury w tekselach
		//! Pobranie koordynaty x dla lewego brzegu
		float x1() const { return _x1; }			//!< \return TexCoord dla lewego brzegu
		//! Pobranie koordynaty y dla górnego brzegu
		float y1() const { return _y1; }			//!< \return TexCoord dla górnego brzegu
		//! Pobranie koordynaty x dla prawego brzegu
		float x2() const { return _x2; }			//!< \return TexCoord dla prawego brzegu
		//! Pobranie koordynaty y dla dolnego brzegu
		float y2() const { return _y2; }			//!< \return TexCoord dla dolnego brzegu
		//! Sprawdzenie czy jest przydzielona tekstura
		bool empty() const { return counter == 0 || counter.empty(); }		//!< \return true jeżeli tekstura nie jest przydzielona
											//!< \return false w przeciwnym wypadku
		//! Sprawdzenie czy jest załadowana tekstura
		bool loaded() { return counter && counter -> n != 0; }
		
		Texture& operator = ( const Texture &pattern );	//!< Operator podstawienia
		operator bool() const { return !empty(); }
		
		bool operator < ( const Texture& t ) { return counter.tc < t.counter.tc; }
};

inline void Texture::TexCounter::decreaseNeeders()
{
	if ( need ) tc -> needers--;
	need = 0;
	if ( tc -> needers == 0 && tc -> n != 0 ){
		glDeleteTextures( 1, &( tc -> n ) );
		tc -> n = 0;
	}
}

inline void Texture::TexCounter::increaseNeeders()
{
	if ( !need ){
		tc -> needers++;
	}
	need = 1;
}

inline void Texture::createCounter( GLuint n, std::string path, bool _mipmaps )
{
	counter = new TexCounter::TC( n, path, _mipmaps );
}

inline void Texture::bind()
{	
	if ( !counter.need ){
		counter.increaseNeeders();
		reload();
	}
	glBindTexture( GL_TEXTURE_2D, counter -> n );
}

inline void Texture::safeBind()
{	
	if ( counter ){
		if ( !counter.need ){
			counter.increaseNeeders();
			reload();
		}
		glBindTexture( GL_TEXTURE_2D, counter -> n );
	} else {
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
}

#endif // _TEXTURE_HPP_
