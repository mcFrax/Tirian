#include <texture.hpp>


inline int min2( const int n )
{
	for ( int i = 1;; i <<= 1 )
		if ( n <= i ) return i;
}

/*!
 * Zeruje pola obiektu
 */ 
Texture::Texture( bool log ) : logging( log )
{
	W = H = 0;
	_x1 = _x2 = _y1 = _y2 = 0.0f;
	counter = 0;
}

/*!
 * Tworzy teksturę z bufora.
 * \param w wysokość tekstury w pikselach.
 * \param h szerokość tekstury w pikselach.
 * \param pixelFormat format pikseli w danej bitmapie.
 * \param textureFormat format pikseli w teksturze.
 * \param offset przerwa pomiędzy kolejnymi pikselami
 * \param data wskaźnik do pikseli
 * \param buildMipmaps określa, czy mają być generowane mipmapy. Domyślnie 1.
 * \param log określa, czy ma być wypisywany log. Domyślnie 0.
 */
Texture::Texture( int w, int h, GLenum pixelFormat, GLenum textureFormat, int offset, unsigned char * data, bool buildMipmaps, bool log ) : logging( log )
{
	W = H = 0;
	_x1 = _x2 = _y1 = _y2 = 0.0f;
	counter = 0;
	loadFromBuffer( w, h, pixelFormat, textureFormat, offset, data, buildMipmaps );
}

/*!
 * Ładuje teksturę z określonego pliku, korzystając z SDL_image.
 * \param path ścieżka pliku.
 * \param buildMipmaps określa, czy mają być generowane mipmapy. Domyślnie 1.
 * \exception std::runtime_error jeżeli wystąpi błąd podczas ładowania
 * \param log określa, czy ma być wypisywany log. Domyślnie 0.
 */
Texture::Texture( const char * path, bool buildMipmaps, bool log ) : logging( log )
{
	W = H = 0;
	_x1 = _x2 = _y1 = _y2 = 0.0f;
	counter = 0;
	loadFromFile( path, buildMipmaps );
}

/*!
 * Kopiuje obiekt Texture
 * \param pattern obiekt do skopiowania
 */
Texture::Texture( const Texture &pattern )
{
	counter = 0;
	*this = pattern;
}

/*!
 * Tworzy teksturę jako część załadowanej tekstury. Współrzędne nie są ograniczane do rzeczywistej wielkości tekstury.
 * \param tex wskaźnik do obiektu Texture z załadowaną teksturą
 * \param offsetx przesunięcie x w tekselach
 * \param offsety przesunięcie y w tekselach
 * \param width szerokość w tekselach
 * \param height wysokość w tekselach
 * \param log określa, czy ma być wypisywany log. Domyślnie 0.
 */
Texture::Texture( const Texture *tex, unsigned int offsetx, unsigned int offsety, unsigned int width, unsigned int height, bool log ) : logging( log )
{
	memcpy( this, tex, sizeof( Texture ) );
	if ( counter )
		++counter;
		
	W = width;
	H = height;
	
	_x1 = static_cast<float>(offsetx) / static_cast<float>(tex->width()) * ( tex->x2() - tex->x1() ) + tex->x1();
	_y1 = static_cast<float>(offsety) / static_cast<float>(tex->height()) * ( tex->y2() - tex->y1() ) + tex->y1();
	_x2 = _x1 + static_cast<float>(W) / static_cast<float>(tex->width()) * ( tex->x2() - tex->x1() );
	_y2 = _y1 + static_cast<float>(H) / static_cast<float>(tex->height()) * ( tex->y2() - tex->y1() );
}

Texture::Texture( SDL_Surface * pattern, GLenum textureFormat, bool buildMipmaps, bool log ) : logging( log )
{
	W = H = 0;
	_x1 = _x2 = _y1 = _y2 = 0.0f;
	counter = 0;
	loadFromSDL_Surface( pattern, textureFormat, buildMipmaps );
}


/*!
 * Jeżeli tekstura nie jest już używana, usuwa ją z pamięci
 */
Texture::~Texture()
{
	reset();
}

/*!
 * Czyści pola obiektu.
 * Jeżeli tekstura nie jest już używana, usuwa ją z pamięci.
 */
void Texture::reset()
{
	if ( counter ){
		--counter;
		if ( counter -> count == 0 ){
			counter.del();
		}
	}
	counter = 0;
	W = H = 0;
	_x1 = _x2 = _y1 = _y2 = 0.0f;
}

