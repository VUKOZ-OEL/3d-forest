/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file ViewerOpenGLStrokeFont.cpp */

// Include 3D Forest.
#include <Util.hpp>
#include <ViewerOpenGLStrokeFont.hpp>

// Include Qt.
#include <QOpenGLFunctions>

// Include local.
#define LOG_MODULE_NAME "ViewerOpenGLStrokeFont"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ViewerOpenGLStrokeFont::ViewerOpenGLStrokeFont()
    : nChars_(128),
      charHeight_(0.5f)
{
    LOG_DEBUG(<< "Create.");
}

ViewerOpenGLStrokeFont::~ViewerOpenGLStrokeFont()
{
    LOG_DEBUG(<< "Destroy.");
}

// Get base of the display list representation.
GLuint ViewerOpenGLStrokeFont::list(ViewerOpenGLManager *managerGL)
{
    ASSERT(managerGL);

    GLuint displayList;

    if (!isInitialized(managerGL))
    {
        updateGL(managerGL);
    }

    displayList = managerGL->resource(this, displayListId_);

    return displayList;
}

float ViewerOpenGLStrokeFont::textWidth(const std::string &text)
{
    LOG_DEBUG(<< "Get text width <" << text << ">.");
    // if (!isInitialized ())  {
    //    return 0.0F;
    // }

    float result = 0.0F;
    size_t nTextChars = text.size();
    size_t iChar;

    LOG_DEBUG(<< "Characters <" << nTextChars << ">.");
    LOG_DEBUG(<< "CharWidth <" << charWidth_.size() << ">.");

    if (charWidth_.size() > 0)
    {
        for (iChar = 0; iChar < nTextChars; ++iChar)
        {
            result += charWidth_[text[iChar]];
        }
    }

    LOG_DEBUG(<< "Result <" << result << ">.");

    return result;
}

float ViewerOpenGLStrokeFont::textHeight()
{
    return charHeight_;
}

// Update new geometry in OpenGL.
void ViewerOpenGLStrokeFont::update()
{
    deleteGL();
}

// New build.
void ViewerOpenGLStrokeFont::updateGL(ViewerOpenGLManager *managerGL)
{
    LOG_DEBUG(<< "Update.");
    ASSERT(managerGL);

    GLuint displayList;

    registerGL(managerGL);
    initGL(managerGL);

    displayList = managerGL->createResource(this,
                                            displayListId_,
                                            ViewerOpenGLManager::DisplayList,
                                            static_cast<GLsizei>(nChars_));
    if (glIsList(displayList) != GL_TRUE)
    {
        EXIT;
    }

    LOG_DEBUG(<< "Created displayList <" << displayList << ">.");
    createStrokeFont(displayList);
}

