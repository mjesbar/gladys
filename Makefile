CXX = g++

# Qt6 Include and Library Paths (explicitly defined for robustness)
QT_BASE_INCLUDE_DIR = /usr/include/x86_64-linux-gnu/qt6
QT_LIBRARY_DIR = /usr/lib/x86_64-linux-gnu

# Explicit core Qt CXXFLAGS (no reliance on pkg-config for basic paths)
CORE_QT_CXXFLAGS = \
    -I$(QT_BASE_INCLUDE_DIR)/QtCore \
    -I$(QT_BASE_INCLUDE_DIR)/QtGui \
    -I$(QT_BASE_INCLUDE_DIR)/QtWidgets \
    -I$(QT_BASE_INCLUDE_DIR) \
    -I$(QT_BASE_INCLUDE_DIR)/QtX11Extras # Explicitly added QtX11Extras include path

# Explicit core Qt LDFLAGS
CORE_QT_LDFLAGS = -lQt6Core -lQt6Gui -lQt6Widgets

# Check for Qt6X11Extras and conditionally add its flags
QT_X11_EXTRAS_CXXFLAGS = $(shell pkg-config --cflags Qt6X11Extras 2>/dev/null)
QT_X11_EXTRAS_LDFLAGS = $(shell pkg-config --libs Qt6X11Extras 2>/dev/null)

# Compiler Flags
CXXFLAGS = -std=c++17 -Wall -fPIC -DQ_OS_LINUX $(CORE_QT_CXXFLAGS) $(QT_X11_EXTRAS_CXXFLAGS)

# Linker Flags
LDFLAGS = $(CORE_QT_LDFLAGS) $(QT_X11_EXTRAS_LDFLAGS) -lX11 -Wl,--no-as-needed

MOC = /usr/lib/qt6/libexec/moc

# Application Sources
APP_CPP_SOURCES = src/main.cpp src/mainwindow.cpp
APP_HEADERS = src/mainwindow.h

# Global Hotkey Monitor Sources (Linux only)
ifdef Q_OS_LINUX
GLOBAL_HOTKEY_CPP_SOURCES = src/globalhotkeymonitor_x11.cpp
GLOBAL_HOTKEY_HEADERS = src/globalhotkeymonitor_x11.h
else
GLOBAL_HOTKEY_CPP_SOURCES = # Empty for other platforms
GLOBAL_HOTKEY_HEADERS = # Empty for other platforms
endif

# All C++ source files that need to be compiled
ALL_CPP_SOURCES = $(APP_CPP_SOURCES) $(GLOBAL_HOTKEY_CPP_SOURCES)

# All headers that need MOC processing
ALL_MOC_HEADERS = $(APP_HEADERS) $(GLOBAL_HOTKEY_HEADERS)

# Generated MOC C++ files
MOC_MAINWINDOW_CPP = moc/mainwindow.moc.cpp
MOC_GLOBAL_HOTKEY_CPP = moc/globalhotkeymonitor_x11.moc.cpp
ALL_MOC_CPP_SOURCES = $(MOC_MAINWINDOW_CPP) $(MOC_GLOBAL_HOTKEY_CPP)

# Object files
OBJ_MAIN = moc/main.o
OBJ_MAINWINDOW = moc/mainwindow.o
OBJ_GLOBAL_HOTKEY = moc/globalhotkeymonitor_x11.o

OBJ_MOC_MAINWINDOW = moc/mainwindow.moc.o
OBJ_MOC_GLOBAL_HOTKEY = moc/globalhotkeymonitor_x11.moc.o

ALL_OBJECTS = $(OBJ_MAIN) $(OBJ_MAINWINDOW) $(OBJ_GLOBAL_HOTKEY) $(OBJ_MOC_MAINWINDOW) $(OBJ_MOC_GLOBAL_HOTKEY)

TARGET = gladys

# Phony targets
.PHONY: all build clean run mocfiles

# Default target
all: build

# Build target (forces clean and then builds the executable)
build: clean $(TARGET)

# --- MOC Generation Rules ---
moc/mainwindow.moc.cpp: src/mainwindow.h
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

moc/globalhotkeymonitor_x11.moc.cpp: src/globalhotkeymonitor_x11.h
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

# --- Compilation Rules (.cpp to .o) ---
$(OBJ_MAIN): src/main.cpp $(MOC_MAINWINDOW_CPP)
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MAINWINDOW): src/mainwindow.cpp $(MOC_MAINWINDOW_CPP)
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_GLOBAL_HOTKEY): src/globalhotkeymonitor_x11.cpp $(MOC_GLOBAL_HOTKEY_CPP)
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Compilation Rules (MOC .cpp to .o) ---
$(OBJ_MOC_MAINWINDOW): $(MOC_MAINWINDOW_CPP)
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_MOC_GLOBAL_HOTKEY): $(MOC_GLOBAL_HOTKEY_CPP)
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Final executable target
$(TARGET): $(ALL_OBJECTS) icons/mic-light.png icons/mic-dark.png
	$(CXX) $(CXXFLAGS) $(ALL_OBJECTS) -o $@ $(LDFLAGS)

icons/%.png: icons/%.svg
	convert -background none -size 64x64 $< $@

clean:
	rm -f $(TARGET) moc/*.o moc/*.moc moc/*.moc.cpp

run:
	@if [ "$$XDG_SESSION_TYPE" = "wayland" ]; then \
		echo "Running on Wayland, forcing XWayland..."; \
		QT_QPA_PLATFORM=xcb ./$(TARGET) & \
	else \
		echo "Running on X11 or other session type..."; \
		./$(TARGET) & \
	fi
