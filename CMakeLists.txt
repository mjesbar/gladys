cmake_minimum_required(VERSION 3.16)
project(Gladys VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Debug output
add_compile_definitions(MA_DEBUG_OUTPUT)

# Include directories
set(INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/lib
  ${CMAKE_SOURCE_DIR}/include/miniaudio
  ${CMAKE_SOURCE_DIR}/include/sherpa-onnx/shared/include
)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# Find X11
find_package(X11 REQUIRED)

# sherpa-onnx library path
set(SHERPA_ONNX_LIB_DIR ${CMAKE_SOURCE_DIR}/include/sherpa-onnx/shared/lib)

# CUDA (optional)
set(CUDA_LIB_DIR ${CMAKE_SOURCE_DIR}/bin/cuda/cuda-12.4.0/targets/x86_64-linux/lib)

# Source files
set(GLADYS_SOURCES
  src/gladys.cc
  src/lib/ipc.cc
  src/lib/pm.cc
  src/lib/ui.cc
  src/lib/keytype.cc
)

set(GLADYSD_SOURCES
  src/gladysd.cc
  src/lib/ipc.cc
  src/lib/pm.cc
  src/lib/keygrab.cc
  src/lib/stt.cc
  src/lib/keytype.cc
)

# Executable: gladys (GUI)
add_executable(gladys ${GLADYS_SOURCES})
target_include_directories(gladys PRIVATE ${INCLUDE_DIRS})
target_link_libraries(gladys PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::Network
  ${X11_LIBRARIES}
)

# Executable: gladysd (Daemon)
add_executable(gladysd ${GLADYSD_SOURCES})
target_include_directories(gladysd PRIVATE ${INCLUDE_DIRS})

# Add stt.cc specific compile definition for MINIAUDIO_IMPLEMENTATION
set_source_files_properties(src/lib/stt.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)

target_link_libraries(gladysd PRIVATE
  Qt6::Core
  Qt6::Network
  ${X11_LIBRARIES}
)

# Link sherpa-onnx libraries (custom path)
if(EXISTS ${SHERPA_ONNX_LIB_DIR})
  target_link_libraries(gladysd PRIVATE
    -L${SHERPA_ONNX_LIB_DIR}
    -Wl,-rpath,${SHERPA_ONNX_LIB_DIR}
    -lsherpa-onnx-c-api
    -lsherpa-onnx-cxx-api
    -lcargs
    -lonnxruntime
  )
else()
  message(WARNING "sherpa-onnx library not found - speech-to-text will not work")
endif()

# Link CUDA if available
if(EXISTS ${CUDA_LIB_DIR})
  target_link_libraries(gladysd PRIVATE
    -L${CUDA_LIB_DIR}
    -Wl,-rpath,${CUDA_LIB_DIR}
  )
endif()

# Copy model directory to bin/models
add_custom_target(copy_models ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/bin/models
    ${CMAKE_SOURCE_DIR}/bin/models
  COMMENT "Copying models to bin directory"
)

# Install targets
install(TARGETS gladys gladysd RUNTIME DESTINATION bin)