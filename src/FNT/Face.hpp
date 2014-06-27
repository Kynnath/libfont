/*
 * File:   Face.hpp
 * Author: juan.garibotti
 *
 * Created on 24 de junio de 2014, 15:39
 */

#ifndef FNT_FACE_HPP
#define	FNT_FACE_HPP

#include <map>
#include "ft2build.h"
#include FT_FREETYPE_H
#include "Glyph.hpp"

namespace fnt
{
    class Face
    {
        std::map<uint32_t, Glyph> m_characterMap;
        FT_Library m_freetype;
        FT_Face m_face;
        unsigned int m_size;

        public:
            Face( std::string const& i_filename, unsigned int const& i_size );
            void LoadGlyphs( std::u32string const& i_glyphs );
            Glyph const& GlyphData( uint32_t const& i_glyph ) const;
    };
}

#endif	/* FNT_FACE_HPP */

