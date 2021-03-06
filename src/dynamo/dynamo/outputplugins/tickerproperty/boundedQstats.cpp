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

#include "boundedQstats.hpp"
#include "../../base/is_simdata.hpp"
#include "../../schedulers/scheduler.hpp"
#include "../../schedulers/sorters/boundedPQ.hpp"
#include <boost/foreach.hpp>
#include <magnet/xmlwriter.hpp>

OPBoundedQStats::OPBoundedQStats(const dynamo::SimData* tmp, 
				 const magnet::xml::Node&):
  OPTicker(tmp,"BoundedPQstats"),
  treeSize(1)
{}

void 
OPBoundedQStats::initialise()
{  
  if (dynamic_cast<const CSSBoundedPQ<>*>
      (Sim->ptrScheduler->getSorter().get_ptr()) == NULL)
    M_throw() << "Not a bounded queue sorter!";

}

void
OPBoundedQStats::ticker()
{
  const CSSBoundedPQ<>& sorter(dynamic_cast<const CSSBoundedPQ<>&>
		       (*(Sim->ptrScheduler->getSorter())));
 
  treeSize.addVal(sorter.treeSize());

}

void 
OPBoundedQStats::output(magnet::xml::XmlStream& XML)
{
  const CSSBoundedPQ<>& sorter(dynamic_cast<const CSSBoundedPQ<>&>
			     (*(Sim->ptrScheduler->getSorter())));

  XML << magnet::xml::tag("boundedQstats") 
      << magnet::xml::attr("ExceptionEvents") << sorter.exceptionEvents()
      << magnet::xml::tag("CBTSize");

  treeSize.outputHistogram(XML,1.0);

  XML << magnet::xml::endtag("CBTSize")
    << magnet::xml::tag("treedist")
      << magnet::xml::chardata();

  if (!Sim->eventCount)
    {
      derr << "Cannot print the tree as the queue is\n"
	       << "not initialised until an event is run (i.e. N_event != 0).\n"
	       << "Continuing without tree output." << std::endl;
    }
  else
    {
      const std::vector<size_t>& eventdist(sorter.getEventCounts());
      
      for (size_t i = 0; i < eventdist.size(); ++i)
	XML << i << " " 
	    << eventdist[i]
	    << "\n";
    }
  
  XML << magnet::xml::endtag("treedist")
      << magnet::xml::endtag("boundedQstats");
}
