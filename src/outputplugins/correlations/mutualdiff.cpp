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

#include "mutualdiff.hpp"
#include "../../dynamics/include.hpp"
#include "../1partproperty/kenergy.hpp"
#include "../0partproperty/misc.hpp"
#include "../../datatypes/vector.xml.hpp"

COPMutualDiffusion::COPMutualDiffusion(const DYNAMO::SimData* tmp, const XMLNode& XML):
  COutputPlugin(tmp, "MutualDiffusion", 60), //Note the sort order set later
  count(0),
  dt(0),
  currentdt(0.0),
  delGsp1(0.0),
  delGsp2(0.0),
  species1(NULL),
  species2(NULL),
  sysMom(0.0),
  massFracSp1(1),
  massFracSp2(1),
  CorrelatorLength(100),
  currCorrLen(0),
  notReady(true)

{
  operator<<(XML);
}

void 
COPMutualDiffusion::operator<<(const XMLNode& XML)
{
  try 
    {
      if (XML.isAttributeSet("Length"))
	CorrelatorLength = boost::lexical_cast<unsigned int>(XML.getAttribute("Length"));

      if (XML.isAttributeSet("dt"))
	dt = Sim->Dynamics.units().unitTime() * 
	  boost::lexical_cast<Iflt>(XML.getAttribute("dt"));

	if (XML.isAttributeSet("t"))
	  dt = Sim->Dynamics.units().unitTime() * 
	    boost::lexical_cast<Iflt>(XML.getAttribute("t"))/CorrelatorLength;
    }
  catch (boost::bad_lexical_cast &)
    {
      D_throw() << "Failed a lexical cast in COPMutualDiffusion";
    }
  
  try 
    {
      try {
	species1 = Sim->Dynamics
	  .getSpecies(boost::lexical_cast<std::string>
		      (XML.getAttribute("Species1"))).getID();

	species2 = Sim->Dynamics
	  .getSpecies(boost::lexical_cast<std::string>
		      (XML.getAttribute("Species2"))).getID();

      } catch (std::exception& nex)
	{
	  D_throw() << "Failed to find the species for the mutual diffusion\n"
		    << nex.what();
	}
    }
  catch (boost::bad_lexical_cast &)
    {
      D_throw() << "Failed a lexical cast in COPMutualDiffusion";
    }
}

void 
COPMutualDiffusion::stream(const Iflt edt)
{
  //Move the time forward
  currentdt += edt;
  
  //Now test if we've gone over the step time
  if (currentdt >= dt)
    {
      currentdt -= dt;
      newG();
    }
}

void 
COPMutualDiffusion::eventUpdate(const CGlobEvent& iEvent, const CNParticleData& PDat) 
{
  stream(iEvent.getdt());
  updateDelG(PDat);
}

void 
COPMutualDiffusion::eventUpdate(const CSystem&, const CNParticleData& PDat, const Iflt& edt) 
{ 
  stream(edt);
  updateDelG(PDat);
}

void 
COPMutualDiffusion::eventUpdate(const CIntEvent& iEvent, const C2ParticleData& PDat)
{
  stream(iEvent.getdt());
  updateDelG(PDat);
}

Iflt 
COPMutualDiffusion::rescaleFactor()
{
  return 1.0 / (Sim->Dynamics.units().unitMutualDiffusion()
		* count * Sim->Dynamics.units().simVolume()
		* Sim->getOutputPlugin<COPKEnergy>()->getAvgkT());
}

void 
COPMutualDiffusion::output(xmlw::XmlStream& XML)
{
  Iflt factor = rescaleFactor();
  
  CVector<> acc = 0.5*(accG[0] + accG[accG.size()-1]);

  for (unsigned int i = 1; i < accG.size()-1; i++)
    acc += accG[i];

  acc *= factor * dt / Sim->Dynamics.units().unitTime();

  XML << xmlw::tag("Correlator")
      << xmlw::attr("name") << name
      << xmlw::attr("size") << accG.size()
      << xmlw::attr("dt") << dt / Sim->Dynamics.units().unitTime()
      << xmlw::attr("LengthInMFT") << dt * accG.size() 
    / Sim->getOutputPlugin<COPMisc>()->getMFT()
      << xmlw::attr("simFactor") << factor
      << xmlw::attr("SampleCount") << count
      << xmlw::tag("Integral") << acc
      << xmlw::endtag("Integral")
      << xmlw::chardata();
    
  for (size_t i = 0; i < accG.size(); i++)
    {
      XML << (i + 1) * dt / Sim->Dynamics.units().unitTime();
      for (size_t j = 0; j < NDIM; j++)
	XML << "\t" << accG[i][j] * factor;
      XML << "\n";
    }
  
  
  XML << xmlw::endtag("Correlator"); 
}

