/*
 * File:   Glyph.hpp
 * Author: juan.garibotti
 *
 * Created on 25 de junio de 2014, 14:00
 */

#ifndef GLYPH_HPP
#define	GLYPH_HPP

#include "GL/glew.h"

namespace fnt
{
  struct Glyph
  {
    GLvoid const* m_indicesOffset;
    GLfloat m_advance;
  };
}

#endif	/* GLYPH_HPP */

