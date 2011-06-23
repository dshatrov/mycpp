# NOTE: -O2 is important for this test
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

TARGETS = test__print_task

all: $(TARGETS)

test__print_task: test__print_task.cpp

clean:
	rm -f $(TARGETS)

