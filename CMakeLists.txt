cmake_minimum_required(VERSION 3.21)
# Supported targets: linux x64, windows x64, macOS arm64 (ignored elsewhere)
# Target arch (x64 | arm64), passed by scripts/build.sh; selects the sherpa-onnx lib dir
set(GLADYS_ARCH "x64" CACHE STRING "Target arch: x64 or arm64")
if(NOT GLADYS_ARCH MATCHES "^(x64|arm64)$")
  message(FATAL_ERROR "Unsupported GLADYS_ARCH: ${GLADYS_ARCH}")
endif()
if(GLADYS_ARCH STREQUAL "arm64")
  set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "" FORCE)
else()
  set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "" FORCE)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "" FORCE)
# Project name, version and minimum CMake
project(Gladys VERSION 1.0.0 LANGUAGES CXX)

# Find Qt6 modules and set C++17
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets Network Svg HttpServer)
qt_standard_project_setup(REQUIRES 6.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Runtime search path so each executable finds the bundled libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(GLADYS_RPATH "@executable_path/../Frameworks")
else()
  set(GLADYS_RPATH "$ORIGIN;$ORIGIN/lib")
endif()

# Miniaudio debug output, enabled with: cmake -DGLADYS_MA_DEBUG=ON
option(GLADYS_MA_DEBUG "Enable miniaudio debug output" OFF)

# ---------------------------------------------------------------------------
# gladys: GUI process (ui, core, keytoggle, keypaste, settings threads)
# ---------------------------------------------------------------------------
qt_add_executable(gladys WIN32 MACOSX_BUNDLE
  src/main.cc
  src/core.cc
  src/modules/log/log.cc
  src/modules/keytoggle/toggle.cc
  src/modules/keypaste/paste.cc
  src/modules/audio/levels.cc
  src/modules/settings/setting.cc
  src/modules/settings/system.cc
  src/modules/settings/sound.cc
  src/modules/settings/appearance.cc
  src/modules/settings/format.cc
  src/modules/settings/keyboard.cc
  src/modules/settings/meetings.cc
  src/modules/settings/history.cc
  src/modules/ui/osshadow.cc
  src/modules/ui/waves.cc
  src/modules/ui/settings.cc
  src/modules/ui/tray.cc
)

# App icon per OS: Windows exe resource (.ico), macOS bundle (.icns), Linux window icon set at runtime from icon.svg
if(WIN32)
  target_sources(gladys PRIVATE src/assets/icons/icon.rc)
elseif(APPLE)
  set_source_files_properties(src/assets/icons/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  target_sources(gladys PRIVATE src/assets/icons/icon.icns)
  set_target_properties(gladys PROPERTIES MACOSX_BUNDLE_ICON_FILE "icon.icns")
  enable_language(OBJCXX)
  target_sources(gladys PRIVATE src/modules/ui/macos.mm)
endif()

# QML module and its files; Theme is a singleton
set_source_files_properties(src/qml/components/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(gladys
  URI Gladys
  VERSION 1.0
  QML_FILES
    src/qml/components/Theme.qml
    src/qml/components/Mic.qml
    src/qml/components/Section.qml
    src/qml/components/Note.qml
    src/qml/components/MappingEditor.qml
    src/qml/components/ui/Text.qml
    src/qml/components/ui/Heading.qml
    src/qml/components/ui/Icon.qml
    src/qml/components/ui/Box.qml
    src/qml/components/ui/Container.qml
    src/qml/components/ui/Flex.qml
    src/qml/components/ui/Stack.qml
    src/qml/components/ui/Grid.qml
    src/qml/components/ui/Divider.qml
    src/qml/components/ui/Button.qml
    src/qml/components/ui/Input.qml
    src/qml/components/ui/Textarea.qml
    src/qml/components/ui/Checkbox.qml
    src/qml/components/ui/Radio.qml
    src/qml/components/ui/RadioGroup.qml
    src/qml/components/ui/Switch.qml
    src/qml/components/ui/Link.qml
    src/qml/components/ui/FormField.qml
    src/qml/components/ui/Alert.qml
    src/qml/components/ui/Badge.qml
    src/qml/components/ui/Spinner.qml
    src/qml/components/ui/Skeleton.qml
    src/qml/components/ui/Portal.qml
    src/qml/components/ui/Popover.qml
    src/qml/components/ui/Tooltip.qml
    src/qml/components/ui/DropdownMenu.qml
    src/qml/components/ui/Select.qml
    src/qml/components/ui/Dialog.qml
    src/qml/components/ui/Toast.qml
    src/qml/components/ui/Tabs.qml
    src/qml/components/ui/Breadcrumbs.qml
    src/qml/components/ui/TitleBar.qml
    src/qml/components/settings/SoundInputSource.qml
    src/qml/components/settings/FormatDictionary.qml
    src/qml/components/settings/FormatSnippets.qml
    src/qml/components/settings/FormatModes.qml
    src/qml/components/settings/SystemCloudLocal.qml
    src/qml/components/settings/SystemAiModels.qml
    src/qml/components/settings/SystemStoreTranscriptions.qml
    src/qml/components/settings/SoundEnableSoundEffects.qml
    src/qml/components/settings/SoundEnableLlm.qml
    src/qml/components/settings/SoundFxs.qml
    src/qml/components/settings/AppearanceTheme.qml
    src/qml/components/settings/AppearanceOverlaySize.qml
    src/qml/components/settings/AppearancePosition.qml
    src/qml/components/settings/KeyboardPushToTalkToggle.qml
    src/qml/components/settings/KeyboardShortcuts.qml
    src/qml/components/settings/MeetingsMeetings.qml
    src/qml/components/settings/HistoryActivity.qml
    src/qml/components/settings/HistoryStatus.qml
    src/qml/components/settings/HistoryErrors.qml
    src/qml/components/settings/HistoryLogs.qml
    src/qml/windows/Overlay.qml
    src/qml/windows/Splash.qml
    src/qml/windows/Settings.qml
)

# Embed fonts and icons into the binary
qt_add_resources(gladys app_resources PREFIX "/" BASE src/assets FILES
  src/assets/fonts/RobotoSlab-Regular.ttf
  src/assets/fonts/RobotoSlab-Bold.ttf
  src/assets/fonts/Molle-Regular.ttf
  src/assets/icons/spinner.svg
  src/assets/icons/title.svg
  src/assets/icons/icon.svg
  src/assets/icons/icon-app.svg
  src/assets/icons/plus.svg
  src/assets/icons/trash.svg
  src/assets/icons/chevron-right.svg
)

# Miniaudio: device enumeration lives in the Sound menu
set_source_files_properties(src/modules/settings/sound.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)

target_include_directories(gladys PRIVATE src src/include)
target_link_libraries(gladys PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Widgets Qt6::Network Qt6::Svg Qt6::HttpServer)

# OS-specific system libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)
  target_link_libraries(gladys PRIVATE ${X11_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_libraries(gladys PRIVATE "-framework CoreGraphics" "-framework CoreFoundation" "-framework ApplicationServices")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(gladys PRIVATE user32)
endif()

# Install the app; Qt runtime is bundled afterwards by *deployqt in scripts/build.sh
install(TARGETS gladys RUNTIME DESTINATION . BUNDLE DESTINATION .)

# ---------------------------------------------------------------------------
# gladys-asr: speech-to-text process (miniaudio capture + sherpa-onnx)
# ---------------------------------------------------------------------------
qt_add_executable(gladys-asr
  src/apis/asr/server.cc
  src/apis/asr/stt.cc
  src/modules/log/log.cc
)
target_include_directories(gladys-asr PRIVATE src src/include)
target_link_libraries(gladys-asr PRIVATE Qt6::Core Qt6::HttpServer)
set_source_files_properties(src/apis/asr/stt.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)
if(GLADYS_MA_DEBUG)
  target_compile_definitions(gladys-asr PRIVATE MA_DEBUG_OUTPUT)
endif()

# Link and install the speech-to-text runtime (sherpa-onnx)
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
    target_link_libraries(gladys-asr PRIVATE
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.lib
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-cxx-api.lib
      ${SHERPA_ONNX_LIB_DIR}/onnxruntime.lib
    )
    target_link_options(gladys-asr PRIVATE
      /DELAYLOAD:sherpa-onnx-c-api.dll /DELAYLOAD:sherpa-onnx-cxx-api.dll
    )
  else()
    target_link_directories(gladys-asr PRIVATE ${SHERPA_ONNX_LIB_DIR})
    target_link_libraries(gladys-asr PRIVATE sherpa-onnx-c-api sherpa-onnx-cxx-api onnxruntime)
  endif()
  # macOS build-tree run: look for the dylibs next to the executable
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(gladys-asr PROPERTIES BUILD_RPATH "@loader_path")
  endif()

  # Pick the runtime libraries for this OS
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.so*)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.dylib)
  else()
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.dll)
  endif()
  # Install them where the app's runtime search path looks
  if(WIN32)
    install(FILES ${SHERPA_RUNTIME_LIBS} DESTINATION .)
  elseif(APPLE)
    install(FILES ${SHERPA_RUNTIME_LIBS} DESTINATION gladys.app/Contents/Frameworks)
  else()
    install(FILES ${SHERPA_RUNTIME_LIBS} DESTINATION lib)
  endif()
else()
  message(FATAL_ERROR "sherpa-onnx library not found: ${SHERPA_ONNX_LIB_DIR}")
endif()

# ---------------------------------------------------------------------------
# gladys-llm: transcription formatting process (owns llama-server)
# ---------------------------------------------------------------------------
qt_add_executable(gladys-llm
  src/apis/llm/server.cc
  src/apis/llm/llm.cc
  src/apis/llm/prompt.cc
  src/modules/log/log.cc
)
target_include_directories(gladys-llm PRIVATE src)
target_link_libraries(gladys-llm PRIVATE Qt6::Core Qt6::Network Qt6::HttpServer)

# Helpers sit next to gladys (macOS: inside the bundle, next to the llama.cpp folder)
foreach(helper gladys-asr gladys-llm)
  set_target_properties(${helper} PROPERTIES
    INSTALL_RPATH "${GLADYS_RPATH}"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endforeach()
set_target_properties(gladys PROPERTIES
  INSTALL_RPATH "${GLADYS_RPATH}"
  BUILD_WITH_INSTALL_RPATH TRUE
)
if(APPLE)
  install(TARGETS gladys-asr gladys-llm RUNTIME DESTINATION gladys.app/Contents/MacOS)
else()
  install(TARGETS gladys-asr gladys-llm RUNTIME DESTINATION .)
endif()
