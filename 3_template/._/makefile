CC = gcc
AR = ar

# Source and used library directories
i = inc
s = src

# Output directories
o = obj
r = release

CFLAGS = -I./$i -O3 -Wall

.PHONY: all
all: $r/libhello.a $r/libbye.a test

# Make output directories

$r:
	@mkdir -p $@

# libhello.a
n = hello
headers := $i/hello.h
include mak/obj.mak
$r/lib$n.a: $(objects) | $r
	$(AR) rcs $@ $^


# libbye.a
n = bye
headers := $i/bye.h
include mak/obj.mak
$r/lib$n.a: $(objects) | $r
	$(AR) rcs $@ $^

# test
n = test
headers := $i/hello.h $i/bye.h
include mak/obj.mak
$n: $(objects) $r/libhello.a $r/libbye.a
	$(CC) $(CFLAGS) -L$r $(filter %.o,$^) -o $@ -lhello -lbye


# Clean

.PHONY: clean

clean:
	-rm -rf $o $r
	-rm -f test

# Run

.PHONY: run

run: test
	./test
