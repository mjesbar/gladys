CXX = g++

# FLAGS
CXXFLAGS = -std=c++17 -Wall -fPIC -DQ_OS_LINUX -Isrc -Isrc/lib
QT_CXXFLAGS = $(shell pkg-config --cflags Qt6Widgets Qt6Gui Qt6Core Qt6Network)
QT_LDFLAGS = $(shell pkg-config --libs Qt6Widgets Qt6Gui Qt6Core Qt6Network)
LDFLAGS = $(QT_LDFLAGS) -lX11 -Wl,--no-as-needed

MOC = /usr/lib/qt6/libexec/moc

# OBJECTS
OBJ_FILES = moc/main.o \
            moc/mainwindow.o \
            moc/globalhotkeymonitor_x11.o \
            moc/mainwindow.moc.o \
            moc/globalhotkeymonitor_x11.moc.o

TARGET = gladys

.PHONY: all clean run build

all: $(TARGET)

build:
	@$(MAKE) clean
	bear -- $(MAKE) all

$(TARGET): $(OBJ_FILES) icons/mic-light.png icons/mic-dark.png
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $(OBJ_FILES) -o $@ $(LDFLAGS)

# MOC rules
moc/mainwindow.moc.cpp: src/mainwindow.h
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

moc/globalhotkeymonitor_x11.moc.cpp: src/lib/globalhotkeymonitor_x11.h
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

# Compilation rules
moc/main.o: src/main.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/mainwindow.o: src/mainwindow.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/globalhotkeymonitor_x11.o: src/lib/globalhotkeymonitor_x11.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/mainwindow.moc.o: moc/mainwindow.moc.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/globalhotkeymonitor_x11.moc.o: moc/globalhotkeymonitor_x11.moc.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

icons/%.png: icons/%.svg
	convert -background none -size 64x64 $< $@

clean:
	rm -f $(TARGET) moc/*.o moc/*.moc moc/*.moc.cpp compile_commands.json

run:
	@if [ "$$XDG_SESSION_TYPE" = "wayland" ]; then \
		echo "Running on Wayland, forcing XWayland..."; \
		QT_QPA_PLATFORM=xcb ./$(TARGET) & \
	else \
		echo "Running on X11 or other session type..."; \
		./$(TARGET) & \
	fi
