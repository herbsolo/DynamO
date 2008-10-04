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

#include "2RChains.hpp"
#include "../../extcode/xmlwriter.hpp"
#include "../../extcode/xmlParser.h"
#include "../../simulation/particle.hpp"


C2RChains::C2RChains(unsigned long r1, unsigned long r2, unsigned long r3):
  range1(r1),range2(r2),interval(r3) 
{
  if ((r2-r1 + 1) % r3)
    D_throw() << "Range of C2RChains does not split evenly into interval";
}

C2RChains::C2RChains(const XMLNode& XML, const DYNAMO::SimData*):
  range1(0),range2(0), interval(0)
{ 
  if (strcmp(XML.getAttribute("Range"),"Chains"))
    D_throw() << "Attempting to load a chains from a non chains";
  
  range1 = boost::lexical_cast<unsigned long>(XML.getAttribute("Start"));
  range2 = boost::lexical_cast<unsigned long>(XML.getAttribute("End"));
  interval = boost::lexical_cast<unsigned long>(XML.getAttribute("Interval"));
  if ((range2-range1 + 1) % interval)
    D_throw() << "Range of C2RChains does not split evenly into interval";

}

bool 
C2RChains::isInRange(const CParticle&p1, const CParticle&p2) const
{
  if (p1.getID() > p2.getID())
    {
      if (p1.getID() - p2.getID() == 1)
	if ((p2.getID() >= range1) && (p1.getID() <= range2))
	  if (((p2.getID() - range1) / interval) == ((p1.getID() - range1) / interval))
	    return true;
    }
  else 
    if (p2.getID() - p1.getID() == 1)
      if ((p1.getID() >= range1) && (p2.getID() <= range2))
	if (((p1.getID()  - range1) / interval) == ((p2.getID()  - range1) / interval))
	  return true;
  
  return false;
}

void 
C2RChains::operator<<(const XMLNode&)
{
  D_throw() << "Due to problems with CRAll C2RChains::operator<< cannot work for this class";
}

void 
C2RChains::outputXML(xmlw::XmlStream& XML) const
{
  XML << xmlw::attr("Range") << "Chains" 
      << xmlw::attr("Start")
      << range1
      << xmlw::attr("End")
      << range2
      << xmlw::attr("Interval")
      << interval;
}

