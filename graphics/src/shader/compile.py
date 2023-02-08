import sys
import os

base_dir = sys.argv[1]

# create a directory for the binaries
bin_dir = base_dir + "/bin"
if not os.path.exists(bin_dir):
    os.makedirs(bin_dir)

# find all glsl source files in the base_dir
glsl_source_files=[]
for file in os.listdir(base_dir):
    if file.endswith(".vert") or file.endswith(".frag"):
        glsl_source_files.append(file) # contains only the file name, not the path

# compile all the source files
for file in glsl_source_files:
    input_file = base_dir + "/" + file
    bin_file = bin_dir + "/" + file + ".spv"
    print("Building Shader: " + bin_file)
    cmd = "glslc -c " + input_file + " -o " + bin_file # build the command that compiles the glsl source files
    os.system(cmd) # execute the command