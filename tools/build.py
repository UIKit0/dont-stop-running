#!/usr/bin/python
import os
import subprocess

ROOT = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")
ASSETS_DIR = os.path.normpath(os.path.join(ROOT, "extern", "assets"))
BUILD_DIR = os.path.normpath(os.path.join(ROOT, "bin", "data"))
SHADER_COMPILER = os.path.normpath(os.path.join(ROOT, "extern", "bgfx", ".build", "linux64_gcc", "bin", "shadercRelease"))

class color:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def usage():
	print "usage: build"

def requires_update(destination, source):
	return not os.path.exists(destination) or os.path.getctime(destination) < os.path.getmtime(source)

def build_textures():
	CONVERT_CMD = "nvcompress -bc2 %s %s"

	print "============================="
	print "Building textures"
	print "============================="
	print

	# make sure destination path exists
	if not os.path.exists(os.path.join(BUILD_DIR, "textures")):
		os.mkdir(os.path.join(BUILD_DIR, "textures"))

	for file in os.listdir(os.path.join(ASSETS_DIR, "textures")):
		print "* Converting:", color.OKGREEN + file + color.ENDC

		in_file = os.path.join(ASSETS_DIR, "textures", file)
		out_file = os.path.join(BUILD_DIR, "textures", file)
		out_file = os.path.splitext(out_file)[0] + ".dds"

		if not requires_update(out_file, in_file):
			print "Destination is up to date, skipping"
		else:
			subprocess.call(CONVERT_CMD % (in_file, out_file), shell = True)
		print

def build_shaders():

	BUILD_CMD = SHADER_COMPILER + " -f %s -o %s --type %s --platform %s --varyingdef %s"

	print "============================="
	print "Building shaders"
	print "============================="
	print

	# make sure destination path exists
	if not os.path.exists(os.path.join(BUILD_DIR, "shaders")):
		os.mkdir(os.path.join(BUILD_DIR, "shaders"))

	shader_set = set()

	# explore shaders
	for file in os.listdir(os.path.join(ASSETS_DIR, "shaders")):
		ext = os.path.splitext(file)[1]
		if ext == ".frag":
			shader_set.add(os.path.splitext(file)[0])

	print "Found shaders:",
	for shader in shader_set:
		print shader,
	print "\n"

	for shader in shader_set:
		print "* Building shader", color.OKGREEN + shader + color.ENDC, "\n"

		fragment_in = os.path.join(ASSETS_DIR, "shaders", shader + ".frag")
		vertex_in = os.path.join(ASSETS_DIR, "shaders", shader + ".vert")
		varying_in = os.path.join(ASSETS_DIR, "shaders", shader + "_varying.def")

		fragment_out = os.path.join(BUILD_DIR, "shaders", shader + ".frag.bin")
		vertex_out = os.path.join(BUILD_DIR, "shaders", shader + ".vert.bin")

		if not os.path.exists(fragment_in):
			print color.WARNING + "Could not find fragment shader for: " + shader + color.ENDC
			continue
		if not os.path.exists(vertex_in):
			print color.WARNING + "Could not find vertex shader for: " + shader + color.ENDC
			continue
		if not os.path.exists(varying_in):
			print color.WARNING + "Could not find varying definition for: " + shader + color.ENDC
			continue

		print "** Building vertex shader"

		if not (requires_update(vertex_out, vertex_in) or requires_update(vertex_out, varying_in)):
			print "Destination is up to date, skipping"
		else:
			subprocess.call(BUILD_CMD % 
				(vertex_in, vertex_out, "v", "linux", varying_in), 
				shell = True)
		print		

		print "** Building fragment shader"

		if not (requires_update(fragment_out, fragment_in) or requires_update(fragment_out, varying_in)):
			print "Destination is up to date, skipping"
		else:
			subprocess.call(BUILD_CMD % 
				(fragment_in, fragment_out, "f", "linux", varying_in), 
				shell = True)
		print		

if __name__ == "__main__":
	build_textures()
	build_shaders();
