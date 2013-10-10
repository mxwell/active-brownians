integration = {
	iterations = {
		relaxation 	= 100 * 1000,
		observation  = 1000 * 1000
	},
	time_step = 0.005,
	use_grid = false,
}
model = {
	number_of_particles = 100,
	rectangle_size = 10,
	local_visibility = true,
	-- if @local_visibility is true, then epsilon will be visibility radius
	epsilon = {
		start = 0.1,
		finish = 0.3,
		log_step = math.pow(0.3 / 0.1, 1 / 4.0)
	},
	mu = 1.0,
	noise_intensities = {
		passive_noise = 0.01,
		speed_noise = 0,
		angular_noise = {
			start 	= 0.01,
			finish	= 0.50,
			step	= 0.01,
			-- if @log_step present, then @step will be ignored
			log_step = math.pow(0.5 / 0.01, 1 / 4.0)
		}
	},
	speed = {
		lowest 	= -1.0,
		highest	= 1.0
	}
}
