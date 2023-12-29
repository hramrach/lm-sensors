LIB_DIR		:= lib
LIB_TEST_DIR	:= lib/test

LIB_TEST_TARGETS := $(LIB_TEST_DIR)/test-scanner
LIB_TEST_SOURCES := $(LIB_TEST_DIR)/test-scanner.c

LIB_TEST_SCANNER_OBJS := \
	$(LIB_TEST_DIR)/test-scanner.ro \
	$(LIB_DIR)/conf-lex.ao \
	$(LIB_DIR)/error.ao \
	$(LIB_DIR)/general.ao \
	$(LIB_DIR)/strtod/strtod.ao \
	$(LIB_DIR)/strtod/nan64.ao

# Include all dependency files. We use '.rd' to indicate this will create
# executables.
INCLUDEFILES += $(LIB_TEST_SOURCES:.c=.rd)

$(LIB_TEST_DIR)/test-scanner: $(LIB_TEST_SCANNER_OBJS)
	$(CC) -o $@ $(LIB_TEST_SCANNER_OBJS) -Llib $(EXLDFLAGS)

all-lib-test: $(LIB_TEST_TARGETS)
user :: all-lib-test

$(LIB_TEST_DIR)/test-scanner.rd: $(LIB_DIR)/conf-parse.c

clean-lib-test:
	$(RM) $(LIB_TEST_DIR)/*.rd $(LIB_TEST_DIR)/*.ro
	$(RM) $(LIB_TEST_TARGETS)
clean :: clean-lib-test
