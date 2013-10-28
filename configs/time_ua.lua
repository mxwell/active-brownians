integration = {
	iterations = {
		relaxation 	= 0 * 100 * 1000,
		observation  =  100 * 1000
	},
	time_step = 0.001,
	use_grid = false,
}
model = {
	number_of_particles = 100,
	rectangle_size = 10,
	local_visibility = false,
	-- if @local_visibility is true, then epsilon will be visibility radius
	epsilon = 1.0,
	mu = 10.0,
	noise_intensities = {
		passive_noise = 0.00,
		speed_noise = 0,
		angular_noise = 0.2
	},
	speed = {
		lowest 	= 0.5,
		highest	= 1.0
	}
}
