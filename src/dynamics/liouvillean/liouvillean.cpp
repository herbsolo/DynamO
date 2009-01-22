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

#include "include.hpp"
#include "../../extcode/xmlwriter.hpp"
#include "../../extcode/xmlParser.h"
#include "../../base/is_exception.hpp"
#include "../../base/is_simdata.hpp"
#include "../2particleEventData.hpp"
#include "../units/units.hpp"
#include <boost/foreach.hpp>
#include <boost/progress.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/filter/base64.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/chain.hpp>
#include <boost/iostreams/device/stream_sink.hpp>
#include <boost/iostreams/device/stream_source.hpp>

xmlw::XmlStream& operator<<(xmlw::XmlStream& XML, const CLiouvillean& g)
{
  g.outputXML(XML);
  return XML;
}

CLiouvillean* 
CLiouvillean::loadClass(const XMLNode& XML, DYNAMO::SimData* tmp)
{
  if (!strcmp(XML.getAttribute("Type"),"Newtonian"))
    return new CLNewton(tmp);
  else if (!strcmp(XML.getAttribute("Type"),"NOrientation"))
    return new CLNOrientation(tmp, XML);
  else if (!strcmp(XML.getAttribute("Type"),"SLLOD"))
    return new CLSLLOD(tmp);
  else 
    D_throw() << "Unknown type of Liouvillean encountered";
}

C2ParticleData 
CLiouvillean::runLineLineCollision(const CIntEvent&,
				   const Iflt&) const
{ D_throw() << "Not implemented for this Liouvillean."; }

bool
CLiouvillean::getLineLineCollision(CPDData&, const Iflt&, 
				   const CParticle&, const CParticle&
				   ) const
{ D_throw() << "Not implemented for this Liouvillean."; }

void 
CLiouvillean::loadParticleXMLData(const XMLNode& XML, std::istream& os)
{
  I_cout() << "Loading Particle Data ";
  fflush(stdout);

  if (XML.isAttributeSet("AttachedBinary")
      && (std::toupper(XML.getAttribute("AttachedBinary")[0]) == 'Y'))
    {
      Sim->binaryXML = true;
      unsigned long nPart = boost::lexical_cast<unsigned long>(XML.getAttribute("N"));

      D_throw() << "Not implemented";
    }
  else
    {
      int xml_iter = 0;
      bool outofsequence = false;
      
      unsigned long nPart = XML.nChildNode("Pt");
      boost::progress_display prog(nPart);
      
      for (unsigned long i = 0; i < nPart; i++)
	{
	  XMLNode xBrowseNode = XML.getChildNode("Pt", &xml_iter);
	  
	  if (boost::lexical_cast<unsigned long>
	      (xBrowseNode.getAttribute("ID")) != i)
	    outofsequence = true;
	  
	  CParticle part(xBrowseNode, i);
	  part.scaleVelocity(Sim->Dynamics.units().unitVelocity());
	  part.scalePosition(Sim->Dynamics.units().unitLength());
	  Sim->vParticleList.push_back(part);
	  ++prog;
	}
      if (outofsequence)
	I_cout() << IC_red << "Particle ID's out of sequence!\n"
		 << IC_red << "This can result in incorrect capture map loads etc.\n"
		 << IC_red << "Erase any capture maps in the configuration file so they are regenerated."
		 << IC_reset;      
    }  
}

//! \brief Helper function for writing out data
template<class T>
void 
CLiouvillean::binarywrite(std::ostream& os, const T& val ) const
{
  os.write(reinterpret_cast<const char*>(&val), sizeof(T));
}

template<class T>
void 
CLiouvillean::binaryread(std::istream& os, T& val) const
{
  os.read(reinterpret_cast<char*>(&val), sizeof(T));
}

void 
CLiouvillean::outputParticleBin64Data(std::ostream& os) const
{
  if (!Sim->binaryXML)
    return;
  
  
  boost::iostreams::filtering_ostream base64Convertor;
  base64Convertor.push(boost::iostreams::base64_encoder());
  base64Convertor.push(boost::iostreams::stream_sink<std::ostream>(os));
  
  BOOST_FOREACH(const CParticle& part, Sim->vParticleList)
    {
      CParticle tmp(part);
      Sim->Dynamics.BCs().setPBC(tmp.getPosition(), tmp.getVelocity());
      
      tmp.scaleVelocity(1.0 / Sim->Dynamics.units().unitVelocity());
      tmp.scalePosition(1.0 / Sim->Dynamics.units().unitLength());	  

      binarywrite(base64Convertor, tmp.getID());
      for (size_t iDim(0); iDim < NDIM; ++iDim)
	binarywrite(base64Convertor, tmp.getVelocity()[iDim]);

      for (size_t iDim(0); iDim < NDIM; ++iDim)
	binarywrite(base64Convertor, tmp.getPosition()[iDim]);
    }

}

void 
CLiouvillean::outputParticleXMLData(xmlw::XmlStream& XML) const
{
  XML << xmlw::tag("ParticleData");

  if (Sim->binaryXML)
    {
      XML << xmlw::attr("N") << Sim->lN
	  << xmlw::attr("AttachedBinary") << "Y";
    }
  else
    {
      XML << xmlw::attr("N") << Sim->lN
	  << xmlw::attr("AttachedBinary") << "N";
      
      I_cout() << "Writing Particles ";
      
      boost::progress_display prog(Sim->lN);
      
      for (unsigned long i = 0; i < Sim->lN; ++i)
	{
	  CParticle tmp(Sim->vParticleList[i]);
	  Sim->Dynamics.BCs().setPBC(tmp.getPosition(), tmp.getVelocity());
	  
	  tmp.scaleVelocity(1.0 / Sim->Dynamics.units().unitVelocity());
	  tmp.scalePosition(1.0 / Sim->Dynamics.units().unitLength());
	  
	  XML << xmlw::tag("Pt") << tmp << xmlw::endtag("Pt");
	  
	  ++prog;
	}
    }

  XML << xmlw::endtag("ParticleData");
}
