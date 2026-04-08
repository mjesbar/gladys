CXX = g++

# FLAGS
CXXFLAGS = -std=c++17 -O3 -Wall -fPIC -Isrc -Isrc/lib
QT_CXXFLAGS = $(shell pkg-config --cflags Qt6Widgets Qt6Gui Qt6Core Qt6Network)
QT_LDFLAGS = $(shell pkg-config --libs Qt6Widgets Qt6Gui Qt6Core Qt6Network)
LDFLAGS = $(QT_LDFLAGS) -lX11 -Wl,--no-as-needed

MOC = /usr/lib/qt6/libexec/moc

# OBJECTS FOR GLADYS
GLADYS_OBJS = moc/gladys.o \
              moc/gladyswindow.o \
              moc/gladyswindow.moc.o

TARGET = bin/gladys
DAEMON_TARGET = bin/gladysd

.PHONY: all clean run build

all: $(TARGET) $(DAEMON_TARGET)

build:
	@$(MAKE) clean
	bear -- $(MAKE) all

$(TARGET): $(GLADYS_OBJS) icons/mic-light.png icons/mic-dark.png
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $(GLADYS_OBJS) -o $@ $(LDFLAGS)

$(DAEMON_TARGET): src/gladysd.cpp
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) src/gladysd.cpp -o $@ $(QT_LDFLAGS) -lX11

# MOC rules
moc/gladyswindow.moc.cpp: src/gladyswindow.h
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

# Compilation rules
moc/gladys.o: src/gladys.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/gladyswindow.o: src/gladyswindow.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/gladyswindow.moc.o: moc/gladyswindow.moc.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

icons/%.png: icons/%.svg
	convert -background none -size 64x64 $< $@

clean:
	rm -rf bin moc/*.o moc/*.moc moc/*.moc.cpp compile_commands.json

run:
	@if [ "$$XDG_SESSION_TYPE" = "wayland" ]; then \
		echo "Running on Wayland, forcing XWayland..."; \
		QT_QPA_PLATFORM=xcb ./bin/gladys & \
	else \
		echo "Running on X11 or other session type..."; \
		./bin/gladys & \
	fi