TARGET ?= run

MAKE := make
CPP := g++
SRCDIR := src
TMPDIR := _

CFLAGS := -std=c++0x -Wall -O3 -I$(SRCDIR)

.PHONY: all deps clean

all:
	@$(MAKE) deps --no-print-directory -s
	@$(MAKE) $(TARGET) --no-print-directory

ls = $(wildcard $(1)/*)
recls = $(strip $(call ls,$(1)) $(foreach f,$(call ls,$(1)),$(call recls,$(f))))

ALLFILES = $(call recls,$(SRCDIR))
CPPFILES = $(filter %.cpp,$(ALLFILES))
HFILES = $(filter %.h,$(ALLFILES))

cpp2obj = $(patsubst $(SRCDIR)/%.cpp,$(TMPDIR)/obj/%.o,$(1))
OBJFILES = $(call cpp2obj,$(CPPFILES))
cpp2dep = $(patsubst $(SRCDIR)/%.cpp,$(TMPDIR)/dep/%.d,$(1))
DEPFILES = $(call cpp2dep,$(CPPFILES))

$(TMPDIR)/dep/%.d: $(SRCDIR)/%.cpp $(HFILES)
	@-mkdir -p $(@D)
	@$(CPP) $(CFLAGS) -MM $< -MT $(call cpp2obj,$<) -o $@

deps: $(DEPFILES)

$(TMPDIR)/obj/%.o: $(SRCDIR)/%.cpp
	@-mkdir -p $(@D)
	$(CPP) $(CFLAGS) $< -c -o $@

$(TARGET): $(OBJFILES)
	$(CPP) $(CFLAGS) $^ -o $@

-include $(DEPFILES)

clean:
	-rm -r _ $(TARGET)