void ViewerOpenGLStrokeFont::createStrokeFont(GLuint listId)
{
    LOG_DEBUG(<< "Create stroke font.");

    // Based on www.opengl.org/resources/code/samples/redbook/stroke.c
    // some geometry data taken from:
    // http://local.wasp.uwa.edu.au/~pbourke/dataformats/hershey/
    //      The Hershey set of vectors is one of the standard descriptions
    //      in the public domain. They were originally created by
    //      Dr. A. V. Hershey while working at the U. S. National
    //      Bureau of Standards.

    int i;

    GLuint currentChar = 32;
    GLuint maxChar = 126;

    const float end = -100.0;
    const float up = 100.0;
    const float endMark = -99.0;
    const float upMark = 99.0;

    // clang-format off
    static float charset[] = {
    /* ' ' 32 */  end, 16,
    /* '!' 33 */   5,21, 5, 7,up, 5, 2, 4, 1, 5, 0, 6, 1, 5, 2, end,10,
                  
    /* '"' 34 */   4,21, 4,14,up,12,21,12,14,end,16,
   
    /* '#' 35 */   11,25, 4,-7,up,17,25,10,-7,up, 4,12,18,12,up, 3, 6,17, 6,
                   end,21,
   
    /* '$' 36 */   8,25, 8,-4,up,12,25,12,-4,up,17,18,15,20,12,21, 8,21, 5,20,
                   3,18, 3,16, 4,14, 5,13, 7,12,13,10,15, 9,16, 8,17, 6,17, 3,
                  15, 1,12, 0, 8, 0, 5, 1, 3, 3,end,20,
   
    /* '%' 37 */  21,21, 3, 0,up, 8,21,10,19,10,17, 9,15, 7,14, 5,14, 3,16, 3,
                  18, 4,20, 6,21, 8,21,10,20,13,19,16,19,19,20,21,21,up,17, 7,
                  15, 6,14, 4,14, 2,16, 0,18, 0,20, 1,21, 3,21, 5,19, 7,17, 7,
                  end,24,
   
    /* '&' 38 */  23,12,23,13,22,14,21,14,20,13,19,11,17, 6,15, 3,13, 1,11, 0,
                   7, 0, 5, 1, 4, 2, 3, 4, 3, 6, 4, 8, 5, 9,12,13,13,14,14,16,
                  14,18,13,20,11,21, 9,20, 8,18, 8,16, 9,13,11,10,16, 3,18, 1,
                  20, 0,22, 0,23, 1,23, 2,end,26,
   
    /* ''' 39 */   5,19, 4,20, 5,21, 6,20, 6,18, 5,16, 4,15,end,10,
   
    /* '(' 40 */  11,25, 9,23, 7,20, 5,16, 4,11, 4, 7, 5, 2, 7,-2, 9,-5,11,-7,
                   end,14,
   
    /* ')' 41 */   3,25, 5,23, 7,20, 9,16,10,11,10, 7, 9, 2, 7,-2, 5,-5, 3,-7,
                   end,14,
   
    /* '*' 42 */   8,21, 8, 9,up, 3,18,13,12,up,13,18, 3,12,end,16,
   
    /* '+' 43 */   3,18,13, 0,up, 4, 9,22, 9,end,26,
   
    /* ',' 44 */   6, 1, 5, 0, 4, 1, 5, 2, 6, 1, 6,-1, 5,-3, 4,-4,end,10,
   
    /* '-' 45 */   4, 9,22, 9,end,26,
   
    /* '.' 46 */   5, 2, 4, 1, 5, 0, 6, 1, 5, 2,end,10,
   
    /* '/' 47 */  20,25, 2,-7,end,22,
                  
    /* '0' 48 */   9,21, 6,20, 4,17, 3,12, 3, 9, 4, 4, 6, 1, 9, 0,11, 0,14,
                   1,16, 4,17, 9,17,12,16,17,14,20,11,21, 9,21,end,20,
   
    /* '1' 49 */   6,17, 8,18,11,21,11, 0,end,20,
   
    /* '2' 50 */   4,16, 4,17, 5,19, 6,20, 8,21,12,21,14,20,15,19,16,17,16,
                  15,15,13,13,10, 3, 0,17, 0,end,20,
   
    /* '3' 51 */   5,21,16,21,10,13,13,13,15,12,16,11,17, 8,17, 6,16, 3,14,
                   1,11, 0, 8, 0, 5, 1, 4, 2, 3, 4,end,20,
                   
    /* '4' 52 */  13,21, 3, 7,18, 7,up,13,21,13, 0,end,20,
   
    /* '5' 53 */  15,21, 5,21, 4,12, 5,13, 8,14,11,14,14,13,16,11,17, 8,17,
                   6,16, 3,14, 1,11, 0, 8, 0, 5, 1, 4, 2, 3, 4,end,20,
                   
    /* '6' 54 */  16,18,15,20,12,21,10,21, 7,20, 5,17, 4,12, 4, 7, 5, 3, 7, 1,
                  10, 0,11, 0,14, 1,16, 3,17, 6,17, 7,16,10,14,12,11,13,10,13,
                   7,12, 5,10, 4, 7,end,20,
                   
    /* '7' 55 */  17,21, 7, 0,up, 3,21,17,21,end,20,
   
    /* '8' 56 */   8,21, 5,20, 4,18, 4,16, 5,14, 7,13,11,12,14,11,16, 9,17,
                   7,17, 4,16, 2,15, 1,12, 0, 8, 0, 5, 1, 4, 2, 3, 4, 3, 7,
                   4, 9, 6,11, 9,12,13,13, 15,14,16,16,16,18,15,20,12,21,
                   8,21,end,20,
                   
    /* '9' 57 */  16,14,15,11,13, 9,10, 8, 9, 8, 6, 9, 4,11, 3,14, 3,15,
                   4,18, 6,20, 9,21,10,21,13,20,15,18,16,14,16, 9,15, 4,13,
                   1,10, 0, 8, 0, 5, 1, 4, 3,end,20,
                   
    /* ':' 58 */   5,14, 4,13, 5,12, 6,13, 5,14,up, 5, 2, 4, 1, 5, 0, 6, 1,
                   5, 2, end,10,
                   
    /* ';' 59 */   5,14, 4,13, 5,12, 6,13, 5,14,up, 6, 1, 5, 0, 4, 1, 5, 2, 6,
                   1, 6,-1, 5,-3, 4,-4,end,10,
                   
    /* '<' 60 */  20,18, 4, 9,20, 0, end,24,
                   
    /* '=' 61 */   4,12,22,12,up, 4, 6,22, 6,end,26,
                   
    /* '>' 62 */   4,18,20, 9, 4, 0,end,24,
                   
    /* '?' 63 */   3,16, 3,17, 4,19, 5,20, 7,21,11,21,13,20,14,19,15,17,15,15,
                  14,13,13,12, 9,10, 9, 7,up, 9, 2, 8, 1, 9, 0,10, 1, 9, 2,
                  end,18,
   
    /* '@' 64 */  18,13,17,15,15,16,12,16,10,15, 9,14, 8,11, 8, 8, 9, 6,11, 5,
                  14, 5,16, 6,17, 8,up,12,16,10,14, 9,11, 9, 8,10, 6,11, 5,up,
                  18,16,17, 8,17, 6,19, 5,21, 5,23, 7,24,10,24,12,23,15,22,17,
                  20,19,18,20,15,21,12,21, 9,20, 7,19, 5,17, 4,15, 3,12, 3, 9,
                   4, 6, 5, 4, 7, 2, 9, 1,12, 0,15, 0,18, 1,20, 2,21, 3,up,19,
                  16,18, 8,18, 6,19, 5,end,27,
   
    /* 'A' 65 */   9,21, 1, 0,up, 9,21,17, 0,up, 4, 7,14, 7,end,18,
                   
    /* 'B' 66 */   4,21, 4, 0,up, 4,21,13,21,16,20,17,19,18,17,18,15,17,13,16,
                  12,13,11,up, 4,11,13,11,16,10,17, 9,18, 7,18, 4,17, 2,16, 1,
                  13, 0, 4, 0,end,21, 
   
    /* 'C' 67 */  18,16,17,18,15,20,13,21, 9,21, 7,20, 5,18, 4,16, 3,13, 3, 8,
                   4, 5, 5, 3, 7, 1, 9, 0,13, 0,15, 1,17, 3,18, 5,end,21,
    
    /* 'D' 68 */   4,21, 4, 0,up, 4,21,11,21,14,20,16,18,17,16,18,13,18, 8,17,
                   5,16, 3,14, 1,11, 0, 4, 0,end,21,
    
    /* 'E' 69 */   4,21, 4, 0,up, 4,21,17,21,up, 4,11,12,11,up, 4, 0,17, 0,
                   end,19,
    
    /* 'F' 70 */   4,21, 4, 0,up, 4,21,17,21,up, 4,11,12,11,end,18,
    
    /* 'G' 71 */  18,16,17,18,15,20,13,21, 9,21, 7,20, 5,18, 4,16, 3,13, 3, 8,
                   4, 5, 5, 3, 7, 1, 9, 0,13, 0,15, 1,17, 3,18, 5,18, 8,up,13,
                   8,18, 8,end,21,
    
    /* 'H' 72 */    4,21, 4, 0,up,18,21,18, 0,up, 4,11,18,11,end,22,
    
    /* 'I' 73 */    4,21, 4, 0,end,8,
    
    /* 'J' 74 */   12,21,12, 5,11, 2,10, 1, 8, 0, 6, 0, 4, 1, 3, 2, 2, 5, 2, 7,
                    end,16,   
    
    /* 'K' 75 */    4,21, 4, 0,up,18,21, 4, 7,up, 9,12,18, 0,end,21,
    
    /* 'L' 76 */    4,21, 4, 0,up, 4, 0,16, 0,end,17,
    
    /* 'M' 77 */    4,21, 4, 0,up, 4,21,12, 0,up,20,21,12, 0,up,20,21,20, 0,
                    end,24,
    
    /* 'N' 78 */    4,21, 4, 0,up, 4,21,18, 0,up,18,21,18, 0,end,22,
    
    /* 'O' 79 */    9,21, 7,20, 5,18, 4,16, 3,13, 3, 8, 4, 5, 5, 3, 7, 1, 9, 0,
                   13, 0,15, 1,17, 3,18, 5,19, 8,19,13,18,16,17,18,15,20,13,21,
                    9,21,end,22,
   
    /* 'P' 80 */    4,21, 4, 0,up, 4,21,13,21,16,20,17,19,18,17,18,14,17,12,16,
                   11,13,10, 4,10,end,21,
   
    /* 'Q' 81 */    9,21, 7,20, 5,18, 4,16, 3,13, 3, 8, 4, 5, 5, 3, 7, 1, 9, 0,
                   13, 0,15, 1,17, 3,18, 5,19, 8,19,13,18,16,17,18,15,20,13,21,
                    9,21,up,12, 4,18,-2,end,22,
   
    /* 'R' 82 */    4,21, 4, 0,up, 4,21,13,21,16,20,17,19,18,17,18,15,17,13,16,
                   12,13,11, 4,11,up,11,11,18, 0,end,21,
   
    /* 'S' 83 */   17,18,15,20,12,21, 8,21, 5,20, 3,18, 3,16, 4,14, 5,13, 7,12,
                   13,10,15, 9,16, 8,17, 6,17, 3,15, 1,12, 0, 8, 0, 5, 1, 3, 3,
                   end,20,
    
    /* 'T' 84 */    8,21, 8, 0,up, 1,21,15,21,end,16,
    
    /* 'U' 85 */    4,21, 4, 6, 5, 3, 7, 1,10, 0,12, 0,15, 1,17, 3,18, 6,18,21,
                    end,22,
   
    /* 'V' 86 */    1,21, 9, 0,up,17,21, 9, 0,end,18,
   
    /* 'W' 87 */    2,21, 7, 0,up,12,21, 7, 0,up,12,21,17, 0,up,22,21,17, 0,
                    end,24,
   
    /* 'X' 88 */    3,21,17, 0,up,17,21, 3, 0,end,20,
   
    /* 'Y' 89 */    1,21, 9,11, 9, 0,up,17,21, 9,11,end,18,
   
    /* 'Z' 90 */   17,21, 3, 0,up, 3,21,17,21,up, 3, 0,17, 0,end,20,
   
    /* '[' 91 */    4,25, 4,-7,up, 5,25, 5,-7,up, 4,25,11,25,up, 4,-7,11,-7,
                    end,14,
   
    /* '\' 92 */    0,21,14,-3,end,14,
   
    /* ']' 93 */    9,25, 9,-7,up,10,25,10,-7,up, 3,25,10,25,up, 3,-7,10,-7,
                    end,14,
   
    /* '^' 94 */    6,15, 8,18,10,15,up, 3,12, 8,17,13,12,up, 8,17, 8, 0,end,16,
   
    /* '_' 95 */    0,-2,16,-2,end,16,
   
    /* '`' 96 */    6,21, 5,20, 4,18, 4,16, 5,15, 6,16, 5,17,end,10,
   
    /* 'a' 97 */   15,14,15, 0,up,15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8,
                    3, 6, 4, 3, 6, 1, 8, 0,11, 0,13, 1,15, 3,end,19,
   
    /* 'b' 98 */    4,21, 4, 0,up, 4,11, 6,13, 8,14,11,14,13,13,15,11,16, 8,
                   16, 6,15, 3,13, 1,11, 0, 8, 0, 6, 1, 4, 3,end,19,
   
    /* 'c' 99 */   15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8, 3, 6, 4, 3, 6,
                    1, 8, 0,11, 0,13, 1,15, 3,end,18,
   
    /* 'd' 100*/   15,21,15, 0,up,15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8,
                    3, 6, 4, 3, 6, 1, 8, 0,11, 0,13, 1,15, 3,end,19,
   
    /* 'e' 101*/    3, 8,15, 8,15,10,14,12,13,13,11,14, 8,14, 6,13, 4,11, 3,
                    8, 3, 6, 4, 3, 6, 1, 8, 0,11, 0,13, 1,15, 3,end,18,
   
    /* 'f' 102*/   10,21, 8,21, 6,20, 5,17, 5, 0,up, 2,14, 9,14,end,12,
   
    /* 'g' 103*/   15,14,15,-2,14,-5,13,-6,11,-7, 8,-7, 6,-6,up,15,11,13,13,
                   11,14, 8,14, 6,13, 4,11, 3, 8, 3, 6, 4, 3, 6, 1, 8, 0,11,
                   0,13, 1,15, 3,end,19,
   
    /* 'h' 104*/    4,21, 4, 0,up, 4,10, 7,13, 9,14,12,14,14,13,15,10,15, 0,
                    end,19,
   
    /* 'i' 105*/    3,21, 4,20, 5,21, 4,22, 3,21,up, 4,14, 4, 0,end,8,
   
    /* 'j' 106*/    5,21, 6,20, 7,21, 6,22, 5,21,up, 6,14, 6,-3, 5,-6, 3,-7,
                    1,-7,end,10,
   
    /* 'k' 107*/    4,21, 4, 0,up,14,14, 4, 4,up, 8, 8,15, 0,end,17,
   
    /* 'l' 108*/    4,21, 4, 0,end,8,
   
    /* 'm' 109*/    4,14, 4, 0,up, 4,10, 7,13, 9,14,12,14,14,13,15,10,15, 0,
                   up,15,10,18,13,20,14,23,14,25,13,26,10,26, 0,end,30,
   
    /* 'n' 110*/    4,14, 4, 0,up, 4,10, 7,13, 9,14,12,14,14,13,15,10,15, 0,
                    end,19,
   
    /* 'o' 111*/    8,14, 6,13, 4,11, 3, 8, 3, 6, 4, 3, 6, 1, 8, 0,11, 0,13,
                    1,15, 3,16, 6,16, 8,15,11,13,13,11,14, 8,14,end,19,
   
    /* 'p' 112*/    4,14, 4,-7,up, 4,11, 6,13, 8,14,11,14,13,13,15,11,16, 8,
                   16, 6,15, 3,13, 1,11, 0, 8, 0, 6, 1, 4, 3,end,19,
   
    /* 'q' 113*/   15,14,15,-7,up,15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8,
                    3, 6, 4, 3, 6, 1, 8, 0,11, 0,13, 1,15, 3,end,19,
   
    /* 'r' 114*/    4,14, 4, 0,up, 4, 8, 5,11, 7,13, 9,14,12,14,end,13,
   
    /* 's' 115*/   14,11,13,13,10,14, 7,14, 4,13, 3,11, 4, 9, 6, 8,11, 7,13,
                    6,14, 4,14, 3,13, 1,10, 0, 7, 0, 4, 1, 3, 3,end,17,
   
    /* 't' 116*/    5,21, 5, 4, 6, 1, 8, 0,10, 0,up, 2,14, 9,14,end,12,
   
    /* 'u' 117*/    4,14, 4, 4, 5, 1, 7, 0,10, 0,12, 1,15, 4,up,15,14,15, 0,
                    end,19,
   
    /* 'v' 118*/    2,14, 8, 0,up,14,14, 8, 0,end,16,
   
    /* 'w' 119*/    3,14, 7, 0,up,11,14, 7, 0,up,11,14,15, 0,up,19,14,15, 0,
                    end,22,
   
    /* 'x' 120*/    3,14,14, 0,up,14,14, 3, 0,end,17,
   
    /* 'y' 121*/    2,14, 8, 0,up,14,14, 8, 0, 6,-4, 4,-6, 2,-7, 1,-7,end,16,
   
    /* 'z' 122*/   14,14, 3, 0,up, 3,14,14,14,up, 3, 0,14, 0,end,17,
   
    /* '{' 123*/    9,25, 7,24, 6,23, 5,21, 5,19, 6,17, 7,16, 8,14, 8,12, 6,10,
                   up, 7,24, 6,22, 6,20, 7,18, 8,17, 9,15, 9,13, 8,11, 4, 9,
                    8, 7, 9, 5, 9, 3, 8, 1, 7, 0, 6,-2, 6,-4, 7,-6,up, 6, 8,
                    8, 6, 8, 4, 7, 2, 6, 1, 5,-1, 5,-3, 6,-5, 7,-6, 9,-7,end,14,
   
    /* '|' 124*/    4,25, 4,-7,end,8,
   
    /* '}' 125*/    5,25, 7,24, 8,23, 9,21, 9,19, 8,17, 7,16, 6,14, 6,12, 8,10,
                   up, 7,24, 8,22, 8,20, 7,18, 6,17, 5,15, 5,13, 6,11,10, 9, 6,
                    7, 5, 5, 5, 3, 6, 1, 7, 0, 8,-2, 8,-4, 7,-6,up, 8, 8, 6, 6,
                    6, 4, 7, 2, 8, 1, 9,-1, 9,-3, 8,-5, 7,-6, 5,-7,end,14,
   
    /* '~' 126*/    3, 6, 3, 8, 4,11, 6,12, 8,12,10,11,14, 8,16, 7,18, 7,20, 8,
                   21,10,up, 3, 8, 4,10, 6,11, 8,11,10,10,14, 7,16, 6,18, 6,20,
                    7,21,10,21,12,end,24,
                    
                    end,20
    };
    // clang-format on

    charWidth_.resize(128);
    set(charWidth_, 0.0F);

    // Build the font.
    i = 0;
    SAFE_GL(glNewList(listId + currentChar, GL_COMPILE));
    glBegin(GL_LINE_STRIP);
    while (1)
    {
        if (charset[i] > upMark)
        {
            // Pen up.
            SAFE_GL(glEnd());
            glBegin(GL_LINE_STRIP);
            i += 1;
        }
        else if (charset[i] < endMark)
        {
            // End character.
            SAFE_GL(glEnd());

            // Offset to the next char = char width.
            glTranslatef(charset[i + 1] * 0.02f, 0.0, 0.0);
            SAFE_GL(glEndList());

            charWidth_[currentChar] = charset[i + 1] * 0.02f;

            currentChar++;
            if (currentChar < maxChar)
            {
                // Start new char.
                SAFE_GL(glNewList(listId + currentChar, GL_COMPILE));
                glBegin(GL_LINE_STRIP);
            }
            else
            {
                // All characters completed.
                break;
            }
            i += 2;
        }
        else
        {
            // Point.
            glVertex2f(charset[i] * 0.02, charset[i + 1] * 0.02);
            i += 2;
        }
    }
}
