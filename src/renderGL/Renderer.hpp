/*
 * Copyright (C) 2020 of Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class to manage all draw functions

#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include <GL/glew.h>
#include <vulkan/vulkan.h>

class VertexArray;
class shaderProgram;

class Renderer {
public:
    static void clearBuffer();
    static void clearColor();
    static void clearDepthBuffer();
    static void drawArrays(shaderProgram* shader, VertexArray* va, VkPrimitiveTopology mode, unsigned int first, unsigned int count);
    static void drawArraysWithoutShader(VertexArray* va, VkPrimitiveTopology mode, unsigned int first, unsigned int count);
    static void drawMultiArrays(shaderProgram* shader, VertexArray* va, VkPrimitiveTopology mode, unsigned int boucle, unsigned int count );
    static void drawElementsWithoutShader(VertexArray* va, VkPrimitiveTopology mode);
    static void viewport(int x, int y, int w, int h);
private:
    static bool needClearColor;
    static bool needClearDepthBuffer;
};

#endif
