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

#include "MFL.hpp"
#include <boost/foreach.hpp>
#include "../../base/is_simdata.hpp"
#include "../../dynamics/dynamics.hpp"
#include "../../dynamics/species/species.hpp"
#include "../../dynamics/1particleEventData.hpp"
#include "../../dynamics/units/units.hpp"

COPMFL::COPMFL(const DYNAMO::SimData* tmp):
  COP1PP(tmp,"MeanFreeLength", 250)
{}

void
COPMFL::initialise()
{
  lastTime.resize(Sim->lN, 0.0);
  data.resize(Sim->Dynamics.getSpecies().size(), C1DHistogram(Sim->Dynamics.units().unitLength() * 0.01));
  SLLODdata.resize(Sim->Dynamics.getSpecies().size(), C1DHistogram(Sim->Dynamics.units().unitLength() * 0.01));
}

void 
COPMFL::A1ParticleChange(const C1ParticleData& PDat)
{
  //We ignore stuff that hasn't had an event yet
  if (lastTime[PDat.getParticle().getID()] != 0.0)
    {
      data[PDat.getSpecies().getID()]
	.addVal(PDat.getParticle().getVelocity().length() 
	     * (Sim->dSysTime 
		- lastTime[PDat.getParticle().getID()]));

      CVector<> vel = PDat.getParticle().getVelocity();

      vel[0] -= PDat.getParticle().getPosition()[1];

      SLLODdata[PDat.getSpecies().getID()]
	.addVal(vel.length() 
	     * (Sim->dSysTime 
		- lastTime[PDat.getParticle().getID()]));
    }
  
  lastTime[PDat.getParticle().getID()] = Sim->dSysTime;
}

void
COPMFL::output(xmlw::XmlStream &XML)
{
  XML << xmlw::tag("MFL");
  
  for (size_t id = 0; id < data.size(); ++id)
    {
      XML << xmlw::tag("Species")
	  << xmlw::attr("Name")
	  << Sim->Dynamics.getSpecies()[id].getName();

      data[id].outputHistogram(XML, 1.0 / Sim->Dynamics.units().unitLength());
      
      XML << xmlw::tag("SLLOD");
      SLLODdata[id].outputHistogram
	(XML, 1.0 / Sim->Dynamics.units().unitLength());

      XML << xmlw::endtag("Species");
    }

  XML << xmlw::endtag("MFL");
}
