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

#ifndef COPMutualDiffusionGK_H
#define COPMutualDiffusionGK_H

#include <boost/circular_buffer.hpp>
#include "../outputplugin.hpp"

class COPMutualDiffusionGK: public COutputPlugin
{
public:
  COPMutualDiffusionGK(const DYNAMO::SimData*, const XMLNode&);
  
  virtual void operator<<(const XMLNode&);

  virtual COutputPlugin* Clone() const { return new COPMutualDiffusionGK(*this); }

  virtual void stream(const Iflt);

  virtual void eventUpdate(const CGlobEvent&, const CNParticleData&);

  virtual void eventUpdate(const CLocalEvent&, const CNParticleData&);

  virtual void eventUpdate(const CSystem&, const CNParticleData&, const Iflt&);
  
  virtual void eventUpdate(const CIntEvent&, const C2ParticleData&);

  virtual Iflt rescaleFactor();

  virtual void output(xmlw::XmlStream&);

  virtual void initialise();
 
  std::list<CVector<> > getAvgAcc() const;
  
 protected:  
  virtual void updateDelG(const C2ParticleData&);

  virtual void updateDelG(const C1ParticleData&);

  virtual void updateDelG(const CNParticleData&);
    
  virtual void newG();
  
  virtual void accPass();

  Iflt getdt();
    
  boost::circular_buffer<CVector<> > G;
  std::vector<CVector<> > accG;
  size_t count;
  Iflt dt, currentdt;
  CVector<> delGsp1, delGsp2;

  size_t species1;
  size_t species2;
  
  CVector<> sysMom;

  Iflt massFracSp1;
  Iflt massFracSp2;

  size_t CorrelatorLength;
  size_t currCorrLen;
  bool notReady;

};

#endif