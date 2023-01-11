base_dir="${1}" # pass in the directory as the first parameter


vert_source_files=`ls $base_dir/*.vert`
frag_source_files=`ls $base_dir/*.frag`

glsl_source_files="$vert_source_files $frag_source_files"

mkdir -p $base_dir/bin

for glsl in $glsl_source_files
do
echo "Building Shader: "$glsl
file_name=$(basename "${glsl}")
glslc -c $glsl -o $base_dir/bin/$file_name.spv
done
