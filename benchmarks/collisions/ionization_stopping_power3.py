# ---------------------------------------------
# SIMULATION PARAMETERS FOR THE PIC-CODE SMILEI
# ---------------------------------------------

import math
L0 = 2.*math.pi # conversion from normalization length to wavelength


Main(
    geometry = "1Dcartesian",

    number_of_patches = [ 4 ],

    interpolation_order = 2,

    timestep = 0.25 * L0,
    simulation_time = 5000 * L0,


    time_fields_frozen = 100000000000.,

    cell_length = [1.*L0],
    grid_length = [40.*L0],

    EM_boundary_conditions = [ ["periodic"] ],


    random_seed = 0,

	reference_angular_frequency_SI = L0 * 3e8 /1.e-6,
    print_every = 100,
)




el = "electron1"
E = 50. # keV
E /= 511.
vel = math.sqrt(1.-1./(1.+E)**2)
mom = math.sqrt((1.+E)**2-1.)
Species(
	name = el,
	position_initialization = "regular",
	momentum_initialization = "maxwell-juettner",
	particles_per_cell= 2,
	mass = 1.0,
	charge = -1.0,
	charge_density = 1e-9,
	mean_velocity = [vel, 0., 0.],
	temperature = [0.0000000001]*3,
	time_frozen = 100000000.0,
	boundary_conditions = [
		["periodic", "periodic"],
	],
	c_part_max = 10.
)

Species(
	name = "ion1",
	position_initialization = "regular",
	momentum_initialization = "maxwell-juettner",
	particles_per_cell= 2,
	mass = 1836.0*27.,
	charge = 0,
	number_density = 1.,
	mean_velocity = [0., 0., 0.],
	temperature = [0.00000000001]*3,
	time_frozen = 100000000.0,
	boundary_conditions = [
		["periodic", "periodic"],
	],
	atomic_number = 13
)


Collisions(
	species1 = [el],
	species2 = ["ion1"],
	coulomb_log = 0.00000001,
	ionizing = True
)




DiagFields(
	every = 1000000
)


DiagScalar(
	every = 1000000000
)



DiagParticleBinning(
	deposited_quantity = "weight_px",
	every = 100,
	species = [el],
	axes = [
		 ["x",    0.,    Main.grid_length[0],   1]
	]
)
DiagParticleBinning(
	deposited_quantity = "weight",
	every = 100,
	species = [el],
	axes = [
		 ["x",    0.,    Main.grid_length[0],   1]
	]
)
