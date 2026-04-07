CXX = g++

# Compiler Flags
CXXFLAGS = -std=c++17 -Wall -fPIC -DQ_OS_LINUX

# Qt6 Flags (using pkg-config for simplicity)
QT_CXXFLAGS = $(shell pkg-config --cflags Qt6Widgets Qt6Gui Qt6Core)
QT_LDFLAGS = $(shell pkg-config --libs Qt6Widgets Qt6Gui Qt6Core)

# Linker Flags
LDFLAGS = $(QT_LDFLAGS) -lX11 -Wl,--no-as-needed # -lX11 is for X11 related functionality if needed by QtCore/QtGui directly

MOC = /usr/lib/qt6/libexec/moc

# Source files
APP_CPP_SOURCES = src/main.cpp src/mainwindow.cpp
APP_HEADERS = src/mainwindow.h

# MOC generated files
MOC_CPP_SOURCES = $(APP_HEADERS:src/%.h=moc/%.moc.cpp)

# Object files
OBJ_FILES = $(APP_CPP_SOURCES:src/%.cpp=moc/%.o) $(MOC_CPP_SOURCES:moc/%.cpp=moc/%.o)

TARGET = gladys

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ_FILES) icons/mic-light.png icons/mic-dark.png
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $(OBJ_FILES) -o $@ $(LDFLAGS)

moc/%.moc.cpp: src/%.h
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

moc/%.o: moc/%.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/%.o: src/%.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

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
