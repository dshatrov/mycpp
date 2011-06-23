CPPFLAGS = -ggdb -Wall `pkg-config --cflags mycpp-1.0`
LIBS = `pkg-config --libs mycpp-1.0`

.PHONY: all clean

TARGETS = ads

all: $(TARGETS)

%.o: %.cpp
	g++ $(CPPFLAGS) -c -o $@ $^

ads: ads.o
	g++ -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGETS) *.o

