cmake_minimum_required(VERSION 3.21)
project(Gladys VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# Qt6 (standard project setup: AUTOMOC/AUTOUIC/AUTORCC + C++17)
# =============================================================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)
qt_standard_project_setup(REQUIRES 6.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)

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
# Executable: gladys
# =============================================================================
qt_add_executable(gladys WIN32
  src/main.cc
  src/modules/log/log.cc
  src/modules/ai/stt.cc
  src/modules/ai/llm.cc
  src/modules/key/toggle.cc
  src/modules/key/paste.cc
  src/modules/setting/setting.cc
  src/modules/ui/app_window.cc
  src/modules/ui/mic.cc
  src/modules/ui/setting_window.cc
)

target_include_directories(gladys PRIVATE
  src
  src/modules
  src/modules/ai
  src/modules/setting
  src/modules/ui
  src/modules/key
  src/include
)

# Miniaudio: single-header library, implementation compiled in stt.cc only
set_source_files_properties(src/modules/ai/stt.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)
target_compile_definitions(gladys PRIVATE MA_DEBUG_OUTPUT)

target_link_libraries(gladys PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Svg)

set_target_properties(gladys PROPERTIES
  INSTALL_RPATH "$ORIGIN"
  BUILD_WITH_INSTALL_RPATH TRUE
)

# =============================================================================
# Platform libraries
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)
  target_link_libraries(gladys PRIVATE ${X11_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_libraries(gladys PRIVATE "-framework CoreGraphics" "-framework CoreFoundation")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(gladys PRIVATE user32)
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
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(gladys PRIVATE
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.lib
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-cxx-api.lib
      ${SHERPA_ONNX_LIB_DIR}/onnxruntime.lib
    )
  else()
    target_link_directories(gladys PRIVATE ${SHERPA_ONNX_LIB_DIR})
    target_link_libraries(gladys PRIVATE sherpa-onnx-c-api sherpa-onnx-cxx-api onnxruntime)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(gladys PROPERTIES BUILD_RPATH "@loader_path")
  endif()

  # Copy sherpa-onnx runtime libraries next to the executable
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.so*)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.dylib)
  else()
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.dll)
  endif()
  foreach(LIB ${SHERPA_RUNTIME_LIBS})
    configure_file(${LIB} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ COPYONLY)
  endforeach()
else()
  message(WARNING "sherpa-onnx library not found - speech-to-text will not work")
endif()

# =============================================================================
# Install
# =============================================================================
install(TARGETS gladys RUNTIME DESTINATION bin)
