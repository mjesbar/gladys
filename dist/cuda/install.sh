# Thanks NVIDIA

PWD=$(pwd)

# CUDA 12.4
if [ ! -f cuda_12.4.0_550.54.14_linux.run ]; then
  wget https://developer.download.nvidia.com/compute/cuda/12.4.0/local_installers/cuda_12.4.0_550.54.14_linux.run
fi

chmod +x cuda_12.4.0_550.54.14_linux.run

mkdir -p ${PWD}/cuda-12.4.0

./cuda_12.4.0_550.54.14_linux.run \
  --silent \
  --toolkit \
  --override \
  --installpath=${PWD}/cuda-12.4.0 \
  --no-opengl-libs \
  --no-drm \
  --no-man-page

# cuDNN 8
if [ ! -f cudnn-linux-x86_64-8.9.7.29_cuda12-archive.tar.xz ]; then
  wget https://huggingface.co/csukuangfj/cudnn/resolve/main/cudnn-linux-x86_64-8.9.7.29_cuda12-archive.tar.xz
fi

tar xvf cudnn-linux-x86_64-8.9.7.29_cuda12-archive.tar.xz --strip-components=1 -C ${PWD}/cuda-12.4.0

# cuDNN 9 (Required for .so.9 support)
if [ ! -f cudnn-linux-x86_64-9.0.0.312_cuda12-archive.tar.xz ]; then
  wget https://developer.download.nvidia.com/compute/cudnn/redist/cudnn/linux-x86_64/cudnn-linux-x86_64-9.0.0.312_cuda12-archive.tar.xz
fi

tar xvf cudnn-linux-x86_64-9.0.0.312_cuda12-archive.tar.xz --strip-components=1 -C "${PWD}/cuda-12.4.0"
