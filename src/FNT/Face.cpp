/*
 * File:   Face.cpp
 * Author: juan.garibotti
 *
 * Created on 24 de junio de 2014, 15:39
 */

#include "Face.hpp"

#include <cstring>
#include <memory>
#include "GLT/Image.cpp"
#include "GLT/Texture.hpp"

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

    void Face::LoadGlyphs( std::u32string const& i_characters )
    {
        int const sideLength = 256;
        std::size_t const bitmapSize = sideLength*sideLength;
        std::unique_ptr<unsigned char[]> faceBitmap { new unsigned char[ bitmapSize ] };
        int yFloor = 0;
        int yCeiling = 0;
        int xOrigin = 0;

        for ( auto const& character: i_characters )
        {
            FT_Load_Char( m_face, character, FT_LOAD_DEFAULT );
            if ( m_face->glyph->format != FT_GLYPH_FORMAT_BITMAP )
            {
                FT_Render_Glyph( m_face->glyph, FT_RENDER_MODE_NORMAL );
            }

            auto const glyphWidth = m_face->glyph->bitmap.width;
            auto const glyphHeight = m_face->glyph->bitmap.rows;
            if ( glyphWidth > 256 || glyphHeight > 256 )
            {
                throw 0; // throw glyph too large error
            }
            if ( xOrigin + glyphWidth > 256 )
            {
                yFloor += yCeiling;
                yCeiling = 0;
                xOrigin = 0;
            }
            if ( yFloor + glyphHeight > 256 )
            {
                throw 0; // throw can't fit in texture error
            }
            if ( glyphHeight > yCeiling )
            {
                yCeiling = glyphHeight;
            }

            auto const bitmapPitch = m_face->glyph->bitmap.pitch;
            auto const*const buffer = m_face->glyph->bitmap.buffer;
            int bitmapStart = 0;
            if ( bitmapPitch > 0 )
            {
                bitmapStart = bitmapPitch*( glyphHeight - 1 );
            }
            for ( int yIndex = 0; yIndex < glyphHeight; ++yIndex )
            {
                std::memcpy( &faceBitmap[ static_cast<std::size_t>(((yFloor+yIndex)*256)+xOrigin) ],
                             &buffer[ bitmapStart-(yIndex*bitmapPitch) ],
                             static_cast<std::size_t>(glyphWidth) );
            }
            xOrigin += glyphWidth;
        }

        glt::Image faceImage { glt::ImageDescription { sideLength, sideLength, GL_RED, GL_UNSIGNED_BYTE }
                               , std::move(faceBitmap) };
        glt::Texture faceTexture { faceImage };
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