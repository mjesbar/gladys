cmake_minimum_required(VERSION 3.16)
project(Gladys VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# Compiler & build settings
# =============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
foreach(CONFIG DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_SOURCE_DIR}/dist)
endforeach()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# =============================================================================
# Architecture
# =============================================================================
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  set(GLADYS_ARCH "arm")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
  set(GLADYS_ARCH "x64")
else()
  message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# =============================================================================
# Source files
# =============================================================================
set(GLADYS_SOURCES
  src/gladys.cc
  src/modules/gladysd.cc
  src/modules/ipc.cc
  src/modules/stt.cc
  src/modules/llm.cc
  src/modules/keygrab.cc
  src/modules/keytype.cc
  src/modules/ui.cc
)

# =============================================================================
# Include directories
# =============================================================================
set(INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/src/include
)

# =============================================================================
# Qt6
# =============================================================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

# =============================================================================
# Miniaudio (single-header library, define implementation in stt.cc)
# =============================================================================
set_source_files_properties(src/modules/stt.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)
add_compile_definitions(MA_DEBUG_OUTPUT)

# =============================================================================
# X11 (Linux only)
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)
endif()

# =============================================================================
# Sherpa-ONNX (Speech-to-Text)
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(SHERPA_ONNX_OS_DIR "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(SHERPA_ONNX_OS_DIR "macos")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(SHERPA_ONNX_OS_DIR "win")
endif()

set(SHERPA_ONNX_LIB_DIR
  ${CMAKE_SOURCE_DIR}/src/include/sherpa-onnx/lib/${SHERPA_ONNX_OS_DIR}/${GLADYS_ARCH}
)

if(EXISTS ${SHERPA_ONNX_LIB_DIR})
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SHERPA_ONNX_LIBS
      -L${SHERPA_ONNX_LIB_DIR}
      -Wl,-rpath,$ORIGIN
      -lsherpa-onnx-c-api
      -lsherpa-onnx-cxx-api
      -lonnxruntime
    )
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SHERPA_ONNX_LIBS
      -L${SHERPA_ONNX_LIB_DIR}
      -Wl,-rpath,@loader_path
      -lsherpa-onnx-c-api
      -lsherpa-onnx-cxx-api
      -lonnxruntime
    )
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SHERPA_ONNX_LIBS
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.lib
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-cxx-api.lib
      ${SHERPA_ONNX_LIB_DIR}/onnxruntime.lib
    )
  endif()
else()
  message(WARNING "sherpa-onnx library not found - speech-to-text will not work")
  set(SHERPA_ONNX_LIBS "")
endif()

# =============================================================================
# Platform-specific libraries
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(PLATFORM_LIBS ${X11_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(PLATFORM_LIBS "-framework CoreGraphics" "-framework CoreFoundation")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(PLATFORM_LIBS user32)
endif()

# =============================================================================
# Executable: gladys
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(GLADYS_EXE_NAME "gladys")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(GLADYS_EXE_NAME "gladys.app")
else()
  set(GLADYS_EXE_NAME "gladys")
endif()

add_executable(gladys ${GLADYS_SOURCES})
set_target_properties(gladys PROPERTIES OUTPUT_NAME "${GLADYS_EXE_NAME}")
target_include_directories(gladys PRIVATE ${INCLUDE_DIRS})
target_link_libraries(gladys PRIVATE
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Svg
  ${PLATFORM_LIBS}
  ${SHERPA_ONNX_LIBS}
)

# =============================================================================
# Install
# =============================================================================
install(TARGETS gladys RUNTIME DESTINATION bin)

# =============================================================================
# Run target
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gladys
    COMMENT "Running gladys"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gladys.app
    COMMENT "Running gladys"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gladys.exe
    COMMENT "Running gladys"
  )
endif()
