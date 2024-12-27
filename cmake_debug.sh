DIR_NAME=`basename ${PWD}`
mkdir -p ../${DIR_NAME}-debug
cd ../${DIR_NAME}-debug
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Debug" ../${DIR_NAME}
