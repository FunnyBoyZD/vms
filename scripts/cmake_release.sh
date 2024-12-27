DIR_NAME=`basename ${PWD}`
mkdir -p ../${DIR_NAME}-release
cd ../${DIR_NAME}-release
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" ../${DIR_NAME}
