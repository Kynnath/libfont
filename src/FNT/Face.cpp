/*
 * File:   Face.cpp
 * Author: juan.garibotti
 *
 * Created on 24 de junio de 2014, 15:39
 */

#include "Face.hpp"

#include <cstring>
#include <memory>
#include <utility>
#include "GLT/Image.cpp"
#include "GLT/Model.hpp"

namespace fnt
{
  Face::Face( std::string const& i_filename, unsigned int const& i_size )
    : m_size ( i_size )
  {
    FT_Init_FreeType( &m_freetype );

    FT_Long const faceIndex = 0;
    FT_New_Face( m_freetype, i_filename.c_str(), faceIndex, &m_face );

    FT_UInt const xSize = 0;
    FT_Set_Pixel_Sizes( m_face, xSize, i_size );
  }
  
  Face::Face(Face && f)
    : m_characterMap {std::move(f.m_characterMap)}
    , m_freetype {std::move(f.m_freetype)}
    , m_face {std::move(f.m_face)}
    , m_size {std::move(f.m_size)}
    , m_vertexArray {std::move(f.m_vertexArray)}
    , m_texture {std::move(f.m_texture)}
  {
      f.m_freetype = 0;
      f.m_face = 0;
  }
  
  Face::~Face()
  {
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_freetype);
  }

  struct Bitmap
  {
    std::size_t m_width;
    std::size_t m_height;
    std::unique_ptr<unsigned char[]> m_data;
    int yFloor;
    int yCeiling;
    int xOrigin;

    Bitmap( std::size_t const& i_width, std::size_t const& i_height )
      : m_width   { i_width }
      , m_height  { i_height }
      , m_data    { new unsigned char[ m_width*m_height ] }
      , yFloor    { 0 }
      , yCeiling  { 0 }
      , xOrigin   { 0 }
    {}
  };

  static void AddGlyphToBitmap( FT_GlyphSlotRec_ const& i_glyph, Bitmap & o_bitmap )
  {
    auto const glyphWidth = i_glyph.bitmap.width;
    auto const glyphHeight = i_glyph.bitmap.rows;
    if ( glyphWidth > 256 || glyphHeight > 256 )
    {
      throw 0; // throw glyph too large error
    }
    if ( o_bitmap.xOrigin + glyphWidth > 256 )
    {
      o_bitmap.yFloor += o_bitmap.yCeiling;
      o_bitmap.yCeiling = 0;
      o_bitmap.xOrigin = 0;
    }
    if ( o_bitmap.yFloor + glyphHeight > 256 )
    {
      throw 0; // throw can't fit in texture error
    }
    if ( glyphHeight > o_bitmap.yCeiling )
    {
      o_bitmap.yCeiling = glyphHeight;
    }

    auto const bitmapPitch = i_glyph.bitmap.pitch;
    auto const*const buffer = i_glyph.bitmap.buffer;
    int bitmapStart = 0;
    if ( bitmapPitch > 0 )
    {
      bitmapStart = bitmapPitch*( glyphHeight - 1 );
    }
    for ( int yIndex = 0; yIndex < glyphHeight; ++yIndex )
    {
      std::memcpy( &o_bitmap.m_data[ static_cast<std::size_t>(((o_bitmap.yFloor+yIndex)*256)+o_bitmap.xOrigin) ],
                   &buffer[ bitmapStart-(yIndex*bitmapPitch) ],
                   static_cast<std::size_t>(glyphWidth) );
    }
    o_bitmap.xOrigin += glyphWidth;
  }

  static void AddVerticesToArray( FT_GlyphSlotRec_ const& i_glyph, Bitmap const& i_bitmap, glt::Model & o_model )
  {
    GLfloat const width = static_cast<GLfloat>(i_glyph.metrics.width) / 64.0f;
    GLfloat const height = static_cast<GLfloat>(i_glyph.metrics.height) / 64.0f;
    GLfloat const posLeft = static_cast<GLfloat>(i_glyph.metrics.horiBearingX) / 64.0f;
    GLfloat const posRight = posLeft + width;
    GLfloat const posTop = static_cast<GLfloat>(i_glyph.metrics.horiBearingY) / 64.0f;
    GLfloat const posBottom = posTop - height;
    GLfloat const bitmapWidth = static_cast<GLfloat>( i_bitmap.m_width );
    GLfloat const bitmapHeight = static_cast<GLfloat>( i_bitmap.m_height );
    GLfloat const texWidth = width / bitmapWidth;
    GLfloat const texHeight = height / bitmapHeight;
    GLfloat const texLeft = (static_cast<GLfloat>(i_bitmap.xOrigin) - width) / bitmapWidth;
    GLfloat const texRight = texLeft + texWidth;
    GLfloat const texBottom = static_cast<GLfloat>(i_bitmap.yFloor) / bitmapHeight;
    GLfloat const texTop = texBottom + texHeight;

    GLuint const vert = static_cast<GLuint>(o_model.m_vertexList.size());
    o_model.m_vertexList.push_back({ {posLeft, posBottom,0}, {0,0,0}, {texLeft,texBottom} });
    o_model.m_vertexList.push_back({ {posRight, posBottom,0}, {0,0,0}, {texRight,texBottom} });
    o_model.m_vertexList.push_back({ {posLeft, posTop,0}, {0,0,0}, {texLeft,texTop} });
    o_model.m_vertexList.push_back({ {posRight, posTop,0}, {0,0,0}, {texRight,texTop} });

    o_model.m_indexList.push_back( vert+0 );
    o_model.m_indexList.push_back( vert+1 );
    o_model.m_indexList.push_back( vert+2 );
    o_model.m_indexList.push_back( vert+2 );
    o_model.m_indexList.push_back( vert+1 );
    o_model.m_indexList.push_back( vert+3 );
  }

  static GLuint CreateVertexArrayObject( glt::Model const& i_model )
  {
    GLuint vertexArray;
    glGenVertexArrays( 1, &vertexArray );
    glBindVertexArray( vertexArray );
    {
      // Create vertex buffer object
      GLuint vertexBuffer;
      glGenBuffers( 1, &vertexBuffer );
      // Copy data to buffer object
      glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
      glBufferData( GL_ARRAY_BUFFER, GLsizeiptr(sizeof(glt::Vertex)*i_model.m_vertexList.size()), i_model.m_vertexList.data() , GL_STATIC_DRAW );
    }
    {
      // Enable vertex position attribute
      glEnableVertexAttribArray( glt::Vertex::Position );
      glVertexAttribPointer( glt::Vertex::Position, 3, GL_FLOAT, GL_FALSE, GLsizei( sizeof( glt::Vertex ) ), 0 );
    }
    /*{
      // Not enabled since font files don't hold color information
      glEnableVertexAttribArray( glt::Vertex::Color );
      glVertexAttribPointer( glt::Vertex::Color, 3, GL_FLOAT, GL_FALSE, GLsizei( sizeof( glt::Vertex ) ), (GLvoid const*)(sizeof(GLfloat)*3) );
    }*/
    {
      // Enable texture attribute
      glEnableVertexAttribArray( glt::Vertex::Texture );
      glVertexAttribPointer( glt::Vertex::Texture, 2, GL_FLOAT, GL_FALSE, GLsizei( sizeof( glt::Vertex ) ), (GLvoid const*)(sizeof(GLfloat)*6) );
    }
    {
      // Create index buffer object
      GLuint indexBuffer;
      glGenBuffers( 1, &indexBuffer );
      // Copy index data
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(sizeof(GLuint)*i_model.m_indexList.size()), i_model.m_indexList.data(), GL_STATIC_DRAW);
    }
    // Unbind vertex array
    glBindVertexArray(0);

    return vertexArray;
  }

  void Face::LoadGlyphs( std::u32string const& i_characters )
  {
    int const sideLength = 256;
    Bitmap faceBitmap { sideLength, sideLength };
    glt::Model faceModel;

    for ( auto const& character: i_characters )
    {
      FT_Load_Char( m_face, character, FT_LOAD_DEFAULT );
      if ( m_face->glyph->format != FT_GLYPH_FORMAT_BITMAP )
      {
          FT_Render_Glyph( m_face->glyph, FT_RENDER_MODE_NORMAL );
      }

      AddGlyphToBitmap( *m_face->glyph, faceBitmap );
      AddVerticesToArray( *m_face->glyph, faceBitmap, faceModel );
      std::size_t const numIndices = 6;
      m_characterMap.insert( std::make_pair( character, Glyph{ reinterpret_cast<GLvoid*>( sizeof(GL_UNSIGNED_INT) * ( faceModel.m_indexList.size() - numIndices ) ),
                                                               static_cast<GLfloat>( m_face->glyph->linearHoriAdvance ) / 65536.0f } ) );
    }

    glt::Image const faceImage { glt::ImageDescription { sideLength, sideLength, GL_RED, GL_UNSIGNED_BYTE },
                                 std::move(faceBitmap.m_data) };
    m_texture = glt::Texture { faceImage, { 0, GL_CLAMP_TO_EDGE, GL_NEAREST, false } };
    m_vertexArray = CreateVertexArrayObject( faceModel );
  }

  Glyph const& Face::GlyphData( uint32_t const& i_glyph ) const
  {
    auto glyphIter = m_characterMap.find( i_glyph );
    if ( glyphIter != m_characterMap.end() )
    {
      return glyphIter->second;
    }
    else
    {
      return m_characterMap.find( 0 )->second;
    }
  }
}