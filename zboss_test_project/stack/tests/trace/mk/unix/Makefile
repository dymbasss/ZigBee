# PURPOSE: Trace test makefile
# */

BUILD_HOME=../..
include $(BUILD_HOME)/Options

SRCS = trace_test.c

$(BIN)trace_test$(EXE) : $(OBJS) $(LIBS)
	$(LINK_BINARY)

BINS = $(BIN)trace_test$(EXE)

BINS_TEST=$(BINS)

all: $(BINS)
