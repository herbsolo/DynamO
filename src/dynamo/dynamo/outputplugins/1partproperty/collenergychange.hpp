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
#include "1partproperty.hpp"
#include "../../datatypes/histogram.hpp"
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <vector>
#include <map>

class OPCollEnergyChange: public OP1PP
{
 public:
  OPCollEnergyChange(const dynamo::SimData*, const magnet::xml::Node&);

  void A1ParticleChange(const ParticleEventData&);

  void A2ParticleChange(const PairEventData&);

  void stream(const double&) {}

  void output(magnet::xml::XmlStream &); 

  void periodicOutput() {}

  virtual void initialise();

  virtual OutputPlugin *Clone() const 
  { return new OPCollEnergyChange(*this); }

  void operator<<(const magnet::xml::Node&);

 protected:  
  double binWidth; 

  static double KEBinWidth;

  struct histogram: public C1DHistogram
  {
    histogram(): C1DHistogram(OPCollEnergyChange::KEBinWidth) {}
  };

  typedef boost::tuple<size_t, size_t, EEventType> mapkey;

  std::map<mapkey, histogram> collisionKE;

  std::vector<C1DHistogram> data;
  C1DHistogram specialhist;
};
