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

#ifndef COPSHCrystal_H
#define COPSHCrystal_H

#include <boost/math/special_functions/spherical_harmonic.hpp>
#include "ticker.hpp"

class COPSHCrystal: public COPTicker
{
 public:
  COPSHCrystal(const DYNAMO::SimData*, const XMLNode&);

  virtual COutputPlugin *Clone() const
  { return new COPSHCrystal(*this); }

  virtual void initialise();

  virtual void stream(Iflt) {}

  virtual void ticker();
  
  virtual void output(xmlw::XmlStream&);

 protected:

  std::complex<Iflt> localq(const CParticle& part, int l, int m);

  //! Cut-off radius 
  Iflt rg;
  size_t maxl;
  size_t nblistID;
  size_t count;
  
  std::vector<std::vector<std::complex<Iflt> > > globalcoeff;

  struct sphericalsum
  {
    sphericalsum(const DYNAMO::SimData * const, 
		 const Iflt&, const size_t&);
    
    void operator()(const CParticle&, const size_t&) const;
    
    void clear();

    const DYNAMO::SimData* const Sim;
    const Iflt rg;
    const size_t maxl;
    mutable size_t count;
    mutable std::vector<std::vector<std::complex<Iflt> > > coeffsum;
  };
};

#endif
