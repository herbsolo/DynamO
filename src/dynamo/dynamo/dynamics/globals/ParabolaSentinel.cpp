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

#include "ParabolaSentinel.hpp"
#include "globEvent.hpp"
#include "../NparticleEventData.hpp"
#include "../../base/is_simdata.hpp"
#include "../liouvillean/liouvillean.hpp"
#include "../../schedulers/scheduler.hpp"
#include <magnet/xmlreader.hpp>

#ifdef DYNAMO_DEBUG 
#include <boost/math/special_functions/fpclassify.hpp>
#endif


CGParabolaSentinel::CGParabolaSentinel(dynamo::SimData* nSim, const std::string& name):
  Global(nSim, "ParabolaSentinel")
{
  globName = name;
  dout << "ParabolaSentinel Loaded" << std::endl;
}

void 
CGParabolaSentinel::initialise(size_t nID)
{
  ID=nID;
}

GlobalEvent 
CGParabolaSentinel::getEvent(const Particle& part) const
{
  Sim->dynamics.getLiouvillean().updateParticle(part);

  return GlobalEvent(part, Sim->dynamics.getLiouvillean()
		     .getParabolaSentinelTime(part), 
		     VIRTUAL, *this);
}

void 
CGParabolaSentinel::runEvent(const Particle& part, const double) const
{
  Sim->dynamics.getLiouvillean().updateParticle(part);

  GlobalEvent iEvent(getEvent(part));

  if (iEvent.getdt() == HUGE_VAL)
    {
      //We've numerically drifted slightly passed the parabola, so
      //just reschedule the particles events, no need to enforce anything
      Sim->ptrScheduler->fullUpdate(part);
      return;
    }

#ifdef DYNAMO_DEBUG 
  if (boost::math::isnan(iEvent.getdt()))
    M_throw() << "A NAN Interaction collision time has been found when recalculating this global"
	      << iEvent.stringData(Sim);
#endif

  Sim->dSysTime += iEvent.getdt();
    
  Sim->ptrScheduler->stream(iEvent.getdt());
  
  Sim->dynamics.stream(iEvent.getdt());

  Sim->dynamics.getLiouvillean().enforceParabola(part);
  
#ifdef DYNAMO_DEBUG
  iEvent.addTime(Sim->freestreamAcc);
  
  Sim->freestreamAcc = 0;

  NEventData EDat(ParticleEventData(part, Sim->dynamics.getSpecies(part), VIRTUAL));

  Sim->signalParticleUpdate(EDat);

  BOOST_FOREACH(magnet::ClonePtr<OutputPlugin> & Ptr, Sim->outputPlugins)
    Ptr->eventUpdate(iEvent, EDat);
#else
  Sim->freestreamAcc += iEvent.getdt();
#endif

  Sim->ptrScheduler->fullUpdate(part);
}
