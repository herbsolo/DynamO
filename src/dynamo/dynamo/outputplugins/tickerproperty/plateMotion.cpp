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

#include "plateMotion.hpp"
#include "../../dynamics/include.hpp"
#include "../../base/is_simdata.hpp"
#include "../../dynamics/liouvillean/liouvillean.hpp"
#include "../../dynamics/interactions/squarebond.hpp"
#include "../../dynamics/ranges/2RList.hpp"
#include "../../dynamics/locals/oscillatingplate.hpp"
#include <magnet/xmlwriter.hpp>
#include <magnet/xmlreader.hpp>
#include <boost/foreach.hpp>
#include <fstream>

OPPlateMotion::OPPlateMotion(const dynamo::SimData* tmp, const magnet::xml::Node& XML):
  OPTicker(tmp,"PlateMotion"),
  partpartEnergyLoss(0),
  oldPlateEnergy(0)
{
  operator<<(XML);
}

OPPlateMotion::OPPlateMotion(const OPPlateMotion& cp):
  OPTicker(cp),
  plateID(cp.plateID),
  plateName(cp.plateName),
  partpartEnergyLoss(0),
  oldPlateEnergy(0)
{
  if (cp.logfile.is_open())
    cp.logfile.close();
}

void
OPPlateMotion::initialise()
{
  try {
    plateID = Sim->dynamics.getLocal(plateName)->getID();
  } catch(...)
    {
      M_throw() << "Could not find the PlateName specified. You said " << plateName;
    }
  
  if (dynamic_cast<const CLOscillatingPlate*>(Sim->dynamics.getLocals()[plateID].get_ptr()) == NULL) 
    M_throw() << "The PlateName'd local is not a CLOscillatingPlate";

  if (logfile.is_open())
    logfile.close();
  
  logfile.open("plateMotion.out", std::ios::out|std::ios::trunc);

  localEnergyLoss.resize(Sim->dynamics.getLocals().size(), std::make_pair(double(0),std::vector<double>()));
  localEnergyFlux.resize(Sim->dynamics.getLocals().size(), std::make_pair(double(0),std::vector<double>()));

  oldPlateEnergy = dynamic_cast<const CLOscillatingPlate*>(Sim->dynamics.getLocals()[plateID].get_ptr())->getPlateEnergy();
  partpartEnergyLoss = 0;

  momentumChange = Vector(0, 0, 0);
  ticker();
}

void 
OPPlateMotion::eventUpdate(const LocalEvent& localEvent, const NEventData& SDat)
{
  double newPlateEnergy = oldPlateEnergy;

  if (localEvent.getLocalID() == plateID)
    newPlateEnergy = dynamic_cast<const CLOscillatingPlate*>(Sim->dynamics.getLocals()[plateID].get_ptr())->getPlateEnergy();

  double EnergyChange(0);

  BOOST_FOREACH(const ParticleEventData& pData, SDat.L1partChanges)
    EnergyChange += pData.getDeltaKE();  
  
  BOOST_FOREACH(const PairEventData& pData, SDat.L2partChanges)
    EnergyChange += pData.particle1_.getDeltaKE() + pData.particle2_.getDeltaKE();
  
  localEnergyFlux[localEvent.getLocalID()].first += EnergyChange;

  localEnergyLoss[localEvent.getLocalID()].first += EnergyChange + newPlateEnergy - oldPlateEnergy;

  oldPlateEnergy = newPlateEnergy;

  BOOST_FOREACH(const ParticleEventData& pData, SDat.L1partChanges)
    momentumChange += pData.getDeltaP();
}

void 
OPPlateMotion::eventUpdate(const IntEvent&, const PairEventData& pData)
{
  partpartEnergyLoss += pData.particle1_.getDeltaKE()
    + pData.particle2_.getDeltaKE();
}

