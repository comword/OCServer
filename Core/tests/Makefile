SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJS = $(SOURCES:%.cpp=$(ODIR)/%.o)
ASTYLE_BINARY = astyle
STRIP = $(CROSS)strip

MAIN_LIB=../$(BUILD_PREFIX)libOCSCore.ar

# If you invoke this makefile directly and the parent directory was
# built with BUILD_PREFIX set, you must set it for this invocation as well.
ODIR ?= obj

LDFLAGS += -L.

CXXFLAGS += -I../src -Wno-unused-variable -Wno-sign-compare -Wno-unknown-pragmas -Wno-parentheses

TEST_TARGET = $(BUILD_PREFIX)OCServer_test

tests: $(TEST_TARGET)

$(BUILD_PREFIX)OCServer_test: $(ODIR) $(OBJS) $(MAIN_LIB)
	+$(CXX) $(W32FLAGS) -o $@ $(DEFINES) $(OBJS) $(MAIN_LIB) $(CXXFLAGS) $(LDFLAGS)
ifdef RELEASE
  ifndef DEBUG_SYMBOLS
	$(STRIP) $(TEST_TARGET)
  endif
endif

# Iterate over all the individual tests.
check: $(TEST_TARGET)
	cd .. && tests/$(TEST_TARGET) -d yes

clean:
	rm -rf *obj
	rm -f *OCServer_test

astyle:
	$(ASTYLE_BINARY) --options="../.astylerc" -n $(shell cat astyled_whitelist)

astyle-all: $(SOURCES) $(HEADERS)
	$(ASTYLE_BINARY) --options="../.astylerc" -n $(SOURCES) $(HEADERS)

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.cpp
	$(CXX) $(DEFINES) $(CXXFLAGS) -c $< -o $@

.PHONY: clean check tests astyle-all astyle

.SECONDARY: $(OBJS)
