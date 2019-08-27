/* -------------------------------------------------------------------------- *
 *                                   OpenMM                                   *
 * -------------------------------------------------------------------------- *
 * This is part of the OpenMM molecular simulation toolkit originating from   *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2019 Stanford University and the Authors.           *
 * Authors: Andreas Krämer and Andrew C. Simmonett                            *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

#include "openmm/internal/AssertionUtilities.h"
#include "openmm/NoseHooverChain.h"
#include "openmm/CMMotionRemover.h"
#include "openmm/DrudeNoseHooverIntegrator.h"
#include "openmm/Context.h"
#include "openmm/State.h"
#include "openmm/HarmonicBondForce.h"
#include "openmm/VirtualSite.h"
#include "openmm/NonbondedForce.h"
#include "openmm/CustomExternalForce.h"
#include "openmm/System.h"
#include "openmm/DrudeForce.h"
#include "SimTKOpenMMRealType.h"
#include "sfmt/SFMT.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace OpenMM;
using namespace std;
//extern "C" OPENMM_EXPORT void registerDrudeReferenceKernelFactories();
extern "C" OPENMM_EXPORT void registerKernelFactories();
Platform& initializePlatform(int argc, char* argv[]);

void testWaterBox(Platform& platform){
    // Create a box of SWM4-NDP water molecules.  This involves constraints, virtual sites,
    // and Drude particles.
    const int gridSize = 3;
    const int numMolecules = gridSize*gridSize*gridSize;
    const double spacing = 0.8;
    const double boxSize = spacing*(gridSize+1);
    const double temperature = 300.0;
    const double temperatureDrude = 10.0;
    System system;
    NonbondedForce* nonbonded = new NonbondedForce();
    DrudeForce* drude = new DrudeForce();
    CMMotionRemover* cmm = new CMMotionRemover(1);
    system.addForce(cmm);
    system.addForce(nonbonded);
    system.addForce(drude);
    system.setDefaultPeriodicBoxVectors(Vec3(boxSize, 0, 0), Vec3(0, boxSize, 0), Vec3(0, 0, boxSize));
    nonbonded->setNonbondedMethod(NonbondedForce::CutoffPeriodic);
    nonbonded->setCutoffDistance(1.2);
    nonbonded->setSwitchingDistance(0.8);
    for (int i = 0; i < numMolecules; i++) {
        int startIndex = system.getNumParticles();
        system.addParticle(15.6); // O
        system.addParticle(0.4);  // D
        system.addParticle(1.0);  // H1
        system.addParticle(1.0);  // H2
        system.addParticle(0.0);  // M
        nonbonded->addParticle(1.71636, 0.318395, 0.21094*4.184);
        nonbonded->addParticle(-1.71636, 1, 0);
        nonbonded->addParticle(0.55733, 1, 0);
        nonbonded->addParticle(0.55733, 1, 0);
        nonbonded->addParticle(-1.11466, 1, 0);
        for (int j = 0; j < 5; j++)
            for (int k = 0; k < j; k++)
                nonbonded->addException(startIndex+j, startIndex+k, 0, 1, 0);
        system.addConstraint(startIndex, startIndex+2, 0.09572);
        system.addConstraint(startIndex, startIndex+3, 0.09572);
        system.addConstraint(startIndex+2, startIndex+3, 0.15139);
        system.setVirtualSite(startIndex+4, new ThreeParticleAverageSite(startIndex, startIndex+2, startIndex+3, 0.786646558, 0.106676721, 0.106676721));
        drude->addParticle(startIndex+1, startIndex, -1, -1, -1, -1.71636, ONE_4PI_EPS0*1.71636*1.71636/(100000*4.184), 1, 1);
    }
    vector<Vec3> positions;
    for (int i = 0; i < gridSize; i++)
        for (int j = 0; j < gridSize; j++)
            for (int k = 0; k < gridSize; k++) {
                Vec3 pos(i*spacing, j*spacing, k*spacing);
                positions.push_back(pos);
                positions.push_back(pos);
                positions.push_back(pos+Vec3(0.09572, 0, 0));
                positions.push_back(pos+Vec3(-0.023999, 0.092663, 0));
                positions.push_back(pos);
            }
    
    int numStandardDof = 3*3*numMolecules - system.getNumConstraints();
    int numDrudeDof = 3*numMolecules;
    int numDof = numStandardDof+numDrudeDof;
    // Simulate it and check the temperature.
    
    int chainLength = 4;
    int numMTS = 3;
    int numYS = 3;
    double frequency = 100.0;
    double frequencyDrude = 80.0;
    int randomSeed = 100;
    DrudeNoseHooverIntegrator integ(0.0005, system, temperature, temperatureDrude,
                                    frequency, frequencyDrude, chainLength, numMTS, numYS);;
    Context context(system, integ, platform);
    context.setPositions(positions);
    context.setVelocitiesToTemperature(temperature, randomSeed);
    std::vector<Vec3> velocities = context.getState(State::Velocities).getVelocities();
    for (int i = 0; i < numMolecules; i++){
        Vec3 noize;
        for (int j = 0; j < 3; j++){
            noize[j] = float(((i+18311)*(j+18253) * 313419097822414) % 18313) / float(18313);
            noize[j] *= sqrt(3 * BOLTZ * temperatureDrude / 0.4);
        }
        velocities[5*i+1] = velocities[5*i] + noize;
    }
    context.setVelocities(velocities);
    context.applyConstraints(1e-6);
    // Equilibrate.
    
    integ.step(800);
    
    // Compute the internal and center of mass temperatures.
    
    double totalKE = 0;
    const int numSteps = 800;
    double meanTemp = 0.0;
    double meanDrudeTemp = 0.0;
    double meanConserved = 0.0;
    for (int i = 0; i < numSteps; i++) {
        integ.step(1);
        State state = context.getState(State::Energy);
        double KE = state.getKineticEnergy();
        double PE = state.getPotentialEnergy();
        double fullKE = integ.computeTotalKineticEnergy();
        double drudeKE = integ.computeDrudeKineticEnergy();
        double temp = KE/(0.5*numStandardDof*BOLTZ);
        double drudeTemp = drudeKE/(0.5*numDrudeDof*BOLTZ);
        meanTemp = (i*meanTemp + temp)/(i+1);
        meanDrudeTemp = (i*meanDrudeTemp + drudeTemp)/(i+1);
        double heatBathEnergy = integ.computeHeatBathEnergy();
        double conserved = PE + fullKE + heatBathEnergy;
        meanConserved = (i*meanConserved + conserved)/(i+1);
#define DEBUG 0
#if DEBUG
        if(i%10 == 0)
        std::cout << std::setw(6) << i
                  << std::setprecision(8) << std::setw(16) << KE
                  << std::setprecision(8) << std::setw(16) << drudeKE
                  << std::setprecision(8) << std::setw(16) << meanTemp
                  << std::setprecision(8) << std::setw(16) << meanDrudeTemp
                  << std::setprecision(8) << std::setw(16) << heatBathEnergy
                  << std::setprecision(8) << std::setw(16) << fullKE
                  << std::setprecision(8) << std::setw(16) << conserved
                  << std::endl;
#endif
        totalKE += KE;
        ASSERT(fabs(meanConserved - conserved) < 0.2);
    }
    totalKE /= numSteps;
    ASSERT_USUALLY_EQUAL_TOL(temperature, meanTemp, 0.03);
    ASSERT_USUALLY_EQUAL_TOL(temperatureDrude, meanDrudeTemp, 0.03);
}

int main(int argc, char* argv[]) {
    try {
        Platform& platform = initializePlatform(argc, argv);
        //registerDrudeReferenceKernelFactories();
        //registerKernelFactories();
        testWaterBox(platform);
    }
    catch(const exception& e) {
        cout << "exception: " << e.what() << endl;
        return 1;
    }
    cout << "Done" << endl;
    return 0;
}

