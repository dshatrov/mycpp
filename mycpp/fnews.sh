#!/bin/sh

# This script finds non-grabbed 'new' operations

fx () {
    STR=$1
    EXT="h hpp c cpp cc pl S def mod"

    echo $EXT |
	sed 's/\([^ \t]*\)/-name "*.\1" -o/g; s/-o$//; s/^/find /' |
	    sh |
		xargs -l1 -i0 grep -Hn "$STR" 0
}

fx "new " | sed -n 's/^.*grab (\(new\|static_cast\) .*$//; t; p'

