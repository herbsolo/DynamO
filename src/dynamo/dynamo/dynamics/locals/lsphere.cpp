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

#include "lsphere.hpp"
#include "../liouvillean/liouvillean.hpp"
#include "../liouvillean/NewtonL.hpp"
#include "localEvent.hpp"
#include "../NparticleEventData.hpp"
#include "../overlapFunc/CubePlane.hpp"
#include "../units/units.hpp"
#include "../../datatypes/vector.xml.hpp"
#include "../../schedulers/scheduler.hpp"


CLSphere::CLSphere(dynamo::SimData* nSim, double ne,
		       Vector  norigin, double nr, std::string nname, 
		       CRange* nRange, bool nrender):
  Local(nRange, nSim, "SphereWall"),
  vPosition(norigin),
  e(ne),
  radius(nr),
  r2(nr*nr),
  render(nrender)
{
  localName = nname;
}

CLSphere::CLSphere(const magnet::xml::Node& XML, dynamo::SimData* tmp):
  Local(tmp, "SphereWall")
{
  operator<<(XML);
}

LocalEvent 
CLSphere::getEvent(const Particle& part) const
{
#ifdef ISSS_DEBUG
  if (!Sim->dynamics.getLiouvillean().isUpToDate(part))
    M_throw() << "Particle is not up to date";
#endif

  Particle fakeParticle(vPosition, Vector(0,0,0),std::numeric_limits<size_t>::max());
    
  CPDData colldat(*Sim, part, fakeParticle);

  dynamic_cast<const LNewtonian&>(Sim->dynamics.getLiouvillean())
    . LNewtonian::SphereSphereOutRoot(colldat, r2,
				      part.testState(Particle::DYNAMIC), false);

  return LocalEvent(part, colldat.dt, WALL, *this);
}

void
CLSphere::runEvent(const Particle& part, const LocalEvent& iEvent) const
{
  ++Sim->eventCount;

  ///Run the collision and catch the data
  NEventData EDat(Sim->dynamics.getLiouvillean().runSphereWallCollision
  		      (part, vPosition, e));
  
  Sim->signalParticleUpdate(EDat);
  
  //Now we're past the event update the scheduler and plugins
  Sim->ptrScheduler->fullUpdate(part);
  
  BOOST_FOREACH(magnet::ClonePtr<OutputPlugin> & Ptr, Sim->outputPlugins)
    Ptr->eventUpdate(iEvent, EDat);
}

bool 
CLSphere::isInCell(const Vector & Origin, const Vector& CellDim) const
{
  return true;
}

void 
CLSphere::initialise(size_t nID)
{
  ID = nID;
}

void 
CLSphere::operator<<(const magnet::xml::Node& XML)
{
  range.set_ptr(CRange::getClass(XML,Sim));
  
  try {
    e = XML.getAttribute("Elasticity").as<double>();
    radius = XML.getAttribute("Radius").as<double>() * Sim->dynamics.units().unitLength();
    r2 = radius * radius;
    render = XML.getAttribute("Render").as<bool>();
    localName = XML.getAttribute("Name");
    vPosition << XML.getNode("Origin");
    vPosition *= Sim->dynamics.units().unitLength();
  } 
  catch (boost::bad_lexical_cast &)
    {
      M_throw() << "Failed a lexical cast in CLSphere";
    }
}

void 
CLSphere::outputXML(magnet::xml::XmlStream& XML) const
{
  XML << magnet::xml::attr("Type") << "SphereWall" 
      << magnet::xml::attr("Name") << localName
      << magnet::xml::attr("Elasticity") << e
      << magnet::xml::attr("Radius") << radius / Sim->dynamics.units().unitLength()
      << magnet::xml::attr("Render") << render
      << range
      << magnet::xml::tag("Origin")
      << vPosition / Sim->dynamics.units().unitLength()
      << magnet::xml::endtag("Origin");
}
