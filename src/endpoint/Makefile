
OBJECTS=main.o socket.o
CXXFLAGS=-std=c++14
LDFLAGS=-lbluetooth -lboost_system -lpthread

EXEC=ble-client

.PHONY: clean

ifeq ($(DESTDIR),)
DESTDIR=/usr
endif

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: all
	install -m0755 $(EXEC) $(DESTDIR)/bin

clean:
	$(RM) -vf $(EXEC) $(OBJECTS)
