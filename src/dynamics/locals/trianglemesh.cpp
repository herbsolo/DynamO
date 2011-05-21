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

#include "trianglemesh.hpp"
#include "../liouvillean/liouvillean.hpp"
#include "localEvent.hpp"
#include "../NparticleEventData.hpp"
#include "../overlapFunc/CubePlane.hpp"
#include "../units/units.hpp"
#include "../../datatypes/vector.xml.hpp"
#include "../../schedulers/scheduler.hpp"


LTriangleMesh::LTriangleMesh(dynamo::SimData* nSim, double e, std::string name, CRange* nRange):
  Local(nRange, nSim, "LocalWall"),
  _e(e)
{
  localName = name;
}

LTriangleMesh::LTriangleMesh(const magnet::xml::Node& XML, dynamo::SimData* tmp):
  Local(tmp, "LocalWall")
{
  operator<<(XML);
}

LocalEvent 
LTriangleMesh::getEvent(const Particle& part) const
{
#ifdef ISSS_DEBUG
  if (!Sim->dynamics.getLiouvillean().isUpToDate(part))
    M_throw() << "Particle is not up to date";
#endif

  double tmin = HUGE_VAL;
  size_t triangleid = 0;

  for (size_t id(0); id < _elements.size(); ++id)
    {
      double t = Sim->dynamics.getLiouvillean()
	.getParticleTriangleEvent(part,
				  _vertices[_elements[id].get<0>()],
				  _vertices[_elements[id].get<1>()],
				  _vertices[_elements[id].get<2>()]);
      if (t < tmin) { tmin = t; triangleid = id; } 
    }

  return LocalEvent(part, tmin, WALL, *this, triangleid);
}

void
LTriangleMesh::runEvent(const Particle& part, const LocalEvent& iEvent) const
{ 
  ++Sim->eventCount;
  
  const TriangleElements& e = _elements[iEvent.getExtraData()];

  Vector normal
    = (_vertices[e.get<1>()] - _vertices[e.get<0>()])
    ^ (_vertices[e.get<2>()] - _vertices[e.get<1>()]);

  normal /= normal.nrm();

  //Run the collision and catch the data
  NEventData EDat(Sim->dynamics.getLiouvillean().runWallCollision
		  (part, normal, _e));

  Sim->signalParticleUpdate(EDat);

  //Now we're past the event update the scheduler and plugins
  Sim->ptrScheduler->fullUpdate(part);
  
  BOOST_FOREACH(magnet::ClonePtr<OutputPlugin> & Ptr, Sim->outputPlugins)
    Ptr->eventUpdate(iEvent, EDat);
}

bool 
LTriangleMesh::isInCell(const Vector & Origin, const Vector& CellDim) const
{ return true; }

void 
LTriangleMesh::initialise(size_t nID)
{ ID = nID; }

void 
LTriangleMesh::operator<<(const magnet::xml::Node& XML)
{
  range.set_ptr(CRange::getClass(XML,Sim));
  
  try {
    _e = XML.getAttribute("Elasticity").as<double>();
    localName = XML.getAttribute("Name");

    {//Load the vertex coordinates
      std::istringstream is(XML.getNode("Vertices").getValue());
      is.exceptions(std::ostringstream::badbit | std::ostringstream::failbit);
      is.peek(); //Set the eof flag if needed
      Vector tmp;
      while (!is.eof())
	{
	  is >> tmp[0];
	  if (is.eof()) M_throw() << "The vertex coordinates is not a multiple of 3";

	  is >> tmp[1];
	  if (is.eof()) M_throw() << "The vertex coordinates is not a multiple of 3";

	  is >> tmp[2];	  
	  _vertices.push_back(tmp * Sim->dynamics.units().unitLength());
	}
    }

    {//Load the triangle elements
      std::istringstream is(XML.getNode("Elements").getValue());
      is.exceptions(std::ostringstream::badbit | std::ostringstream::failbit);
      is.peek(); //Set the eof flag if needed

      TriangleElements tmp;
      while (!is.eof())
	{
	  is >> tmp.get<0>();
	  if (is.eof()) M_throw() << "The triangle elements are not a multiple of 3";
	  
	  is >> tmp.get<1>();;
	  if (is.eof()) M_throw() << "The triangle elements are not a multiple of 3";

	  is >> tmp.get<2>();;

	  if ((tmp.get<0>() >= _vertices.size()) 
	      || (tmp.get<1>() >= _vertices.size()) 
	      || (tmp.get<2>() >= _vertices.size()))
	    M_throw() << "Triangle " << _elements.size() << " has an out of range vertex ID";

	  Vector normal
	    = (_vertices[tmp.get<1>()] - _vertices[tmp.get<0>()])
	    ^ (_vertices[tmp.get<2>()] - _vertices[tmp.get<1>()]);

	  if (normal.nrm() == 0) 
	    M_throw() << "Triangle " << _elements.size() << " has a zero normal!";


	  _elements.push_back(tmp);
	}
    }

  }
  catch (boost::bad_lexical_cast &)
    {
      M_throw() << "Failed a lexical cast in LTriangleMesh";
    }
}

void 
LTriangleMesh::outputXML(xml::XmlStream& XML) const
{
  XML << xml::attr("Type") << "TriangleMesh" 
      << xml::attr("Name") << localName
      << xml::attr("Elasticity") << _e
      << range;

  XML << xml::tag("Vertices") << xml::chardata();
  BOOST_FOREACH(Vector vert, _vertices)
    XML << vert[0] / Sim->dynamics.units().unitLength() << " " 
	<< vert[1] / Sim->dynamics.units().unitLength() << " "
	<< vert[2] / Sim->dynamics.units().unitLength() << "\n";
  XML << xml::endtag("Vertices");

  XML << xml::tag("Elements") << xml::chardata();
  BOOST_FOREACH(TriangleElements elements, _elements)
    XML << elements.get<0>() << " " 
	<< elements.get<1>() << " "
	<< elements.get<2>() << "\n";
  XML << xml::endtag("Elements");
}

void 
LTriangleMesh::checkOverlaps(const Particle& p1) const
{}


#ifdef DYNAMO_visualizer
# include <coil/RenderObj/TriangleMesh.hpp>

magnet::thread::RefPtr<RenderObj>& 
LTriangleMesh::getCoilRenderObj() const
{
  const double lengthRescale = 1 / Sim->primaryCellSize.maxElement();

  if (!_renderObj.isValid())
    {
      std::vector<float> verts;
      verts.reserve(3 * _vertices.size());
      BOOST_FOREACH(const Vector& v, _vertices)
	{
	  verts.push_back(v[0] * lengthRescale);
	  verts.push_back(v[1] * lengthRescale);
	  verts.push_back(v[2] * lengthRescale);
	}

      std::vector<int> elems;
      elems.reserve(3 * _elements.size());
      BOOST_FOREACH(const TriangleElements& e, _elements)
	{
	  elems.push_back(e.get<0>());
	  elems.push_back(e.get<1>());
	  elems.push_back(e.get<2>());
	}
      
      _renderObj = new RTriangleMesh(getName(), verts, elems);
    }
  
  return _renderObj;
}

void 
LTriangleMesh::updateRenderData(magnet::CL::CLGLState&) const
{}
#endif
