/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include "RenderObj.hpp"
#include <vector>

#include <magnet/CL/GLBuffer.hpp>

class RLines : public RenderObj
{
public:
  RLines(size_t N, std::string name);
  ~RLines();

  virtual void glRender();
  virtual void initOpenGL();
  virtual void initOpenCL();

  void setGLColors(std::vector<cl_uchar4>& VertexColor);
  void setGLPositions(std::vector<float>& VertexPos);
  void setGLElements(std::vector<int>& Elements);

  void initOCLVertexBuffer(cl::Context Context);
  void initOCLColorBuffer(cl::Context Context);
  void initOCLElementBuffer(cl::Context Context);

  magnet::GL::Buffer& getVertexGLData() { return _posBuff; }
  magnet::GL::Buffer& getColorGLData() { return _colBuff; }

  virtual void releaseCLGLResources();

protected:
  size_t _N;
  magnet::GL::Buffer _colBuff;
  cl::GLBuffer _clbuf_Colors;
  
  magnet::GL::Buffer _posBuff;
  cl::GLBuffer _clbuf_Positions;
  
  magnet::GL::Buffer _elementBuff;
  cl::GLBuffer _clbuf_Elements;
};
