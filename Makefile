CXX = g++

CXXFLAGS = -std=c++17 -O3 -Wall -fPIC -Isrc -Isrc/lib
QT_CXXFLAGS = $(shell pkg-config --cflags Qt6Widgets Qt6Gui Qt6Core Qt6Network)
QT_LDFLAGS = $(shell pkg-config --libs Qt6Widgets Qt6Gui Qt6Core Qt6Network)
LDFLAGS = $(QT_LDFLAGS) -lX11 -Wl,--no-as-needed

MOC = /usr/lib/qt6/libexec/moc

.PHONY: all clean run build

all: bin/gladys bin/gladysd

build:
	$(MAKE) clean
	$(MAKE) all

bin/gladys: bin/obj/gladys.o bin/obj/gladyswindow.o bin/obj/ipc_server.o \
           moc/gladyswindow.moc.cc moc/ipc_server.moc.cc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $^ -o $@ $(LDFLAGS)

bin/gladysd: bin/obj/gladysd.o bin/obj/gladyswindow.o bin/obj/ipc_server.o \
             bin/obj/process_utils.o bin/obj/x11_keygrab.o \
             moc/gladyswindow.moc.cc moc/ipc_server.moc.cc \
             moc/process_utils.moc.cc moc/x11_keygrab.moc.cc
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) $^ -o $@ $(LDFLAGS)

bin/obj/%.o: src/%.cc
	@mkdir -p bin/obj
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

bin/obj/%.o: src/lib/%.cc
	@mkdir -p bin/obj
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

moc/%.moc.cc: src/lib/%.h
	@mkdir -p moc
	$(MOC) $< -o $@

clean:
	rm -rf bin/obj moc bin/gladys bin/gladysd