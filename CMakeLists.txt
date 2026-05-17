cmake_minimum_required(VERSION 3.16)
project(Gladys VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  set(GLADYS_ARCH "arm")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
  set(GLADYS_ARCH "x64")
else()
  message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

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
  ${CMAKE_SOURCE_DIR}/include/sherpa-onnx/include
)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

# X11 is Linux-only
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)
endif()

# Map OS name to sherpa-onnx directory
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(SHERPA_ONNX_OS_DIR "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(SHERPA_ONNX_OS_DIR "macos")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(SHERPA_ONNX_OS_DIR "win")
endif()

# sherpa-onnx library path (dynamic per OS and architecture)
set(SHERPA_ONNX_LIB_DIR
  ${CMAKE_SOURCE_DIR}/include/sherpa-onnx/${SHERPA_ONNX_OS_DIR}/${GLADYS_ARCH}/lib
)

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

# Executable name depends on target OS
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(GLADYS_EXE_NAME "gladys.exe")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(GLADYS_EXE_NAME "gladys.app")
else()
  set(GLADYS_EXE_NAME "gladys")
endif()

# Executable: gladys (single entry point)
add_executable(gladys ${GLADYS_SOURCES})
set_target_properties(gladys PROPERTIES OUTPUT_NAME "${GLADYS_EXE_NAME}")
target_include_directories(gladys PRIVATE ${INCLUDE_DIRS})

target_link_libraries(gladys PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::Network
  Qt6::Svg
)

# Platform-specific libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(gladys PRIVATE ${X11_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_libraries(gladys PRIVATE
    "-framework CoreGraphics"
    "-framework CoreFoundation"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(gladys PRIVATE user32)
endif()

# Add stt.cc specific compile definition for MINIAUDIO_IMPLEMENTATION
set_source_files_properties(src/lib/modules/stt.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)

# Link sherpa-onnx libraries (custom path)
if(EXISTS ${SHERPA_ONNX_LIB_DIR})
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(gladys PRIVATE
      -L${SHERPA_ONNX_LIB_DIR}
      -Wl,-rpath,${SHERPA_ONNX_LIB_DIR}
      -lsherpa-onnx-c-api
      -lsherpa-onnx-cxx-api
      -lonnxruntime
    )
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(gladys PRIVATE
      -L${SHERPA_ONNX_LIB_DIR}
      -Wl,-rpath,${SHERPA_ONNX_LIB_DIR}
      -lsherpa-onnx-c-api
      -lsherpa-onnx-cxx-api
      -lonnxruntime
    )
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(gladys PRIVATE
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.lib
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-cxx-api.lib
      ${SHERPA_ONNX_LIB_DIR}/onnxruntime.lib
    )
  endif()
else()
  message(WARNING "sherpa-onnx library not found - speech-to-text will not work")
endif()

# Install targets
install(TARGETS gladys RUNTIME DESTINATION bin)

# Run the application
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${SHERPA_ONNX_LIB_DIR}:$ENV{LD_LIBRARY_PATH}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gladys
    COMMENT "Running gladys"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E env DYLD_LIBRARY_PATH=${SHERPA_ONNX_LIB_DIR}:$ENV{DYLD_LIBRARY_PATH}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gladys.app
    COMMENT "Running gladys"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gladys.exe
    COMMENT "Running gladys"
  )
endif()
