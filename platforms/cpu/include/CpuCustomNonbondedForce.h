
/* Portions copyright (c) 2009-2014 Stanford University and Simbios.
 * Contributors: Peter Eastman
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OPENMM_CPU_CUSTOM_NONBONDED_FORCE_H__
#define OPENMM_CPU_CUSTOM_NONBONDED_FORCE_H__

#include "AlignedArray.h"
#include "CpuNeighborList.h"
#include "openmm/internal/ThreadPool.h"
#include "openmm/internal/vectorize.h"
#include "lepton/CompiledExpression.h"
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace OpenMM {
    
class CpuCustomNonbondedForce {

   private:
      class ComputeForceTask;
      class ThreadData;

      bool cutoff;
      bool useSwitch;
      bool periodic;
      const CpuNeighborList* neighborList;
      RealOpenMM periodicBoxSize[3];
      RealOpenMM cutoffDistance, switchingDistance;
      ThreadPool& threads;
      std::vector<ThreadData*> threadData;
      std::vector<std::string> paramNames;
      std::vector<std::pair<std::set<int>, std::set<int> > > interactionGroups;
      std::vector<double> threadEnergy;
    // The following variables are used to make information accessible to the individual threads.
    int numberOfAtoms;
    float* posq;
    RealVec const* atomCoordinates;
    RealOpenMM** atomParameters;        
    const std::map<std::string, double>* globalParameters;
    std::set<int> const* exclusions;
    std::vector<AlignedArray<float> >* threadForce;
    bool includeForce, includeEnergy;
    void* atomicCounter;

    /**
     * This routine contains the code executed by each thread.
     */
    void threadComputeForce(ThreadPool& threads, int threadIndex);

      /**---------------------------------------------------------------------------------------

         Calculate custom pair ixn between two atoms

         @param atom1            the index of the first atom
         @param atom2            the index of the second atom
         @param atomCoordinates  atom coordinates
         @param atomParameters   atomParameters[atomIndex][parameterIndex]
         @param forces           force array (forces added)
         @param totalEnergy      total energy

         --------------------------------------------------------------------------------------- */

      void calculateOneIxn(int atom1, int atom2, ThreadData& data, float* forces, double& totalEnergy, const fvec4& boxSize, const fvec4& invBoxSize);


   public:

      /**---------------------------------------------------------------------------------------

         Constructor

         --------------------------------------------------------------------------------------- */

       CpuCustomNonbondedForce(const Lepton::CompiledExpression& energyExpression, const Lepton::CompiledExpression& forceExpression,
                                   const std::vector<std::string>& parameterNames, ThreadPool& threads);

      /**---------------------------------------------------------------------------------------

         Destructor

         --------------------------------------------------------------------------------------- */

       ~CpuCustomNonbondedForce( );

      /**---------------------------------------------------------------------------------------

         Set the force to use a cutoff.

         @param distance            the cutoff distance
         @param neighbors           the neighbor list to use

         --------------------------------------------------------------------------------------- */

      void setUseCutoff( RealOpenMM distance, const CpuNeighborList& neighbors );

      /**---------------------------------------------------------------------------------------

         Restrict the force to a list of interaction groups.

         @param distance            the cutoff distance
         @param neighbors           the neighbor list to use

         --------------------------------------------------------------------------------------- */

      void setInteractionGroups(const std::vector<std::pair<std::set<int>, std::set<int> > >& groups);

      /**---------------------------------------------------------------------------------------
      
         Set the force to use a switching function.
      
         @param distance            the switching distance
      
         --------------------------------------------------------------------------------------- */
      
      void setUseSwitchingFunction( RealOpenMM distance );

      /**---------------------------------------------------------------------------------------

         Set the force to use periodic boundary conditions.  This requires that a cutoff has
         already been set, and the smallest side of the periodic box is at least twice the cutoff
         distance.

         @param boxSize             the X, Y, and Z widths of the periodic box

         --------------------------------------------------------------------------------------- */

      void setPeriodic( OpenMM::RealVec& boxSize );

      /**---------------------------------------------------------------------------------------

         Calculate custom pair ixn

         @param numberOfAtoms    number of atoms
         @param posq             atom coordinates in float format
         @param atomCoordinates  atom coordinates
         @param atomParameters   atom parameters (charges, c6, c12, ...)     atomParameters[atomIndex][paramterIndex]
         @param exclusions       atom exclusion indices
                                 exclusions[atomIndex] contains the list of exclusions for that atom
         @param fixedParameters  non atom parameters (not currently used)
         @param globalParameters the values of global parameters
         @param forces           force array (forces added)
         @param totalEnergy      total energy
         @param threads          the thread pool to use

         --------------------------------------------------------------------------------------- */

      void calculatePairIxn(int numberOfAtoms, float* posq, std::vector<OpenMM::RealVec>& atomCoordinates,
                            RealOpenMM** atomParameters, std::vector<std::set<int> >& exclusions,
                            RealOpenMM* fixedParameters, const std::map<std::string, double>& globalParameters,
                            std::vector<AlignedArray<float> >& threadForce, bool includeForce, bool includeEnergy, double& totalEnergy);

      /**
       * Compute the displacement and squared distance between two points, optionally using
       * periodic boundary conditions.
       */
      void getDeltaR(const fvec4& posI, const fvec4& posJ, fvec4& deltaR, float& r2, const fvec4& boxSize, const fvec4& invBoxSize) const;
};

class CpuCustomNonbondedForce::ThreadData {
public:
    ThreadData(const Lepton::CompiledExpression& energyExpression, const Lepton::CompiledExpression& forceExpression, const std::vector<std::string>& parameterNames);
    Lepton::CompiledExpression energyExpression;
    Lepton::CompiledExpression forceExpression;
    std::vector<double*> energyParticleParams;
    std::vector<double*> forceParticleParams;
    double* energyR;
    double* forceR;
};

} // namespace OpenMM

#endif // OPENMM_CPU_CUSTOM_NONBONDED_FORCE_H__
