integration = {
	iterations = {
		relaxation 	= 1000 * 1000,
		observation  = 100 * 1000
	},
	time_step = 0.005,
	use_grid = true,
}
model = {
	number_of_particles = 10000,
	rectangle_size = 10,
	local_visibility = true,
	-- if @local_visibility is true, then it'll be visibility radius
	epsilon = 0.1,
	mu = 1.0,
	noise_intensities = {
		passive_noise = 0.01,
		speed_noise = 0,
		angular_noise = {
			start 	= 0.024545664690001,
			finish	= 0.50,
			step	= 0.01,
			-- if @log_step present, then @step will be ignored
			log_step = math.pow(10 / 3.0, 1 / 6.0)
		}
	},
	speed = {
		lowest 	= -1.0,
		highest	= 1.0
	}
}

