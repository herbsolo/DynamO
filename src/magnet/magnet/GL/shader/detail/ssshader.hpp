/*    dynamo:- Event driven molecular dynamics simulator 
 *    http://www.marcusbannerman.co.uk/dynamo
 *    Copyright (C) 2009  Marcus N Campbell Bannerman <m.bannerman@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    version 3 as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <magnet/GL/shader/detail/shader.hpp>
#include <sstream>
#define STRINGIFY(A) #A

namespace magnet {
  namespace GL {
    namespace shader {
      namespace detail {
	/*! \brief A base class for OpenGL Shaders implementing a
	 * Screen Space Filter.
	 *
	 * Screen space filters are filters that take a rendered image
	 * and apply an image transform using only the rendered image
	 * data (such as the pixel color and depth).
	 */
	class SSShader : public Shader
	{
	public:
	  /*! \brief Actually calls the shader function.
	   *
	   * Attaches the filter shader, renders a full screen quad
	   * to generate a fragment shader for each output pixel and
	   * then restores the fixed pipeline.
	   */
	  void invoke()
	  {
	    //Setup the shader arguments
	    glUseProgram(_shaderID);
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    drawScreenQuad();
	    //Restore the fixed pipeline
	    glUseProgramObjectARB(0);
	  }
	
	  /*! \brief A trivial passthrough vertex shader. */
	  virtual std::string initVertexShaderSource()
	  { return STRINGIFY(void main() { gl_Position = ftransform(); gl_TexCoord[0] = gl_MultiTexCoord0; }); }
	};
      }
    }
  }
}

#undef STRINGIFY
