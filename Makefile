SOURCES = proxy.c orientation.c
TESTSOURCES = test.cpp wiimote.cpp wiimoteRegistry.cpp wiimoteServer.cpp
APPNAME = proxy
TESTAPPNAME = test

CFLAGS += -lpthread -lcwiid -lm
LDLIBS += -lpthread -lcwiid -lm

proxy: $(SOURCES) $(TESTSOURCES)
	g++ $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(SOURCES) -o $(APPNAME)
	g++ $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(TESTSOURCES) -o $(TESTAPPNAME)


clean:
	rm -rf proxy test *.o
