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

#include "dumbsched.hpp"
#include "../dynamics/interactions/intEvent.hpp"
#include "../simulation/particle.hpp"
#include "../dynamics/dynamics.hpp"
#include "../dynamics/liouvillean/liouvillean.hpp"
#include "../dynamics/BC/BC.hpp"
#include "../dynamics/BC/LEBC.hpp"
#include "../base/is_simdata.hpp"
#include "../dynamics/globals/globEvent.hpp"
#include "../dynamics/systems/system.hpp"
#include "../dynamics/globals/global.hpp"
#include "../dynamics/globals/globEvent.hpp"
#include "../dynamics/globals/neighbourList.hpp"
#include "../dynamics/locals/local.hpp"
#include "../dynamics/locals/localEvent.hpp"
#include <magnet/xmlreader.hpp>
#include <cmath> //for huge val

CSDumb::CSDumb(const magnet::xml::Node& XML, dynamo::SimData* const Sim):
  CScheduler(Sim,"DumbScheduler", NULL)
{ 
  dout << "Dumb Scheduler Algorithmn" << std::endl;
  operator<<(XML);
}

CSDumb::CSDumb(dynamo::SimData* const Sim, CSSorter* ns):
  CScheduler(Sim,"DumbScheduler", ns)
{ dout << "Dumb Scheduler Algorithmn" << std::endl; }

void 
CSDumb::operator<<(const magnet::xml::Node& XML)
{
  sorter.set_ptr(CSSorter::getClass(XML.getNode("Sorter"), Sim));
}

void
CSDumb::initialise()
{
  dout << "Reinitialising on collision " << Sim->eventCount << std::endl;
  
  sorter->clear();
  sorter->resize(Sim->N+1);
  eventCount.clear();
  eventCount.resize(Sim->N+1, 0);
  
  //Now initialise the interactions
  BOOST_FOREACH(const Particle& part, Sim->particleList)
    addEvents(part);
  
  sorter->init();
  rebuildSystemEvents();
}

void 
CSDumb::rebuildList()
{
#ifdef DYNAMO_DEBUG
  initialise();
#else
  sorter->clear();
  sorter->resize(Sim->N+1);
  eventCount.clear();
  eventCount.resize(Sim->N+1, 0);
  
  //Now initialise the interactions
  BOOST_FOREACH(const Particle& part, Sim->particleList)
    addEvents(part);
  
  sorter->rebuild();
  rebuildSystemEvents();
#endif
}


void 
CSDumb::outputXML(magnet::xml::XmlStream& XML) const
{
  XML << magnet::xml::attr("Type") << "Dumb"
      << magnet::xml::tag("Sorter")
      << sorter
      << magnet::xml::endtag("Sorter");
}

void 
CSDumb::addEvents(const Particle& part)
{  
  Sim->dynamics.getLiouvillean().updateParticle(part);

  //Add the global events
  BOOST_FOREACH(const magnet::ClonePtr<Global>& glob, Sim->dynamics.getGlobals())
    if (glob->isInteraction(part))
      sorter->push(glob->getEvent(part), part.getID());
  
  //Add the local cell events
  BOOST_FOREACH(const magnet::ClonePtr<Local>& local, Sim->dynamics.getLocals())
    addLocalEvent(part, local->getID());

  //Add the interaction events
  BOOST_FOREACH(const Particle& part2, Sim->particleList)
    if (part2 != part)
      addInteractionEvent(part, part2.getID());
}
