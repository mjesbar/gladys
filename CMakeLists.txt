cmake_minimum_required(VERSION 3.21)
project(Gladys VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# Qt6 (standard project setup: AUTOMOC/AUTOUIC/AUTORCC + C++17)
# =============================================================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 LabsPlatform Widgets Network Svg)
qt_standard_project_setup(REQUIRES 6.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
qt_add_executable(gladys WIN32 MACOSX_BUNDLE
  src/main.cc
  src/modules/log/log.cc
  src/modules/ai/stt.cc
  src/modules/ai/llm.cc
  src/modules/key/toggle.cc
  src/modules/key/paste.cc
  src/modules/setting/setting.cc
  src/modules/ui/x11_shadow.cc
  src/modules/ui/audio_bars.cc
  src/modules/ui/settings.cc
  src/modules/ui/tray.cc
)

set_source_files_properties(src/qml/components/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(gladys
  URI Gladys
  VERSION 1.0
  QML_FILES
    src/qml/components/Theme.qml
    src/qml/components/Mic.qml
    src/qml/components/Card.qml
    src/qml/components/IconButton.qml
    src/qml/components/ThemedCheckBox.qml
    src/qml/components/Field.qml
    src/qml/windows/Overlay.qml
    src/qml/windows/Splash.qml
    src/qml/windows/Settings.qml
)

qt_add_resources(gladys app_resources PREFIX "/" BASE src/assets FILES
  src/assets/fonts/RobotoSlab-Regular.ttf
  src/assets/fonts/RobotoSlab-Bold.ttf
  src/assets/icons/spinner.svg
  src/assets/icons/icon.svg
  src/assets/icons/icon-app.svg
  src/assets/icons/plus.svg
  src/assets/icons/trash.svg
  src/assets/icons/chevron-right.svg
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
# target_compile_definitions(gladys PRIVATE MA_DEBUG_OUTPUT)

target_link_libraries(gladys PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::LabsPlatform Qt6::Widgets Qt6::Network Qt6::Svg)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(GLADYS_RPATH "@executable_path/lib")
else()
  set(GLADYS_RPATH "$ORIGIN;$ORIGIN/lib")
endif()
set_target_properties(gladys PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  INSTALL_RPATH "${GLADYS_RPATH}"
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
  # Qt DLLs load from <exe>/lib, set by main() before first use.
  target_link_libraries(gladys PRIVATE user32)
  target_link_options(gladys PRIVATE
    /DELAYLOAD:Qt6Core.dll /DELAYLOAD:Qt6Gui.dll /DELAYLOAD:Qt6Qml.dll
    /DELAYLOAD:Qt6Quick.dll /DELAYLOAD:Qt6QuickControls2.dll /DELAYLOAD:Qt6LabsPlatform.dll
    /DELAYLOAD:Qt6Widgets.dll /DELAYLOAD:Qt6Network.dll /DELAYLOAD:Qt6Svg.dll
  )
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
    target_link_options(gladys PRIVATE
      /DELAYLOAD:sherpa-onnx-c-api.dll /DELAYLOAD:sherpa-onnx-cxx-api.dll
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
  install(FILES ${SHERPA_RUNTIME_LIBS} DESTINATION lib)
else()
  message(WARNING "sherpa-onnx library not found - speech-to-text will not work")
endif()

# =============================================================================
# Install
# =============================================================================
install(TARGETS gladys RUNTIME DESTINATION . BUNDLE DESTINATION .)

if(APPLE)
  set(deploy_app "gladys.app")
else()
  set(deploy_app "gladys${CMAKE_EXECUTABLE_SUFFIX}")
endif()
set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_gladys.cmake")
file(WRITE ${deploy_script} "
set(QT_DEPLOY_BIN_DIR \"lib\")
set(QT_DEPLOY_LIB_DIR \"lib\")
set(QT_DEPLOY_PLUGINS_DIR \"plugins\")
set(QT_DEPLOY_QML_DIR \"qml\")
include(\"${QT_DEPLOY_SUPPORT}\")
qt_deploy_runtime_dependencies(EXECUTABLE \"\${CMAKE_INSTALL_PREFIX}/${deploy_app}\")
")
install(SCRIPT ${deploy_script})
foreach(plugin platforms imageformats styles tls iconengines xcbglintegrations)
  if(IS_DIRECTORY ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}/${plugin})
    install(DIRECTORY ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}/${plugin} DESTINATION plugins)
  endif()
endforeach()
install(DIRECTORY ${QT6_INSTALL_PREFIX}/lib/ DESTINATION lib
  FILES_MATCHING PATTERN "libQt6*.so*" PATTERN "libicu*.so*")
install(DIRECTORY ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_QML}/QtQuick DESTINATION qml)
install(DIRECTORY ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_QML}/Qt/labs DESTINATION qml/Qt)
install(CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/qt.conf\" \"[Paths]\\nPlugins = plugins\\nQml2Imports = qml\\n\")")
