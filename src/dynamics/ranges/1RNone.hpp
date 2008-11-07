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

#ifndef CRNone_H
#define CRNone_H

#include "1range.hpp"
#include "../../base/is_base.hpp"
#include "../../base/is_simdata.hpp"

class CRNone: public CRange
{
public:
  CRNone() {}

  CRNone(const XMLNode&);

  virtual CRange* Clone() const { return new CRNone(*this); };

  virtual bool isInRange(const CParticle&) const
  { return false; }

  //The data output classes
  virtual void operator<<(const XMLNode&);

  virtual unsigned long size() const { return 0; }

  virtual iterator begin() const { return CRange::iterator(0, this); }

  virtual iterator end() const { return CRange::iterator(0, this); }

  virtual unsigned long operator[](unsigned long i) const  
  {
    D_throw() << "Nothing to access";
  }

  virtual unsigned long at(unsigned long i) const 
  { 
    D_throw() << "Nothing to access";
  }

protected:

  virtual const unsigned long& getIteratorID(const unsigned long &i) const 
  { D_throw() << "Nothing here!"; }

  virtual void outputXML(xmlw::XmlStream&) const;
};

#endif