void 
COPMutualDiffusion::initialise()
{
  Sim->getOutputPlugin<COPKEnergy>();
  Sim->getOutputPlugin<COPMisc>();
  
  accG.resize(CorrelatorLength, CVector<> (0.0));
  dt = getdt();
  
  Iflt sysMass = 0.0;
  
  BOOST_FOREACH(const CParticle& part, Sim->vParticleList)
    {
      sysMom += part.getVelocity() * Sim->Dynamics.getSpecies(part).getMass();
      sysMass += Sim->Dynamics.getSpecies(part).getMass();
      
      if (Sim->Dynamics.getSpecies()[species1].isSpecies(part))
	delGsp1 += part.getVelocity();
      
      if (Sim->Dynamics.getSpecies()[species2].isSpecies(part))
	delGsp2 += part.getVelocity();
    }
  
  delGsp1 *= Sim->Dynamics.getSpecies()[species1].getMass();
  delGsp2 *= Sim->Dynamics.getSpecies()[species2].getMass();
  
  massFracSp1 = Sim->Dynamics.getSpecies()[species1].getCount() 
    * Sim->Dynamics.getSpecies()[species1].getMass() / sysMass; 

  massFracSp2 = Sim->Dynamics.getSpecies()[species2].getCount() 
    * Sim->Dynamics.getSpecies()[species2].getMass() / sysMass;
}

std::list<CVector<> > 
COPMutualDiffusion::getAvgAcc() const
{
  std::list<CVector<> > tmp;
  
  BOOST_FOREACH(const CVector<> &val, accG)
    tmp.push_back(val/((Iflt) count));
  
  return tmp;
}

void 
COPMutualDiffusion::updateDelG(const C2ParticleData& PDat) 
{
  updateDelG(PDat.particle1_);
  updateDelG(PDat.particle2_);
}

void 
COPMutualDiffusion::updateDelG(const C1ParticleData& PDat) 
{
  sysMom += PDat.getDeltaP();
  
  if (PDat.getSpecies().getID() == species1)
    delGsp1 += PDat.getDeltaP();
  
  if (PDat.getSpecies().getID() == species2)
    delGsp2 += PDat.getDeltaP();
  
}

void 
COPMutualDiffusion::updateDelG(const CNParticleData& ndat)
{
  BOOST_FOREACH(const C1ParticleData& dat, ndat.L1partChanges)
    updateDelG(dat);
  
  BOOST_FOREACH(const C2ParticleData& dat, ndat.L2partChanges)
    updateDelG(dat);
}

void 
COPMutualDiffusion::newG()
{
  G.push_front(delGsp2);

  //This ensures the list gets to accumilator size
  if (notReady)
    {
      if (++currCorrLen != CorrelatorLength)
	return;
      
      notReady = false;
    }

  accPass ();
}

void 
COPMutualDiffusion::accPass()
{
  ++count;
  
  for (size_t i = 0; i < CorrelatorLength; ++i)
    accG[i] += (delGsp1 - (massFracSp1 * sysMom)) * (G[i] - (massFracSp2 * sysMom));
}

Iflt 
COPMutualDiffusion::getdt()
{
  //Get the simulation temperature
    if (dt == 0.0)
      {
	if (Sim->lastRunMFT != 0.0)
	  return Sim->lastRunMFT * 30.0 / CorrelatorLength;
	else
	  return 5.0 / (((Iflt) CorrelatorLength)*sqrt(Sim->Dynamics.getkT()) * CorrelatorLength);
      }
    else 
      return dt;
}
