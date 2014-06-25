/*
 * File:   Face.cpp
 * Author: juan.garibotti
 *
 * Created on 24 de junio de 2014, 15:39
 */

#include "Face.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

namespace fnt
{
    Face::Face()
    {
        // test initializing freetype
        FT_Library ft;

        FT_Init_FreeType(&ft);
    }
}