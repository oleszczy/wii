OLDSOURCES = proxy.c orientation.c
SOURCES = test.cpp wiimote.cpp wiimoteRegistry.cpp wiimoteServer.cpp
OLDAPPNAME = oldproxy
APPNAME = proxy

CFLAGS += -lpthread -lcwiid -lm
LDLIBS += -lpthread -lcwiid -lm

proxy: $(SOURCES) $(OLDSOURCES)
	g++ $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(SOURCES) -o $(APPNAME)
	g++ $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(OLDSOURCES) -o $(OLDAPPNAME)


clean:
	rm -rf proxy oldproxy *.o
