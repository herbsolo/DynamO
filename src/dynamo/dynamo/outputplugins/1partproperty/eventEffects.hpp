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
#include "../../dynamics/eventtypes.hpp"
#include "../eventtypetracking.hpp"
#include <map>

using namespace EventTypeTracking;

class Particle;

class OPEventEffects: public OutputPlugin
{
private:
  
public:
  OPEventEffects(const dynamo::SimData*, const magnet::xml::Node&);
  ~OPEventEffects();

  virtual void initialise();

  virtual void eventUpdate(const IntEvent&, const PairEventData&);

  virtual void eventUpdate(const GlobalEvent&, const NEventData&);

  virtual void eventUpdate(const LocalEvent&, const NEventData&);

  virtual void eventUpdate(const System&, const NEventData&, const double&);

  void output(magnet::xml::XmlStream &);

  virtual OutputPlugin *Clone() const { return new OPEventEffects(*this); };

  //This is fine to replica exchange as the interaction, global and system lookups are done using id's
  virtual void changeSystem(OutputPlugin* plug) { std::swap(Sim, static_cast<OPEventEffects*>(plug)->Sim); }
  
 protected:
  typedef std::pair<classKey, EEventType> eventKey;

  void newEvent(const EEventType&, const classKey&, const double&, const Vector &);
  
  struct counterData
  {
    counterData():count(0), energyLoss(0), momentumChange(0,0,0) {}
    unsigned long count;
    double energyLoss;
    Vector  momentumChange;
  };
  
  std::map<eventKey, counterData> counters;
};
