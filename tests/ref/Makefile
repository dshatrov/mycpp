COMMON_CFLAGS =				\
	-D_POSIX_C_SOURCE=199309L	\
	-D_XOPEN_SOURCE=600		\
	-ggdb -pedantic -O2		\
	-Wno-long-long -Wall -Wextra	\
	-rdynamic			\
	`pkg-config --cflags mycpp-1.0`

CFLAGS = -std=c99 $(COMMON_CFLAGS)
CXXFLAGS = -std=c++98 -ansi $(COMMON_CFLAGS) -fno-default-inline

LDFLAGS = `pkg-config --libs mycpp-1.0`

.PHONY: all clean

TARGETS = test__ref

all: $(TARGETS)

test__ref: test__ref.cpp

clean:
	rm -f $(TARGETS)

