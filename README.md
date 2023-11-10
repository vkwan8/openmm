## OpenMM: A High Performance Molecular Dynamics Library

## BK3 Fork

Introduction
------------

[OpenMM](http://openmm.org) is a toolkit for molecular simulation. It can be used either as a stand-alone application for running simulations, or as a library you call from your own code. It
provides a combination of extreme flexibility (through custom forces and integrators), openness, and high performance (especially on recent GPUs) that make it truly unique among simulation codes.  

This OpenMM fork modifies the DrudeSCFIntegrator to enable simulation of polarizable Drude models with Gaussian charge density.

For all intermolecular drude particle pairs (i,j), pass the following value as sigma: $\frac{1}{\sqrt{(2(\sigma_i^2+\sigma_j^2))}}$

`DrudeForce.addScreenedPair(i,j,sigma)`

Getting Help
------------

Need Help? Check out the [documentation](http://docs.openmm.org/) and [discussion forums](https://simtk.org/forums/viewforum.php?f=161).
