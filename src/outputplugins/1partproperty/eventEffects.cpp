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

#include "eventEffects.hpp"
#include "../../base/is_simdata.hpp"
#include "../../extcode/xmlwriter.hpp"
#include "../../extcode/xmlParser.h"
#include <boost/foreach.hpp>
#include "../../dynamics/include.hpp"
#include "../../dynamics/interactions/include.hpp"
#include "../1partproperty/kenergy.hpp"

COPEventEffects::COPEventEffects(const DYNAMO::SimData* tmp, const XMLNode&):
  COutputPlugin(tmp,"CollisionMatrix")
{
}

void 
COPEventEffects::initialise()
{}

COPEventEffects::~COPEventEffects()
{}

void 
COPEventEffects::eventUpdate(const CIntEvent& iEvent, const C2ParticleData& Pdat)
{
  newEvent(iEvent.getType(),getIntClassKey(iEvent.getInteractionID()),
	   Pdat.particle1_.getDeltaKE(),
	   -Pdat.dP);

  newEvent(iEvent.getType(),getIntClassKey(iEvent.getInteractionID()),
	   Pdat.particle2_.getDeltaKE(),
	   Pdat.dP);
}


void 
COPEventEffects::eventUpdate(const CGlobEvent& globEvent, const CNParticleData& SDat)
{
  /*  BOOST_FOREACH(const C1ParticleData& pData, SDat.L1partChanges)
    newEvent(pData.getParticle().getID(), pData.getType(), 
	     getClassKey(globEvent));  
  
  BOOST_FOREACH(const C2ParticleData& pData, SDat.L2partChanges)
    {
      newEvent(pData.particle1_.getParticle().getID(), 
	       pData.getType(), getClassKey(globEvent));

      newEvent(pData.particle2_.getParticle().getID(), 
	       pData.getType(), getClassKey(globEvent));
	       }*/
}

void 
COPEventEffects::eventUpdate(const CLocalEvent& localEvent, const CNParticleData& SDat)
{
  /*BOOST_FOREACH(const C1ParticleData& pData, SDat.L1partChanges)
    newEvent(pData.getParticle().getID(), 
	     pData.getType(), getClassKey(localEvent));  
  
  BOOST_FOREACH(const C2ParticleData& pData, SDat.L2partChanges)
    {
      newEvent(pData.particle1_.getParticle().getID(), 
	       pData.getType(), getClassKey(localEvent));

      newEvent(pData.particle2_.getParticle().getID(),
	       pData.getType(), getClassKey(localEvent));
	       }*/
}

void 
COPEventEffects::eventUpdate(const CSystem& sysEvent, const CNParticleData& SDat, const Iflt&)
{
  /*BOOST_FOREACH(const C1ParticleData& pData, SDat.L1partChanges)
    newEvent(pData.getParticle().getID(), pData.getType(), getClassKey(sysEvent));
  
  BOOST_FOREACH(const C2ParticleData& pData, SDat.L2partChanges)
    {
      newEvent(pData.particle1_.getParticle().getID(), 
	       pData.getType(), getClassKey(sysEvent));

      newEvent(pData.particle2_.getParticle().getID(), 
	       pData.getType(), getClassKey(sysEvent));  
	       } */
}


void 
COPEventEffects::newEvent(const EEventType& eType, const classKey& ck, 
			  const Iflt& deltaKE, const CVector<>& delP)
{
  counterData& ref(counters[eventKey(ck, eType)]);
  ref.energyLoss += deltaKE;
  ref.momentumChange += delP;
}

void
COPEventEffects::output(xmlw::XmlStream &XML)
{
  /*  
  XML << xmlw::tag("CollCounters") 
      << xmlw::tag("TransitionMatrix");
  
  std::map<eventKey, std::pair<unsigned long long, Iflt> > totmap;
  
  typedef std::pair<const counterKey, counterData> locPair;
  
  
  size_t initialsum(0);
  
  typedef std::pair<eventKey,size_t> npair;
  BOOST_FOREACH(const npair& n, initialCounter)
    initialsum += n.second;
  
  BOOST_FOREACH(const locPair& ele, counters)
    {
      XML << xmlw::tag("Count")
	  << xmlw::attr("Event") << CIntEvent::getCollEnumName(ele.first.first.second)
	  << xmlw::attr("Name") << getName(ele.first.first.first, Sim)
	  << xmlw::attr("lastEvent") << CIntEvent::getCollEnumName(ele.first.second.second)
	  << xmlw::attr("lastName") << getName(ele.first.second.first, Sim)
	  << xmlw::attr("Percent") << 100.0 * ((Iflt) ele.second.count) 
	/ ((Iflt) totalCount)
	  << xmlw::attr("mft") << ele.second.totalTime
	/ (Sim->Dynamics.units().unitTime() * ((Iflt) ele.second.count))
	  << xmlw::endtag("Count");
      
      //Add the total count
      totmap[ele.first.first].first += ele.second.count;
      
      //Add the rate
      totmap[ele.first.first].second += ((Iflt) ele.second.count) 
	/ ele.second.totalTime;
    }
  
  XML << xmlw::endtag("TransitionMatrix")
      << xmlw::tag("Totals");
  
  typedef std::pair<eventKey, std::pair<unsigned long long, Iflt> > mappair;
  
  BOOST_FOREACH(const mappair& mp1, totmap)
    XML << xmlw::tag("TotCount")
	<< xmlw::attr("Name") << getName(mp1.first.first, Sim)
	<< xmlw::attr("Event") << CIntEvent::getCollEnumName(mp1.first.second)
	<< xmlw::attr("Percent") 
	<< 100.0 * (((Iflt) mp1.second.first)
		    +((Iflt) initialCounter[mp1.first]))
    / (((Iflt) totalCount) + ((Iflt) initialsum))
	<< xmlw::attr("Count") << mp1.second.first + initialCounter[mp1.first]
	<< xmlw::attr("EventMeanFreeTime")
	<< Sim->dSysTime / ((mp1.second.first + initialCounter[mp1.first])
			    * Sim->Dynamics.units().unitTime())
	<< xmlw::endtag("TotCount");
  
  XML << xmlw::endtag("Totals")
  << xmlw::endtag("CollCounters");*/
}