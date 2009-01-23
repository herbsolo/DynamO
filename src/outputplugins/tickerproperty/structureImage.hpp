/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2008  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#ifndef COPStructureImaging_H
#define COPStructureImaging_H

#include "ticker.hpp"

class COPStructureImaging: public COPTicker
{
 public:
  COPStructureImaging(const DYNAMO::SimData*, const XMLNode&);

  virtual COutputPlugin *Clone() const
  { return new COPStructureImaging(*this); }

  virtual void initialise();

  virtual void stream(Iflt) {}

  virtual void ticker();

  virtual void changeSystem(COutputPlugin*);

  virtual void operator<<(const XMLNode&);

  virtual void output(xmlw::XmlStream&);
  
 protected:

  void printImage();
  size_t id;
  size_t imageCount;
  std::vector<std::vector<CVector<> > > imagelist;
};

#endif