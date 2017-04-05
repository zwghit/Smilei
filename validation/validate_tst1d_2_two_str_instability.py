import os, re, numpy as np
from Smilei import *

S = Smilei(".", verbose=False)

some_particles_x = S.TrackParticles.ion(axes=["x"],select="any(t==0, x<0.02)").getData()["x"][0]
Validate("Regularly spaced particles", some_particles_x, 1e-7)

momentum_distribution = S.ParticleDiagnostic.Diag0(timesteps=0, slice={"x":"all"}).getData()[0]
Validate("Electron momentum distribution", momentum_distribution, 1e-7 )

itimes = S.ParticleDiagnostic.Diag0().getAvailableTimesteps()
Validate("Timesteps in particle diagnostic", itimes )

px = S.ParticleDiagnostic.Diag0(slice={"x":"all"}, timesteps=2000).getData()[0]
Validate("Electron momentum distribution", px, 0.1)