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
#include "../outputplugin.hpp"
#include "../../datatypes/vector.hpp"
#include <boost/circular_buffer.hpp>

class OPThermalConductivitySpeciesSpeciesE: public OutputPlugin
{
public:
  OPThermalConductivitySpeciesSpeciesE(const dynamo::SimData*, 
				       const magnet::xml::Node&);

  virtual void initialise();

  virtual void output(magnet::xml::XmlStream&);

  virtual OutputPlugin* Clone() const 
  { return new OPThermalConductivitySpeciesSpeciesE(*this); }
  
  virtual void eventUpdate(const GlobalEvent&, const NEventData&);

  virtual void eventUpdate(const LocalEvent&, const NEventData&);

  virtual void eventUpdate(const System&, const NEventData&, const double&); 
  
  virtual void eventUpdate(const IntEvent&, const PairEventData&);

  virtual void operator<<(const magnet::xml::Node&);

protected:
  std::vector<boost::circular_buffer<Vector  > > G;
  std::vector<std::vector<Vector   > > accG2;
  size_t count;
  std::vector<Vector  > constDelG;
  std::vector<Vector  > delG;
  double dt, currentdt;
  size_t currlen;
  bool notReady;

  size_t CorrelatorLength;

  void stream(const double&);

  void newG();

  void accPass();

  double rescaleFactor();
  
  void impulseDelG(const PairEventData&);

  void impulseDelG(const NEventData&);

  void updateConstDelG(const NEventData&);
  void updateConstDelG(const PairEventData&);
  void updateConstDelG(const ParticleEventData&);
};
