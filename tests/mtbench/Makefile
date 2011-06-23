COMMON_CFLAGS =				\
	-D_POSIX_C_SOURCE=199309L	\
	-D_XOPEN_SOURCE=600		\
	-ggdb -O3 -pedantic		\
	-Wno-long-long -Wall -Wextra	\
	`pkg-config --cflags mycpp-1.0`

CFLAGS = -std=c99 $(COMMON_CFLAGS)
CXXFLAGS = -std=c++98 -ansi $(COMMON_CFLAGS)

LDFLAGS = `pkg-config --libs mycpp-1.0`

.PHONY: all clean

TARGETS = test__mtbench

all: $(TARGETS)

test__vstack: test__mtbench.cpp

clean:
	rm -f $(TARGETS)

