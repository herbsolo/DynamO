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

#ifndef COPViscosityE_H
#define COPViscosityE_H

#include "../outputplugin.hpp"
#include "../../datatypes/vector.hpp"
#include <boost/circular_buffer.hpp>

/*! \brief The Correlator class for the viscosity.*/
class COPViscosityE: public COutputPlugin
{
  typedef boost::array<Iflt, NDIM> col;
  typedef boost::array<col, NDIM> matrix;
  
public:
  COPViscosityE(const DYNAMO::SimData*, const XMLNode& XML);

  virtual void initialise();

  virtual void output(xmlw::XmlStream &);
  
  virtual COutputPlugin* Clone() const { return new COPViscosityE(*this); }
  
  virtual void eventUpdate(const CGlobEvent&, const CNParticleData&);

  virtual void eventUpdate(const CLocalEvent&, const CNParticleData&);
  
  virtual void eventUpdate(const CSystem&, const CNParticleData&, const Iflt&);
  
  virtual void eventUpdate(const CIntEvent&, const C2ParticleData&);

  virtual void stream(const Iflt&);

  virtual void operator<<(const XMLNode&);

protected:
  inline matrix impulseDelG(const C2ParticleData&);
  inline matrix impulseDelG(const CNParticleData&);

  virtual void updateConstDelG(const C2ParticleData&);
  virtual void updateConstDelG(const C1ParticleData&);
  virtual void updateConstDelG(const CNParticleData&);
  
  void newG(const matrix&);
  void accPass();

  matrix avgTrace;

  size_t count;
  Iflt dt, currentdt;
  matrix constDelG, delG;

  size_t currlen;
  bool notReady;

  size_t CorrelatorLength;

  boost::circular_buffer<matrix> G;
  std::vector<matrix> accG2;
};

#endif