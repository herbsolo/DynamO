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

#pragma once

#include <dynamo/dynamics/dynamics.hpp>
#include <dynamo/datatypes/vector.hpp>
#include <dynamo/simulation/particle.hpp>
#include <dynamo/outputplugins/outputplugin.hpp>
#include <dynamo/simulation/ensemble.hpp>
#include <dynamo/simulation/property.hpp>
#include <magnet/function/delegate.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <include/boost/random/01_normal_distribution.hpp>
#include <vector>
#include <list>

class CScheduler;
class Particle;
class OutputPlugin;

namespace magnet {
  template <class T>
  class ClonePtr;
}

//! \brief Holds the different phases of the simulation initialisation
typedef enum 
  {
    START         = 0, /*!< The first phase of the simulation. */
    CONFIG_LOADED = 1, /*!< After the configuration has been loaded. */
    INITIALISED   = 2, /*!< Once the classes have been initialised and
                          the simulation is ready to begin. */
    PRODUCTION    = 3, /*!< The simulation has already begun. */
    ERROR         = 4  /*!< The simulation has failed. */
  } ESimulationStatus;

namespace dynamo
{  
  typedef boost::mt19937 baseRNG;
  
  /*! \brief Fundamental collection of the Simulation data.
   *
   * This struct contains all the data belonging to a single
   * Simulation. It has been abstracted away from the Simulation
   * class so that every class can contain a pointer to this datatype
   * without knowing the Simulation class and causing a circular
   * reference/dependency.
   *
   * A pointer to this class has been incorporated in the base classes
   * SimBase and SimBase_Const which also provide some general
   * std::cout formatting.
   */
  class SimData : public dynamo::Base
  {
  protected:
    typedef magnet::function::Delegate1
    <const NEventData&, void> particleUpdateFunc;
    
  public:
    /*! \brief Significant default value initialisation.
     */
    SimData();

    /*! \brief Handles deleting the CScheduler pointer
     *
     * \bug Make the Scheduler handled by a smrtPlugPtr, and make the
     * CScheduler class copyable.
     */
    ~SimData();
    
    /*! \brief Adds an output plugin of the type T.
     *
     * Throws an exception if there's already the same plugin loaded.
     */
    template<class T>
    const T* getOutputPlugin() const
    {
      BOOST_FOREACH(const magnet::ClonePtr<OutputPlugin>& plugin, outputPlugins)
	if (dynamic_cast<const T*>(plugin.get_ptr()) != NULL)
	  return dynamic_cast<const T*>(plugin.get_ptr());
      
      M_throw() << "The output plugin " << (typeid(T).name()) << " is required, please add it";
    }   

    /*! \brief Finds a plugin of the given type using RTTI.
     *
     * Shouldn't use this frequently as its expensive, throws an
     * exception if it can't find the plugin.
     */
    template<class T>
    T* getOutputPlugin()
    {
      BOOST_FOREACH(magnet::ClonePtr<OutputPlugin>& plugin, outputPlugins)
	if (dynamic_cast<T*>(plugin.get_ptr()) != NULL)
	  return dynamic_cast<T*>(plugin.get_ptr());

      M_throw() << "The output plugin " << (typeid(T).name()) << " is required, please add it";
    }    

    //! Loads a Simulation from the passed XML file.
    //! \param filename The path to the XML file to load. The filename must
    //! end in either ".xml" for uncompressed xml files or ".bz2" for
    //! bzip2 compressed configuration files.
    void loadXMLfile(std::string filename);
    
    //! Writes the Simulation configuration to a file at the passed path.
    //! \param filename The path to the XML file to write (this file
    //! will either be created or overwritten). The filename must end in
    //! either ".xml" for uncompressed xml files or ".bz2" for bzip2
    //! compressed configuration files.
    //! \param round If true, the data in the XML file will be written
    //! out at 2 s.f. lower precision to round all the values. This is
    //! used in the test harness to remove rounding error ready for a
    //! comparison to a "correct" configuration file.
    void writeXMLfile(std::string filename, bool applyBC = true, bool round = false);

    /*! \brief The Ensemble of the Simulation. */
    boost::scoped_ptr<Ensemble> ensemble;

    /*! \brief The current system time of the simulation. 
     * 
     * This class is long double to reduce roundoff error as this gets
     * very large compared to an events delta t.
     */
    long double dSysTime;


    /*! \brief This accumilator holds the time steps taken in between
     *  updating the outputplugins.
     *
     *  The idea is that outputplugins are only updated on events. but
     *  virtual events sometimes must stream the system. So we
     *  accumilate the time delta here and add it to the time we send
     *  to output plugins.
     */
    double freestreamAcc;
    
    /*! \brief Number of events executed.*/
    unsigned long long eventCount;

    /*! \brief Maximum number of events to execute.*/
    unsigned long long endEventCount;

    /*! \brief How many events between periodic output/sampling*/
    unsigned long long eventPrintInterval;
    
    /*! \brief Speeds the Simulation loop by being the next periodic
        output collision number.*/
    unsigned long long nextPrintEvent;

    /*! \brief Number of Particle's in the system. */
    unsigned long N;
    
    /*! \brief The Particle's of the system. */
    std::vector<Particle> particleList;  
    
    /*! \brief A log of the previous simulation history. */
    std::ostringstream ssHistory;

    /*! \brief A ptr to the CScheduler of the system. */
    CScheduler *ptrScheduler;

    /*! \brief The Dynamics of the system. */
    Dynamics dynamics;
    
    /*! The property store, a list of properties the particles have. */
    PropertyStore _properties;

    /*! \brief A vector of the ratio's of the simulation box/images sides.
     *
     * At least one ratio must be 1 as this is assumed when using the
     * ratio. i.e. it is normalised.
     */
    Vector  primaryCellSize;

    /*! \brief The random number generator of the system.
     */
    mutable baseRNG ranGenerator;

    mutable boost::variate_generator<dynamo::baseRNG&, boost::normal_distribution_01<double> > normal_sampler;

    mutable boost::uniform_01<dynamo::baseRNG, double> uniform_sampler;  

    /*! \brief The collection of OutputPlugin's operating on this system.
     */
    std::vector<magnet::ClonePtr<OutputPlugin> > outputPlugins; 

    /*! \brief The mean free time of the previous simulation run
     *
     * This is zero in the case that there is no previous simulation
     * data and is already in the units of the simulation once loaded
     */
    double lastRunMFT;

    /*! \brief This is just the ID number of the Simulation when
     *  multiple are being run at once.
     *
     * This is used in the EReplicaExchangeSimulation engine.
     */
    size_t simID;
    
    /*! \brief This is the number of replica exchange attempts
     *  performed in the current simulation.
     *
     * This is used in the EReplicaExchangeSimulation engine.
     */
    size_t replexExchangeNumber;


    /*! \brief The current phase of the Simulation.
     */
    ESimulationStatus status;

    /*! \brief Register a callback for particle changes.*/
    void registerParticleUpdateFunc(const particleUpdateFunc& func) const
    { _particleUpdateNotify.push_back(func); }

    /*! \brief Call all registered functions requiring a callback on
        particle changes.*/
    void signalParticleUpdate(const NEventData&) const;

    void replexerSwap(SimData&);
  private:
    
    mutable std::vector<particleUpdateFunc> _particleUpdateNotify;
  };

}
