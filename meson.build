project(
	'MelOS Neo',
	'c',
	default_options: {
		'buildtype': 'debug',
		'warning_level': '3',
		'c_std': 'c23',
	},
)

raylib = dependency('raylib')

executable(
	'melosneo',
	'src/main.c',
	'src/mods/mystring.c',
	c_args: ['-DGRAPHICS_API_OPENGL_33', '-DPLATFORM_DESKTOP', '-g'],
	link_args: [],
	install: true,
	dependencies: [raylib],
)