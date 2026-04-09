# Thanks! https://github.com/k2-fsa/sherpa-onnx.git

PWD=$(pwd)

# Activate CUDA 12.4.0 for this build
source env.sh

echo "${CUDA_HOME}"

# exit

if [ ! -d "${PWD}/repo" ]; then
  echo "Cloning sherpa-onnx repository ..."
  git clone https://github.com/k2-fsa/sherpa-onnx ./repo
  echo "DONE!"
fi

make -j12
make install

# Shared Libraries
cd repo
mkdir build-shared
cd build-shared

cmake \
  -DSHERPA_ONNX_ENABLE_C_API=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_INSTALL_PREFIX=${PWD}/shared \
  -DSHERPA_ONNX_ENABLE_GPU=ON \
  ..

make -j12
make install