Texture& Texture::operator = ( const Texture &pattern )
{
	reset();
	memcpy( this, &pattern, sizeof( Texture ) );
	if ( counter )
		++counter;
	return *this;
}

/*!
 * Tworzy nowy obiekt Texture określający wybrany fragment załadowanej tekstury. Współrzędne nie są ograniczane do rzeczywistej wielkości tekstury.
 * \param offsetx przesunięcie x w tekselach
 * \param offsety przesunięcie y w tekselach
 * \param width szerokość w tekselach
 * \param height wysokość w tekselach
 * \return obiekt Texture określający wybrany fragment tekstury
 */
const Texture Texture::rect( unsigned int offsetx, unsigned int offsety, unsigned int width, unsigned int height ) const
{
	return Texture( this, offsetx, offsety, width, height );
}

void Texture::loadFromSDL_Surface( SDL_Surface * pattern, GLenum textureFormat, bool buildMipmaps )
{
	try
	{
		reset();
		
		int w = pattern -> w;
		int h = pattern -> h;
		int w2 = min2( w );
		int h2 = min2( h );		
		int Bpp = pattern -> format -> BytesPerPixel;
		
		GLuint name;
		auto_array_ptr< unsigned char > buff = new unsigned char[w2*h2*4];
		char lineend = (w*Bpp)%4 == 0 ? 0 : 4 - (w*Bpp)%4;
		
		unsigned char* ptr = reinterpret_cast<unsigned char*>( pattern -> pixels );
		for ( int i = 0; i < h; i++ ){
			for ( int j = 0; j < w; j++ ){
				SDL_GetRGBA( *reinterpret_cast< Uint32 *>( ptr ), pattern -> format, &buff[w2*4*i+j*4], &buff[w2*4*i+j*4+1], &buff[w2*4*i+j*4+2], &buff[w2*4*i+j*4+3] );
				ptr += Bpp;
			}
			ptr+=lineend;
			if ( w2 > w ) memcpy( &buff[w2*4*i+w*4], &buff[w2*4*i+(w-1)*4], 4 );
		}
		if ( h2 > h ){
			memcpy( &buff[w2*4*h], &buff[w2*4*(h-1)], w*4 );
			if ( w2 > w ) memcpy( &buff[w2*4*h+w*4], &buff[w2*4*h+(w-1)*4], 4 );
		}
		
		gltErrorCheck();
		
		GLint currentName = 0;
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &currentName );
		
		glGenTextures( 1, &name );
		glBindTexture( GL_TEXTURE_2D, name );
		
		gltErrorCheck();
		
		if ( buildMipmaps ){
			gluBuild2DMipmaps( GL_TEXTURE_2D, textureFormat, w2, h2, GL_RGBA, GL_UNSIGNED_BYTE, buff );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
		} else {
			glTexImage2D( GL_TEXTURE_2D, 0, textureFormat, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, buff );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		}
		
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
		gltErrorCheck();
		
		glBindTexture ( GL_TEXTURE_2D, currentName );
		
		createCounter( name, std::string(), buildMipmaps );
		W = w;
		H = h;
		_x1 = _y1 = 0.0f;
		_x2 = static_cast<float>(W) / static_cast<float>(min2( W ));
		_y2 = static_cast<float>(H) / static_cast<float>(min2( H ));
		
		if ( logging )
			LOG(( "Loaded texture #%i from SDL_Surface :\n	%i x %i pixels %s, %i x %i texels %s\n", counter -> n, w, h, "", w2, h2, "" ));
	}
	catch( std::logic_error &le )
	{
		MKLE_LOAD_ERROR( "texture", "SDL_Surface", le.what() );
	}
	catch( std::exception &re )
	{
		MKRE_LOAD_ERROR( "texture", "SDL_Surface", re.what() );
	}
}

/*!
 * Ładuje teksturę z bufora. Wszystkie wartości wyrażone są jako unsigned char.
 * \param w wysokość tekstury w pikselach.
 * \param h szerokość tekstury w pikselach.
 * \param pixelFormat format pikseli w danej bitmapie. Działa tylko dla GL_RGB, GL_RGBA i GL_ALPHA.
 * \param textureFormat format pikseli w teksturze.
 * \param offset przerwa pomiędzy kolejnymi pikselami
 * \param data wskaźnik do pikseli
 * \param buildMipmaps określa, czy mają być generowane mipmapy. Domyślnie 1.
 */
