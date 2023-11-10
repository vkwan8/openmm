float2 drudeParams = PARAMS[index]; // screeningScale = thole; energyScale = ONE_4PI_EPS0*charge1*charge2;
real3 force1 = make_real3(0);
real3 force2 = make_real3(0);
real3 force3 = make_real3(0);
real3 force4 = make_real3(0);

// First pair.

real3 delta = make_real3(pos1.x-pos3.x, pos1.y-pos3.y, pos1.z-pos3.z); // This has length r
real rInv = RSQRT(dot(delta, delta));
real r = RECIP(rInv);
real u = drudeParams.x*r;
real screening = ERF(u);
real pairEnergy = drudeParams.y*screening*rInv; // U = drudeParams.y*screening*rInv then du = -drudeParams.y*screening*rInv*rInv + 
energy += pairEnergy;
real3 f = -delta*rInv*(drudeParams.y*rInv*(-screening*rInv+2*drudeParams.x*EXP(-u*u)/SQRT(3.14159f))); // M_2_SQRTPI*drudeParams.x*EXP(-u*u)
force1 += f;
force3 -= f;

// Second pair.

delta = make_real3(pos1.x-pos4.x, pos1.y-pos4.y, pos1.z-pos4.z);
rInv = RSQRT(dot(delta, delta));
r = RECIP(rInv);
u = drudeParams.x*r;
screening = ERF(u);
pairEnergy = -drudeParams.y*screening*rInv;
energy += pairEnergy;
f = delta*rInv*(drudeParams.y*rInv*(-screening*rInv+2*drudeParams.x*EXP(-u*u)/SQRT(3.14159f)));
force1 += f;
force4 -= f;

// Third pair.

delta = make_real3(pos2.x-pos3.x, pos2.y-pos3.y, pos2.z-pos3.z);
rInv = RSQRT(dot(delta, delta));
r = RECIP(rInv);
u = drudeParams.x*r;
screening = ERF(u);
pairEnergy = -drudeParams.y*screening*rInv;
energy += pairEnergy;
f = delta*rInv*(drudeParams.y*rInv*(-screening*rInv+2*drudeParams.x*EXP(-u*u)/SQRT(3.14159f)));
force2 += f;
force3 -= f;

// Fourth pair.

delta = make_real3(pos2.x-pos4.x, pos2.y-pos4.y, pos2.z-pos4.z);
rInv = RSQRT(dot(delta, delta));
r = RECIP(rInv);
u = drudeParams.x*r;
screening = ERF(u);
pairEnergy = drudeParams.y*screening*rInv;
energy += pairEnergy;
f = -delta*rInv*(drudeParams.y*rInv*(-screening*rInv+2*drudeParams.x*EXP(-u*u)/SQRT(3.14159f)));
force2 += f;
force4 -= f;