void 
OPPlateMotion::ticker()
{
  BOOST_FOREACH(localEntry& entry, localEnergyLoss)
    entry.second.push_back(entry.first);

  BOOST_FOREACH(localEntry& entry, localEnergyFlux)
    {
      entry.second.push_back(entry.first);
      entry.first = 0.0;
    }

  Vector com(0,0,0), momentum(0,0,0);
  double sqmom(0);
  double partEnergy(0.0);

  double mass(0);
  BOOST_FOREACH(const Particle& part, Sim->particleList)
    {
      Vector pos(part.getPosition()), vel(part.getVelocity());
      double pmass(Sim->dynamics.getSpecies(part).getMass(part.getID()));
      Sim->dynamics.BCs().applyBC(pos, vel);
      momentum += vel * pmass;
      sqmom += (vel | vel) * (pmass * pmass);
      com += pos * pmass;
      mass += pmass;
      partEnergy += pmass * vel.nrm2();
    }
  
  com /= (mass * Sim->dynamics.units().unitLength());
  Vector comvel = momentum / (mass * Sim->dynamics.units().unitVelocity());
  
  partEnergy *= 0.5;

  const CLOscillatingPlate& plate(*dynamic_cast<const CLOscillatingPlate*>(Sim->dynamics.getLocals()[plateID].get_ptr()));

  Vector platePos = (plate.getPosition() - plate.getCentre()) / Sim->dynamics.units().unitLength();

  Vector plateSpeed = plate.getVelocity() / Sim->dynamics.units().unitVelocity();

  momentumChange /= Sim->dynamics.units().unitMomentum();

  logfile << Sim->dSysTime / Sim->dynamics.units().unitTime()
	  << " " << momentumChange[0] << " " << momentumChange[1] << " " << momentumChange[2]
	  << " " << platePos[0] << " " << platePos[1] << " " << platePos[2] 
	  << " " << com[0] << " " << com[1] << " " << com[2]
	  << " " << comvel[0] << " " << comvel[1] << " " << comvel[2]
	  << " " << plateSpeed[0] << " " << plateSpeed[1] << " " << plateSpeed[2]
	  << " " << (sqmom - ((momentum | momentum) / Sim->N)) / (Sim->N * pow(Sim->dynamics.units().unitMomentum(),2))
	  << " " << plate.getPlateEnergy() / Sim->dynamics.units().unitEnergy()
	  << " " << partEnergy / Sim->dynamics.units().unitEnergy() 
	  << " " << (plate.getPlateEnergy() + partEnergy) / Sim->dynamics.units().unitEnergy()
	  << " " << partpartEnergyLoss / Sim->dynamics.units().unitEnergy()
	  << "\n";
  
  momentumChange = Vector(0, 0, 0);

  //partpartEnergyLoss = 0.0;
}

void 
OPPlateMotion::operator<<(const magnet::xml::Node& XML)
{
  try {
    plateName = std::string(XML.getAttribute("PlateName"));
  } catch(...)
    {
      M_throw() << "Could not find the PlateName for the PlateMotion plugin. Did you specify one?";
    }
}

void 
OPPlateMotion::output(magnet::xml::XmlStream& XML)
{
  XML << magnet::xml::tag("PlateMotion");
 
  for (size_t ID(0); ID < localEnergyLoss.size(); ++ID)
    {
      {
	std::fstream of((Sim->dynamics.getLocals()[ID]->getName() 
		       + std::string("EnergyLoss.out")).c_str(), 
		      std::ios::out | std::ios::trunc);
      
	size_t step(0);
	double deltat(getTickerTime() / Sim->dynamics.units().unitTime());
	double sum = localEnergyLoss[ID].first;
	BOOST_FOREACH(const double& val, localEnergyLoss[ID].second)
	  {
	    sum += val;
	    of << deltat * (step++) << " " 
	       << val / Sim->dynamics.units().unitEnergy() 
	       << "\n";
	  }

	XML << magnet::xml::tag("Plate")
	    << magnet::xml::attr("ID") << ID
	    << magnet::xml::attr("PowerLossRate") 
	    << (sum * Sim->dynamics.units().unitTime() 
		/ (Sim->dSysTime * Sim->dynamics.units().unitEnergy()))
	    << magnet::xml::endtag("Plate");
      }
      {
	std::fstream of((Sim->dynamics.getLocals()[ID]->getName() 
			 + std::string("EnergyFlux.out")).c_str(), 
			std::ios::out | std::ios::trunc);
	
	size_t step(0);
	double deltat(getTickerTime() / Sim->dynamics.units().unitTime());
	
	BOOST_FOREACH(const double& val, localEnergyFlux[ID].second)
	  of << deltat * (step++) << " " << val / (deltat * Sim->dynamics.units().unitEnergy()) << "\n";
      }
    }
  XML << magnet::xml::endtag("PlateMotion");
}
