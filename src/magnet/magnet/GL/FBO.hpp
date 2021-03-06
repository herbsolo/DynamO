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
#include <magnet/GL/texture.hpp>

namespace magnet {
  namespace GL {   
    /*! \brief A Frame Buffer Object.
     *
     * Frame buffer objects are "virtual screens" which can be drawn
     * to, but the output is captured by bound textures instead of the
     * real user screen.
     *
     * \sa MultisampledFBO
     */
    class FBO
    {
    public:
      /*! \brief Default constructor.
       * 
       * The FBO is unusable at this point and must be first \ref
       * init() ialized.
       */
      inline FBO(): _width(0), _height(0) {}

      /*! \brief Initializes the FBO
       * 
       * \param width The width of the FBO in pixels.
       * \param height The height of the FBO in pixels.
       * \param internalformat The pixel type stored by the FBO.
       */
      inline 
      virtual void init(GLsizei width, GLsizei height, GLint internalformat = GL_RGBA)
      {
	if (!GLEW_EXT_framebuffer_object)
	  M_throw() << "GL_EXT_framebuffer_object extension is not supported! Cannot do offscreen rendering!";

	if (_width || _height) 
	  M_throw() << "FBO has already been initialised!";

	if (!width || !height)
	  M_throw() << "Cannot initialise an FBO with a width or height == 0!";

	_internalformat = internalformat;
	_width = width;
	_height = height;

	//Build depth buffer
	_depthTexture.init(_width, _height, GL_DEPTH_COMPONENT24);
	//Where to put the information
	_depthTexture.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	_depthTexture.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	_depthTexture.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
	_depthTexture.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP);
	_depthTexture.parameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
	//////////////////////////////////////////////////////////////////////////

	//Build color texture
	_colorTexture.init(_width, _height, internalformat);
	_colorTexture.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	_colorTexture.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	_colorTexture.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffersEXT(1, &_FBO);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _FBO);

	//Bind the depth texture
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
				  GL_TEXTURE_2D, _depthTexture.getGLHandle(), 0);

	//Bind the color texture
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 
				  _colorTexture.getGLHandle(), 0);

	// check FBO status
	GLenum FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch (FBOstatus)
	  {
	  case GL_FRAMEBUFFER_UNDEFINED: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_UNDEFINED";
	  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
	  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
	  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
	  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
	  case GL_FRAMEBUFFER_UNSUPPORTED: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_UNSUPPORTED";
	  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
	  case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: M_throw() << "Failed to create FrameBufferObject: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
	  default: M_throw() << "Failed to create FrameBufferObject: Unkown error code";
	  case GL_FRAMEBUFFER_COMPLETE_EXT: break;
	  }
	
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      }
      
      /*! \brief Resizes the FBO
       * 
       * If the FBO has been initialized, this will resize the FBO by
       * first deinit()ializing the FBO and re-init()ializing with the
       * correct size. Just resizing the textures does not play well
       * with all GPU's.
       *
       * \param width The new width of the FBO in pixels.
       * \param height The new height of the FBO in pixels.
       */
      inline 
      virtual void resize(GLsizei width, GLsizei height)
      {
	//If we've not been initialised, then just return
	if (!_width) return;
	
	//Skip identity operations
	if ((_width == width) && (_height == height)) return;

	deinit();
	init(width, height, _internalformat);
      }

      /*! \brief Renders the contents of the FBO to the real screen FBO.
       * 
       * \param screenwidth The width of the screen in pixels.
       * \param screenheight The height of the screen in pixels.
       */
      inline void blitToScreen(GLsizei screenwidth, GLsizei screenheight)
      {
	if (!GLEW_EXT_framebuffer_blit)
	  M_throw() << "The GL_EXT_framebuffer_blit extension is not supported! Cannot blit!";
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _FBO);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBlitFramebufferEXT(0, 0, _width, _height, 0, 0, screenwidth, screenheight, 
			     GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
      }

      inline virtual ~FBO() { deinit(); }

      /*! \brief Releases the OpenGL resources of this FBO. */
      inline 
      virtual void deinit()
      {
	_colorTexture.deinit();
	_depthTexture.deinit();
	
	if (_width)
	  glDeleteFramebuffersEXT(1, &_FBO);

	_width = 0;
	_height = 0;
      }

      /*! \brief Attaches this FBO as the current render target. */
      inline 
      virtual void attach()
      {
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, _width, _height);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _FBO);
      }

      /*! \brief Restores the screen FBO as the current render target. */
      inline 
      virtual void detach()
      {
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      }

      /*! \brief Copies the contents of this FBO to another.
       *
       * \param other The FBO to copy this FBO's contents to.
       * \param opts Bit flags marking the channels of the FBO to copy.
       */
      inline 
      virtual void copyto(FBO& other, GLbitfield opts = GL_COLOR_BUFFER_BIT 
			  | GL_DEPTH_BUFFER_BIT)
      {
	//First blit between the two FBO's
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _FBO);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, other._FBO);
	glBlitFramebufferEXT(0, 0, _width, _height, 0, 0, 
			     other._width, other._height, opts, GL_NEAREST);
	
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
      }

      
      /*! \brief Fetch the underlying OpenGL handle. */
      inline GLuint getFBO() { return _FBO; }

      /*! \brief Fetch the texture bound to the color buffer. */
      inline Texture2D& getColorTexture() { return _colorTexture; }

      /*! \brief Fetch the texture bound to the depth buffer. */
      inline Texture2D& getDepthTexture() { return _depthTexture; }

      /*! \brief Fetch the width of the FBO in pixels. */
      inline GLsizei getWidth() { return _width; }

      /*! \brief Fetch the height of the FBO in pixels. */
      inline GLsizei getHeight() { return _height; }

    protected:
      Texture2D _colorTexture;
      Texture2D _depthTexture;
      GLuint _FBO;
      GLsizei _width, _height;
      GLint _internalformat;
    };
  }
}