void Texture::loadFromBuffer( int w, int h, GLenum pixelFormat, GLenum textureFormat, int offset, unsigned char * data, bool buildMipmaps )
{
	try
	{
		reset();
		
		int w2, h2, Bpp, Bpp2;
		GLuint name;
		
		w2 = min2( w );
		h2 = min2( h );
		switch ( pixelFormat ){
			case ( GL_BGR ) :
			case ( GL_RGB ) : Bpp = 3;
				break;
			case ( GL_BGRA ) :
			case ( GL_RGBA ) : Bpp = 4;
				break;
			case ( GL_ALPHA ) : Bpp = 1;
				break;
			default : throw MKLE( "Bad pixelFormat" );
		}
		Bpp2 = Bpp+offset;
		
		unsigned char* buff;
		
		auto_array_ptr< unsigned char > buffer;
		
		if ( w != w2 || h != h2 || offset != 0 ){
			buffer = new unsigned char[w2*h2*Bpp];
			buff = buffer;
			
			unsigned char* ptr = data;
			for ( int i = 0; i < h; i++ ){
				for ( int j = 0; j < w; j++ ){
					memcpy( &buff[w2*Bpp*i+j*Bpp], ptr, Bpp );
					ptr += Bpp2;
				}
				if ( w2 > w ) memcpy( &buff[w2*Bpp*i+w*Bpp], &buff[w2*Bpp*i+(w-1)*Bpp], Bpp );
			}
			if ( h2 > h ){
				memcpy( &buff[w2*Bpp*h], &buff[w2*Bpp*(h-1)], w*Bpp );
				if ( w2 > w ) memcpy( &buff[w2*Bpp*h+w*Bpp], &buff[w2*Bpp*h+(w-1)*Bpp], Bpp );
			}
		} else {
			buff = data;
		}
		
		gltErrorCheck();
		
		GLint currentName = 0;
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &currentName );
		
		glGenTextures( 1, &name );
		glBindTexture( GL_TEXTURE_2D, name );
		
		gltErrorCheck();
		
		if ( buildMipmaps ){
			gluBuild2DMipmaps( GL_TEXTURE_2D, textureFormat, w2, h2, pixelFormat, GL_UNSIGNED_BYTE, buff );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		} else {
			glTexImage2D( GL_TEXTURE_2D, 0, textureFormat, w2, h2, 0, pixelFormat, GL_UNSIGNED_BYTE, buff );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		}
		
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
		gltErrorCheck();
		
		glBindTexture ( GL_TEXTURE_2D, currentName );
		
		createCounter( name, std::string(), buildMipmaps );
		W = w;
		H = h;
		_x1 = _y1 = 0.0f;
		_x2 = static_cast<float>(W) / static_cast<float>(min2( W ));
		_y2 = static_cast<float>(H) / static_cast<float>(min2( H ));
		
		if ( logging )
			LOG(( "Loaded texture from buffer :\n	%i x %i pixels %s, %i x %i texels %s, offset = %i\n", w, h, (pixelFormat == GL_RGBA) ? "RGBA" : (pixelFormat == GL_RGB) ? "RGB" : "Alpha", w2, h2, (textureFormat == GL_RGBA) ? "RGBA" : (textureFormat == GL_RGB) ? "RGB" : "Alpha", offset ));
	}
	catch ( std::runtime_error &re )				
	{												
		throw MKRE_LOAD_ERROR( "texture", "buffer", re.what() );	
	}												
	catch ( std::logic_error &le )					
	{												
		throw MKLE_LOAD_ERROR( "texture", "buffer", le.what() );	
	}
}

void Texture::load()
{
	TRY
		SDL_Surface * img = IMG_Load( counter -> path.c_str() );
        if ( !img )
            throw std::runtime_error( (std::string("IMG_Load (\"")+=counter -> path.c_str()) +="\") failed" );
		loadFromSDL_Surface( img, GL_RGBA, counter -> mipmaps );
		SDL_FreeSurface( img );
	CATCH_LOAD( "texture", counter -> path );
}

/*!
 * \param path ścieżka pliku.
 * \param buildMipmaps określa, czy mają być generowane mipmapy. Domyślnie 1.
 * \exception std::runtime_error w przypadku niemożliwości załadowania tekstury.
 */
void Texture::loadFromFile( const char *path, bool buildMipmaps )
{
	try
	{
	
		reset();
		
		createCounter( 0, path, buildMipmaps );
		load();
		
		if ( logging )
			LOG(( "Loaded texture from \"%s\" :\n	%i x %i pixels, %i x %i texels\n", path, W, H, min2( W ), min2( H ) ));
	}
	catch( std::exception &ex )
	{
		counter.del();
		STDTHROW;
	}
}

