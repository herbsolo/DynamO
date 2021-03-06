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

#include "lwall.hpp"
#include "../liouvillean/liouvillean.hpp"
#include "localEvent.hpp"
#include "../NparticleEventData.hpp"
#include "../overlapFunc/CubePlane.hpp"
#include "../units/units.hpp"
#include "../../datatypes/vector.xml.hpp"
#include "../../schedulers/scheduler.hpp"


CLWall::CLWall(dynamo::SimData* nSim, double ne, Vector  nnorm, 
	       Vector  norigin, std::string nname, 
	       CRange* nRange, bool nrender):
  Local(nRange, nSim, "LocalWall"),
  vNorm(nnorm),
  vPosition(norigin),
  e(ne),
  render(nrender)
{
  localName = nname;
}

CLWall::CLWall(const magnet::xml::Node& XML, dynamo::SimData* tmp):
  Local(tmp, "LocalWall")
{
  operator<<(XML);
}

LocalEvent 
CLWall::getEvent(const Particle& part) const
{
#ifdef ISSS_DEBUG
  if (!Sim->dynamics.getLiouvillean().isUpToDate(part))
    M_throw() << "Particle is not up to date";
#endif

  return LocalEvent(part, Sim->dynamics.getLiouvillean().getWallCollision
		     (part, vPosition, vNorm), WALL, *this);
}

void
CLWall::runEvent(const Particle& part, const LocalEvent& iEvent) const
{
  ++Sim->eventCount;

  //Run the collision and catch the data
  NEventData EDat(Sim->dynamics.getLiouvillean().runWallCollision
		      (part, vNorm, e));

  Sim->signalParticleUpdate(EDat);

  //Now we're past the event update the scheduler and plugins
  Sim->ptrScheduler->fullUpdate(part);
  
  BOOST_FOREACH(magnet::ClonePtr<OutputPlugin> & Ptr, Sim->outputPlugins)
    Ptr->eventUpdate(iEvent, EDat);
}

bool 
CLWall::isInCell(const Vector & Origin, const Vector& CellDim) const
{
  return dynamo::OverlapFunctions::CubePlane(Origin, CellDim, vPosition, vNorm);
}

void 
CLWall::initialise(size_t nID)
{
  ID = nID;
}

void 
CLWall::operator<<(const magnet::xml::Node& XML)
{
  range.set_ptr(CRange::getClass(XML,Sim));
  
  try {
    e = XML.getAttribute("Elasticity").as<double>();
    render = XML.getAttribute("Render").as<bool>();
    magnet::xml::Node xBrowseNode = XML.getNode("Norm");
    localName = XML.getAttribute("Name");
    vNorm << xBrowseNode;
    vNorm /= vNorm.nrm();
    xBrowseNode = XML.getNode("Origin");
    vPosition << xBrowseNode;
    vPosition *= Sim->dynamics.units().unitLength();
  } 
  catch (boost::bad_lexical_cast &)
    {
      M_throw() << "Failed a lexical cast in CLWall";
    }
}

void 
CLWall::outputXML(magnet::xml::XmlStream& XML) const
{
  XML << magnet::xml::attr("Type") << "Wall" 
      << magnet::xml::attr("Name") << localName
      << magnet::xml::attr("Elasticity") << e
      << magnet::xml::attr("Render") << render
      << range
      << magnet::xml::tag("Norm")
      << vNorm
      << magnet::xml::endtag("Norm")
      << magnet::xml::tag("Origin")
      << vPosition / Sim->dynamics.units().unitLength()
      << magnet::xml::endtag("Origin");
}

void 
CLWall::checkOverlaps(const Particle& p1) const
{
  Vector pos(p1.getPosition() - vPosition);
  Sim->dynamics.BCs().applyBC(pos);

  double r = (pos | vNorm);
  
  if (r < 0)
    dout << "Possible overlap of " << r / Sim->dynamics.units().unitLength() << " for particle " << p1.getID()
	     << "\nWall Pos is [" 
	     << vPosition[0] << "," << vPosition[1] << "," << vPosition[2] 
	     << "] and Normal is [" 
	     << vNorm[0] << "," << vNorm[1] << "," << vNorm[2] << "]"
       << std::endl;
}
