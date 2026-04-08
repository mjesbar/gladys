CXX = g++

# FLAGS
CXXFLAGS = -std=c++17 -O3 -Wall -fPIC -Isrc -Isrc/lib
QT_CXXFLAGS = $(shell pkg-config --cflags Qt6Widgets Qt6Gui Qt6Core Qt6Network)
QT_LDFLAGS = $(shell pkg-config --libs Qt6Widgets Qt6Gui Qt6Core Qt6Network)
LDFLAGS = $(QT_LDFLAGS) -lX11 -Wl,--no-as-needed

MOC = /usr/lib/qt6/libexec/moc

# OBJECTS FOR GLADYS
GLADYS_OBJS = moc/gladys.o \
              lib/gladyswindow.o \
              moc/gladyswindow.moc.o \
              lib/ipc_server.o \
              moc/ipc_server.moc.o

DAEMON_OBJS = lib/ipc_server.o \
              lib/process_utils.o \
              lib/x11_keygrab.o \
              moc/ipc_server.moc.o \
              moc/process_utils.moc.o \
              moc/x11_keygrab.moc.o

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

$(DAEMON_TARGET): $(DAEMON_OBJS) src/gladysd.cpp
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) src/gladysd.cpp $(DAEMON_OBJS) -o $@ $(LDFLAGS)

# MOC rules
moc/gladyswindow.moc.cpp: src/lib/gladyswindow.hpp
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

moc/ipc_server.moc.cpp: src/lib/ipc_server.hpp
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

moc/process_utils.moc.cpp: src/lib/process_utils.hpp
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

moc/x11_keygrab.moc.cpp: src/lib/x11_keygrab.hpp
	@mkdir -p moc
	$(MOC) $< -o $(@:.cpp=)
	@echo "#include <QObject>" > $@
	@cat $(@:.cpp=) >> $@

# Compilation rules
moc/gladys.o: src/gladys.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

lib/gladyswindow.o: src/lib/gladyswindow.cc
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/gladyswindow.moc.o: moc/gladyswindow.moc.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

lib/ipc_server.o: src/lib/ipc_server.cc
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/ipc_server.moc.o: moc/ipc_server.moc.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/process_utils.moc.o: moc/process_utils.moc.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/x11_keygrab.moc.o: moc/x11_keygrab.moc.cpp
	@mkdir -p moc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

lib/process_utils.o: src/lib/process_utils.cc
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

lib/x11_keygrab.o: src/lib/x11_keygrab.cc
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

icons/%.png: icons/%.svg
	convert -background none -size 64x64 $< $@

clean:
	rm -rf bin moc/*.o moc/*.moc moc/*.moc.cpp lib/*.o compile_commands.json

run:
	@if [ "$$XDG_SESSION_TYPE" = "wayland" ]; then \
		echo "Running on Wayland, forcing XWayland..."; \
		QT_QPA_PLATFORM=xcb ./bin/gladys & \
	else \
		echo "Running on X11 or other session type..."; \
		./bin/gladys & \
	fi