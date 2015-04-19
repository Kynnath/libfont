/*
 * File:   Face.hpp
 * Author: juan.garibotti
 *
 * Created on 24 de junio de 2014, 15:39
 */

#ifndef FNT_FACE_HPP
#define	FNT_FACE_HPP

#include <map>
#include "GLT/Texture.hpp"
#include "GLT/VertexArray.hpp"
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
    glt::VertexArray m_vertexArray;
    glt::Texture m_texture;

    Face(Face const& f) = delete;
    Face& operator=(Face const& f) = delete;
    Face& operator=(Face && f) = delete;

    public:
      Face( std::string const& i_filename, unsigned int const& i_size );
      Face(Face && f);
      ~Face();
      void LoadGlyphs(std::u32string const& i_glyphs);
      Glyph const& GlyphData(uint32_t glyph) const;
      glt::Texture const& Texture() const { return m_texture; }
      GLuint VertexArray() const { return m_vertexArray.VerArr(); }
      FT_Size_Metrics const& GlobalMetrics() const {return m_face->size->metrics;}
  };
}

#endif	/* FNT_FACE_HPP */

