#include <fontrenderer.hpp>

SDLFontRenderer::SDLFontRenderer() : font( 0 )
{
	for ( int i = 0; i < 4; i++ ) color[ i ] = 1.0f;
}

SDLFontRenderer::SDLFontRenderer( TTF_Font * _font, const bool useMipmaps, const bool blending )
	: blend( blending ), mipmaps( useMipmaps ), font( _font )
{
	if ( !font )
		throw std::logic_error( "#SDLFontRenderer: Font is null" );
	h = TTF_FontHeight( font );
	ls = TTF_FontLineSkip( font );
	for ( int i = 0; i < 4; i++ ) color[ i ] = 1.0f;
}

void SDLFontRenderer::set( TTF_Font * _font, const bool useMipmaps, const bool blending )
{
	font = _font; mipmaps = useMipmaps; blend = blending;
	if ( !font )
		throw std::logic_error( "#SDLFontRenderer: Font is null" );
	h = TTF_FontHeight( font );
	ls = TTF_FontLineSkip( font );
	tex.clear();
	make( ' ' );
}

Texture * SDLFontRenderer::get( char c )
{
	std::map<char, Texture>::iterator t = tex.find( c );
	if ( t != tex.end() ) {
		return &( t -> second );
	} else {
		return make( c );
	}
}

Texture * SDLFontRenderer::make( char c )
{
	if ( !font )
		return 0;
	if ( c == 0 ){
		return &( tex.insert( std::make_pair( c, Texture() ) ).first  -> second );
	}
	SDL_Color color;
	
	//~ Uint16 wt[ 2 ] = {0};
	//~ wt[ 0 ] = static_cast<const Uint16>( *reinterpret_cast<Uint8 *>(&c) );
	//~ SDL_Surface * txt = TTF_RenderUNICODE_Blended( font, wt, color );
	//~ SDL_Surface * txt = TTF_RenderGlyph_Blended( font, static_cast<const Uint16>( *reinterpret_cast<Uint8 *>(&c) ), color );
	
	char t[ 2 ] = {0};
	t[ 0 ] = c;
	SDL_Surface * txt = TTF_RenderUTF8_Blended( font, t, color );
	
	Texture * te = &(tex.insert( std::make_pair( c, Texture( txt, GL_ALPHA, mipmaps, 0 ) ) ).first  -> second);
	SDL_FreeSurface( txt );
	return te;
}

void SDLFontRenderer::setGLflags()
{
	GLTERRORCHECK;
	glActiveTexture( GL_TEXTURE1 );
	glDisable(GL_TEXTURE_2D);
	
	glActiveTexture( GL_TEXTURE0 );
	glEnable(GL_TEXTURE_2D);
	
	glDisable( GL_FOG );
	glDisable( GL_LIGHTING );
	//~ glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glColorMaterial( GL_BACK, GL_AMBIENT );
	glEnable( GL_COLOR_MATERIAL );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLTERRORCHECK;
}

void SDLFontRenderer::render( std::string s, float x, float y )
{
	render( s, x, y, h );
}

void SDLFontRenderer::render( std::string s, float x, float y, float height )
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	//~ glDisableClientState(GL_COLOR_ARRAY);
	
	setGLflags();
	
	vert * vertex = new vert [ ( s.size()*2 + 2 ) ];
	vert texArray[ 4 ];
	unsigned int indices[ 4 ] = { 0, 1, 3, 2 };
	float heightFactor = height / h;
	
	vertex[ 1 ].x = vertex[ 0 ].x = x;
	vertex[ 1 ].y = height + ( vertex[ 0 ].y = y );
	
	glTexCoordPointer( 2, GL_FLOAT, 0, texArray );
	
	glColor4fv( color );
	
	float sx = x;
	
		texArray[ 1 ].x = texArray[ 0 ].x = 0.0f;
		texArray[ 2 ].y = texArray[ 0 ].y = 0.0f;
	
	for ( unsigned int i = 1; i <= s.size(); i++ ){
		Texture * t = get( s[ i-1 ] );
		vertex[ i*2 ].y = y;
		vertex[ i*2+1 ].y = y+height;
		vertex[ i*2+1 ].x = vertex[ i*2 ].x = ( sx += heightFactor*t -> width() );
		t -> fastBind();
		//~ 
		glVertexPointer( 2, GL_FLOAT, 0, &vertex[i*2-2] );
		texArray[ 3 ].y = texArray[ 1 ].y = t -> y2();
		texArray[ 2 ].x = texArray[ 3 ].x = t -> x2();
	
		glDrawElements( GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, indices );
		//~ glDrawRangeElements( GL_TRIANGLE_FAN, 0, 3, 4, GL_UNSIGNED_SHORT, indices );
	}
		
}
