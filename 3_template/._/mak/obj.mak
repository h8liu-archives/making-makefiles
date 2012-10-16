$o/$n:
	@-mkdir -p $@

headers += $(wildcard $s/$n/*.h) 
sources := $(wildcard $s/$n/*.c)

$o/$n/%.o: $s/$n/%.c $(headers) | $o/$n
	$(CC) $(CFLAGS) $(filter %.c,$^) -c -o $@

objects := $(addprefix $o/$n/,$(patsubst %.c,%.o,$(notdir $(sources))))
