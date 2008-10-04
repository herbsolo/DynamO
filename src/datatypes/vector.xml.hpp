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

#ifndef VECTOR_XML_H
#define VECTOR_XML_H

#include "../extcode/xmlwriter.hpp"
#include "../extcode/xmlParser.h"
#include "../base/constants.hpp"
#include <boost/lexical_cast.hpp> //For xml Parsing

template<class T>
void 
CVector<T>::operator<<(const XMLNode &XML)
{
  for (int iDim = 0; iDim < NDIM; iDim++) 
    {
      char name[2] = "x";
      name[0] = 'x' + iDim; //Write the name
      if (!XML.isAttributeSet(name))
	name[0] = '0'+iDim;
      
      try {
	data[iDim] = boost::lexical_cast<T>(XML.getAttribute(name));
      }
      catch (boost::bad_lexical_cast &)
	{
	  D_throw() << "Failed a lexical cast in CVector";
	}
    }
}

template<class T>
inline
xmlw::XmlStream& 
operator<<(xmlw::XmlStream& XML, const CVector<T> &vec)
{
  char name[2] = "x";
  
  for (int iDim = 0; iDim < NDIM; iDim++)
    {
      name[0]= 'x'+iDim; //Write the dimension
      XML << xmlw::attr(name) << vec[iDim];
    }
  
  return XML;
}

template<class T>
inline
xmlw::XmlStream& 
operator<<(xmlw::XmlStream& XML, const CVector<CVector<T> > &vec)
{
  char name[2] = "x";
  
  for (int iDim = 0; iDim < NDIM; iDim++)
    {
      name[0]= 'x'+iDim; //Write the dimension
      XML << xmlw::tag(name) << vec[iDim]
	  << xmlw::endtag(name);
    }
  
  return XML;
}

#endif
