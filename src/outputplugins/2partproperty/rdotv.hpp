/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2010  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#ifndef OPRdotV_H
#define OPRdotV_H

#include "../outputplugin.hpp"
#include "../../datatypes/histogram.hpp"
#include "../eventtypetracking.hpp"
#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

using namespace EventTypeTracking;

class OPRdotV: public OutputPlugin
{
 public:
  OPRdotV(const DYNAMO::SimData*, const XMLNode&);

  virtual void initialise();
  
  virtual void eventUpdate(const CIntEvent&, const C2ParticleData&);

  virtual void eventUpdate(const CGlobEvent&, const CNParticleData&);

  virtual void eventUpdate(const CLocalEvent&, const CNParticleData&);

  virtual void eventUpdate(const CSystem&, const CNParticleData&, const Iflt&);

  void output(xmlw::XmlStream &);

  virtual void changeSystem(OutputPlugin* plug) { std::swap(Sim, static_cast<OPRdotV*>(plug)->Sim); }
  
  virtual OutputPlugin *Clone() const { return new OPRdotV(*this); };
  
 protected:
  struct mapdata
  {
    mapdata(): count(0), accRdotV(0.0), costheta(0.005) {}
    
    unsigned long count;

    Iflt accRdotV;

    void addVal(const Iflt& dval)
    { accRdotV += dval; ++count; }

    Iflt getAvg() const
    { return accRdotV / count; }

    C1DHistogram costheta;
  };
  
  typedef boost::tuple<EEventType, classKey, size_t, size_t> mapKey;

  std::map<mapKey, mapdata> rvdotacc;
};

#endif

