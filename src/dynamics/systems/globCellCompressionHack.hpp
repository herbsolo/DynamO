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

#ifndef CSGlobCellHack_HPP
#define CSGlobCellHack_HPP

#include "system.hpp"
#include "../../datatypes/vector.hpp"

class CSGlobCellHack: public CSystem
{
public:
  CSGlobCellHack(DYNAMO::SimData*, Iflt);
  
  virtual CSystem* Clone() const { return new CSGlobCellHack(*this); }

  virtual void stream(Iflt);

  virtual CNParticleData runEvent();

  virtual void initialise(size_t);

  virtual void operator<<(const XMLNode&) {}

protected:
  virtual void outputXML(xmlw::XmlStream&) const {}
  
  Iflt growthRate;
  Iflt maxOrigDiam;
  CVector<long> cellCount;
  size_t cellID;
};

#endif