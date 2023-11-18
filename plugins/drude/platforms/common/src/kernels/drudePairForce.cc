float2 drudeParams = PARAMS[index]; 
real3 force1 = make_real3(0);
real3 force2 = make_real3(0);
real3 force3 = make_real3(0);
real3 force4 = make_real3(0);

// First pair.

real3 delta = make_real3(pos1.x-pos3.x, pos1.y-pos3.y, pos1.z-pos3.z); // This has length r
real rInv = RSQRT(dot(delta, delta));
real r = RECIP(rInv);
real u = drudeParams.x*r;
real screening = (r<0.5)?ERF(u):1;
real dscreening = (r<0.5)?1.1283791671*drudeParams.x*EXP(-u*u):0;
real pairEnergy = drudeParams.y*screening*rInv;
energy += pairEnergy;
real3 f = -delta*rInv*(drudeParams.y*rInv*(-screening*rInv+dscreening)); 
force1 += f;
force3 -= f;

// Second pair.

delta = make_real3(pos1.x-pos4.x, pos1.y-pos4.y, pos1.z-pos4.z);
rInv = RSQRT(dot(delta, delta));
r = RECIP(rInv);
u = drudeParams.x*r;
screening = (r<0.5)?ERF(u):1;
dscreening = (r<0.5)?1.1283791671*drudeParams.x*EXP(-u*u):0;
pairEnergy = -drudeParams.y*screening*rInv;
energy += pairEnergy;
f = delta*rInv*(drudeParams.y*rInv*(-screening*rInv+dscreening)); 
force1 += f;
force4 -= f;

// Third pair.

delta = make_real3(pos2.x-pos3.x, pos2.y-pos3.y, pos2.z-pos3.z);
rInv = RSQRT(dot(delta, delta));
r = RECIP(rInv);
u = drudeParams.x*r;
screening = (r<0.5)?ERF(u):1;
dscreening = (r<0.5)?1.1283791671*drudeParams.x*EXP(-u*u):0;
pairEnergy = -drudeParams.y*screening*rInv;
energy += pairEnergy;
f = delta*rInv*(drudeParams.y*rInv*(-screening*rInv+dscreening)); 
force2 += f;
force3 -= f;

// Fourth pair.

delta = make_real3(pos2.x-pos4.x, pos2.y-pos4.y, pos2.z-pos4.z);
rInv = RSQRT(dot(delta, delta));
r = RECIP(rInv);
u = drudeParams.x*r;
screening = (r<0.5)?ERF(u):1;
dscreening = (r<0.5)?1.1283791671*drudeParams.x*EXP(-u*u):0;
pairEnergy = drudeParams.y*screening*rInv;
energy += pairEnergy;
f = -delta*rInv*(drudeParams.y*rInv*(-screening*rInv+dscreening)); 
force2 += f;
force4 -= f;
