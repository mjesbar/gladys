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
  ${CMAKE_SOURCE_DIR}/src/lib/modules
  ${CMAKE_SOURCE_DIR}/include/miniaudio
  ${CMAKE_SOURCE_DIR}/include/sherpa-onnx/shared/include
)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

# Find X11
find_package(X11 REQUIRED)

# sherpa-onnx library path
set(SHERPA_ONNX_LIB_DIR ${CMAKE_SOURCE_DIR}/include/sherpa-onnx/shared/lib)

# CUDA (optional)
set(CUDA_LIB_DIR ${CMAKE_SOURCE_DIR}/bin/cuda/cuda-12.4.0/targets/x86_64-linux/lib)

# Source files - all modules in one executable
set(GLADYS_SOURCES
  src/gladys.cc
  src/lib/modules/gladysd.cc
  src/lib/modules/ipc.cc
  src/lib/modules/stt.cc
  src/lib/modules/llm.cc
  src/lib/modules/keygrab.cc
  src/lib/modules/keytype.cc
  src/lib/modules/ui.cc
)

# Executable: gladys (single entry point)
add_executable(gladys ${GLADYS_SOURCES})
target_include_directories(gladys PRIVATE ${INCLUDE_DIRS})
target_link_libraries(gladys PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::Network
  Qt6::Svg
  ${X11_LIBRARIES}
)

# Add stt.cc specific compile definition for MINIAUDIO_IMPLEMENTATION
set_source_files_properties(src/lib/modules/stt.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)

# Link sherpa-onnx libraries (custom path)
if(EXISTS ${SHERPA_ONNX_LIB_DIR})
  target_link_libraries(gladys PRIVATE
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
  target_link_libraries(gladys PRIVATE
    -L${CUDA_LIB_DIR}
    -Wl,-rpath,${CUDA_LIB_DIR}
  )
endif()

# Install targets
install(TARGETS gladys RUNTIME DESTINATION bin)

# Run the application
add_custom_target(run
  COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${SHERPA_ONNX_LIB_DIR}:$ENV{LD_LIBRARY_PATH}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gladys
  COMMENT "Running gladys"
)
