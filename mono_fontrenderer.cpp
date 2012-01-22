#include <mono_fontrenderer.hpp>

static const char cbeg = ' ';
static const char cend = '~';

MonoFontRenderer::MonoFontRenderer()
{
}

MonoFontRenderer::MonoFontRenderer( TTF_Font * _font, const bool useMipmaps, const bool blending )
{
    set(_font, useMipmaps, blending);
}

void MonoFontRenderer::set( TTF_Font * _font, const bool useMipmaps, const bool blending )
{
    font = _font; mipmaps = useMipmaps; blend = blending;
	if ( !font )
		throw std::logic_error( "#MonoFontRenderer: Font is null" );
	h = TTF_FontHeight( font );
	ls = TTF_FontLineSkip( font );
	texmap.clear();
	make(' ');
	createDisplays();
}

MonoFontRenderer::~MonoFontRenderer()
{
	if (displayList)
		glDeleteLists(displayList, cend-cbeg+1);
}

void MonoFontRenderer::createDisplays()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	displayList = glGenLists(cend-cbeg+1);
	if (!displayList) throw RuntimeError("Failed to generate lists");
	char t[ cend-cbeg+2 ];
	for ( char c = cbeg; c <= cend; ++c )
		t[c-cbeg] = c;
	t[ cend-cbeg+1 ] = 0;
	
	SDL_Color color;
	SDL_Surface * txt = TTF_RenderUTF8_Blended( font, t, color );
	
	tex = Texture( txt, GL_ALPHA, mipmaps, 0 );
	SDL_FreeSurface( txt );
	LOG(("%i, %i, %i\n", tex.width(), cend-cbeg+1, tex.width() % (cend-cbeg+1)));
	float charwidth = float(tex.width()) / (cend-cbeg+1);
	
	uint n = cend-cbeg+1;
	for ( uint i = 0; i < n; ++i ){
		glNewList(displayList+i, GL_COMPILE);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(tex.x1()*float(n-i)/n + tex.x2()*float(i)/n, tex.y1());
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(tex.x1()*float(n-i)/n + tex.x2()*float(i)/n, tex.y2());
		glVertex2f(0.0f, h);
		glTexCoord2f(tex.x1()*float(n-i-1)/n + tex.x2()*float(i+1)/n, tex.y1());
		glVertex2f(charwidth, 0.0f);
		glTexCoord2f(tex.x1()*float(n-i-1)/n + tex.x2()*float(i+1)/n, tex.y2());
		glVertex2f(charwidth, h);
		glEnd();
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(charwidth+1, 0.0f, 0.0f);
		glEndList();
	}
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

Texture * MonoFontRenderer::get( char c )
{
	std::map<char, Texture>::iterator t = texmap.find( c );
	if ( t != texmap.end() ) {
		return &( t -> second );
	} else {
		return make( c );
	}
}

Texture * MonoFontRenderer::make( char c )
{
	if ( !font )
		return 0;
	if ( c == 0 ){
		return &( texmap.insert( std::make_pair( c, Texture() ) ).first  -> second );
	}
	SDL_Color color;
	
	char t[ 2 ] = {0};
	t[ 0 ] = c;
	SDL_Surface * txt = TTF_RenderUTF8_Blended( font, t, color );
	
	Texture * te = &(texmap.insert( std::make_pair( c, Texture( txt, GL_ALPHA, mipmaps, 0 ) ) ).first  -> second);
	SDL_FreeSurface( txt );
	return te;
}

void MonoFontRenderer::setGLflags()
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

void MonoFontRenderer::render( std::string s, float x, float y, float height )
{
	//~ LOG(("Rendering: \"%s\"\n", s.c_str()));
	if (s.empty()) return;
	setGLflags();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x,y,0.0f);
	float heightFactor = height / h;
	glScalef(heightFactor, heightFactor, 1.0f);
	uint blockbeg = 0;
	bool curstd = (cbeg <= s[0])&&(s[0] <= cend);
	for ( uint curpos = 0;; ++curpos ){
		if ((((cbeg <= s[curpos])&&(s[curpos] <= cend)) != curstd || s[curpos] == '\0')&& curpos!=blockbeg){
			if ( curstd ){
				//~ LOG(("STDrender at \"%u\" with blockbeg = \"%u\"\n", curpos, blockbeg));
				//~ LOG(("%i\n", curpos-blockbeg));
				render_standard( std::string(s, blockbeg, curpos-blockbeg) );
			} else {
				render_special( std::string(s, blockbeg, curpos-blockbeg) );
			}
			blockbeg = curpos;
			curstd = !curstd;
		}
		if (curpos >= s.size()) break;
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	//~ LOG(("Rendered \"%s\".\n", s.c_str()));
}

void MonoFontRenderer::render_standard( std::string s )
{
	glColor4fv( color );
	tex.safeBind();
	//~ LOG(("Standard render: \"%s\"\n", s.c_str()));
	uint lists[s.size()];
	for ( uint i = 0; i < s.size(); ++i )
		lists[i] = displayList+s[i]-cbeg;
	glCallLists(s.size(), GL_UNSIGNED_INT, lists);
}

void MonoFontRenderer::render_special( std::string s )
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	vert * vertex = new vert [ ( s.size()*2 + 2 ) ];
	vert texArray[ 4 ];
	unsigned int indices[ 4 ] = { 0, 1, 3, 2 };
	
	vertex[ 1 ].x = vertex[ 0 ].x = 0.0f;
	vertex[ 1 ].y = h + ( vertex[ 0 ].y = 0.0f );
	
	glTexCoordPointer( 2, GL_FLOAT, 0, texArray );
	
	glColor4fv( color );
	
	texArray[ 1 ].x = texArray[ 0 ].x = 0.0f;
	texArray[ 2 ].y = texArray[ 0 ].y = 0.0f;
	
	for ( unsigned int i = 1; i <= s.size(); i++ ){
		Texture * t = get( s[ i-1 ] );
		vertex[ i*2 ].y = 0.0f;
		vertex[ i*2+1 ].y = h;
		vertex[ i*2+1 ].x = vertex[ i*2 ].x = ( t -> width() );
		t -> fastBind();
		//~ 
		glVertexPointer( 2, GL_FLOAT, 0, &vertex[i*2-2] );
		texArray[ 3 ].y = texArray[ 1 ].y = t -> y2();
		texArray[ 2 ].x = texArray[ 3 ].x = t -> x2();
	
		glDrawElements( GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, indices );
	}
}
