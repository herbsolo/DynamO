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

#include <gtkmm.h>

#include "Quads.hpp"
#include <magnet/GL/texture.hpp>
#include <magnet/GL/shader/volume.hpp>
#include <magnet/gtk/transferFunction.hpp>
#include <memory>

namespace coil {
  class RVolume : public RQuads
  {
  public:
    RVolume(std::string name);
    ~RVolume();
  
    virtual void initOpenGL();
    virtual void initOpenCL();
    virtual void initGTK();

    virtual void glRender() { RQuads::glRender(); }

    virtual void glRender(magnet::GL::FBO& fbo);

    virtual void resize(size_t width, size_t height);

    virtual void releaseCLGLResources();

    virtual void showControls(Gtk::ScrolledWindow* win);

    void loadRawFile(std::string filename, size_t width, size_t height, 
		     size_t depth, size_t bytes);
    void loadSphereTestPattern();

    void loadData(const std::vector<GLubyte>& inbuffer, size_t width, size_t height, size_t depth);

  protected:
    void guiUpdate();

    void transferFunctionUpdated();

    magnet::GL::shader::VolumeShader _shader;
    std::auto_ptr<magnet::GL::FBO> _fbo;

    magnet::GL::Texture3D _data;
    magnet::GL::Texture1D _transferFuncTexture;

    //GTK gui stuff
    std::auto_ptr<Gtk::VBox> _optList;
    std::auto_ptr<Gtk::Entry> _stepSize;
    std::auto_ptr<Gtk::HScale> _diffusiveLighting;
    std::auto_ptr<Gtk::HScale> _specularLighting;
    std::auto_ptr<Gtk::HScale> _ditherRay;
    std::auto_ptr<magnet::gtk::TransferFunction> _transferFunction;
    GLfloat _stepSizeVal;
  };
}
