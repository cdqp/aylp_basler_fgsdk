project(
	'aylp_basler_fgsdk',
	'c',
	default_options: ['c_std=gnu17', 'warning_level=3', 'optimization=3']
	#default_options: ['warning_level=everything', 'optimization=3']
	# this is worth occasionally turning on to check
)

incdir = include_directories(['libfgsdk/include', 'libaylp'])

c = meson.get_compiler('c')
fgsdk_dep = c.find_library(
	'fglib5',
	dirs: join_paths(meson.source_root(), 'libfgsdk/lib'),
	required: true
)
gsl_dep = dependency('gsl')
json_dep = dependency('json-c')
deps = [fgsdk_dep, gsl_dep, json_dep]

shared_library('aylp_basler_fgsdk', 'aylp_basler_fgsdk.c',
	name_prefix: '',
	install: true,
	dependencies: deps,
	include_directories: incdir,
	override_options: 'b_lundef=false'
)

