RELEASE_FLAGS = -Werror
WARNINGS = -Wall -Wextra
ifeq ($(shell sh -c 'uname -o 2>/dev/null || echo not'),Cygwin)
  DEBUG =-g
else
  DEBUG =-g -D_GLIBCXX_DEBUG
endif
VERSION = 0.0.1
TARGET_NAME = libOCSCore
BUILD_DIR = $(CURDIR)
SRC_DIR = src
ODIR = obj
SHAREDMARK = -shared
W32ODIR = objwin
TARGET = $(BUILD_PREFIX)$(TARGET_NAME)
W32TARGET = $(TARGET)
OS  = $(shell uname -s)
ASTYLE_BINARY = astyle
# if $(OS) contains 'BSD'
ifneq ($(findstring BSD,$(OS)),)
  BSD = 1
endif

ifneq ($(findstring Darwin,$(OS)),)
  NATIVE = osx
endif
CXXFLAGS += -DBUILDING_DLL
LDFLAGS += $(PROFILE)

ifdef CLANG
  ifeq ($(NATIVE), osx)
    USE_LIBCXX = 1
  endif
  ifdef USE_LIBCXX
    OTHERS +=-stdlib=libc++
    LDFLAGS +=-stdlib=libc++
  endif
  ifdef CCACHE
    CXX = CCACHE_CPP2=1 ccache $(CROSS)clang++
    LD  = CCACHE_CPP2=1 ccache $(CROSS)clang++
  else
    CXX = $(CROSS)clang++
    LD  = $(CROSS)clang++
  endif
endif

ifndef RELEASE
  ifeq ($(shell $(CXX) -E -Og - < /dev/null > /dev/null 2>&1 && echo fog),fog)
    OPTLEVEL = -Og
  else
    OPTLEVEL = -O0
  endif
  CXXFLAGS += $(OPTLEVEL)
endif

ifeq ($(shell sh -c 'uname -o 2>/dev/null || echo not'),Cygwin)
    OTHERS +=-std=gnu++14
  else
    OTHERS +=-std=c++14
endif

CXXFLAGS += $(WARNINGS) $(DEBUG) $(PROFILE) $(OTHERS) -MMD

ifeq ($(TARGETSYSTEM),WINDOWS)
  BACKTRACE = 0
  EXTENSION = .dll
  TARGET := $(TARGET)$(EXTENSION)
  GETR = $(W32ODIR)
  ifdef DYNAMIC_LINKING
    # Windows isn't sold with programming support, these are static to remove MinGW dependency.
    #LDFLAGS += -static-libgcc -static-libstdc++
  else
    LDFLAGS += -static
  endif
#  W32FLAGS += -Wl,-stack,12000000,-subsystem,windows
  W32FLAGS += -Wl,--add-stdcall-alias,--out-implib,$(BUILD_PREFIX)$(TARGET_NAME).a
  RFLAGS = -J rc -O coff
  ifeq ($(NATIVE), win64)
    RFLAGS += -F pe-x86-64
  endif
  #LDFLAGS += -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion
endif

ifeq ($(TARGETSYSTEM),CYGWIN)
  BACKTRACE = 0
  ifeq ($(LOCALIZE),1)
    LDFLAGS += -lintl -liconv
  endif
endif

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
_OBJS = $(SOURCES:$(SRC_DIR)/%.cpp=%.o)
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))
ifdef LANGUAGES
  L10N = localization
endif

ifeq ($(BSD), 1)
  # BSDs have backtrace() and friends in a separate library
  ifeq ($(BACKTRACE), 1)
    LDFLAGS += -lexecinfo
    # ...which requires the frame pointer
    CXXFLAGS += -fno-omit-frame-pointer
  endif

 # And similarly, their libcs don't have gettext built in
  ifeq ($(LOCALIZE),1)
    LDFLAGS += -lintl -liconv
  endif
endif

# Global settings for Windows targets (at end)
ifeq ($(TARGETSYSTEM),WINDOWS)
	JAVA_HOME:=$(JAVA_HOME)
	CXXFLAGS += -I"$(JAVA_HOME)\include" -I"$(JAVA_HOME)\include\win32"
    LDFLAGS += -fPIC -L"$(JAVA_HOME)\lib" -ljvm
    #-lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion
endif

ifeq ($(TARGETSYSTEM), LINUX)
  BACKTRACE=1
  ifneq ($(PREFIX),)
    DEFINES += -DPREFIX="$(PREFIX)"
  endif
  JAVA_HOME:=$(shell echo $$(dirname $$(dirname $$(readlink -f $$(which javac)))))
  JVM_PATH:=$(shell dirname $$(find $(JAVA_HOME) -name "libjvm*"))
  CXXFLAGS += -fPIC -fvisibility=hidden
  CXXFLAGS += -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
  LDFLAGS += -fPIC -L$(JVM_PATH) -ljvm
  EXTENSION = .so
  TARGET := $(TARGET)$(EXTENSION)
endif

ifeq ($(TARGETSYSTEM), OSX)
  BACKTRACE=1
  ifneq ($(PREFIX),)
    DEFINES += -DPREFIX="$(PREFIX)"
  endif
  JAVA_HOME:=$(shell /usr/libexec/java_home)
  JVM_PATH:=$(shell dirname $$(find $(JAVA_HOME) -name "libjvm*"))
  SHAREDMARK = -dynamiclib
  CXXFLAGS += -I/opt/local/include -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin
  CXXFLAGS += -fPIC -fvisibility=hidden
  LDFLAGS += -L/opt/local/lib -L$(JVM_PATH) -ljvm
  EXTENSION = .dylib
  TARGET := $(TARGET)$(EXTENSION)
endif

ifeq ($(TARGETSYSTEM), CYGWIN)
  ifneq ($(PREFIX),)
    DEFINES += -DPREFIX="$(PREFIX)"
  endif
  DEFINES += -D_GLIBCXX_USE_C99_MATH_TR1
  EXTENSION = .dll
  TARGET := $(TARGET)$(EXTENSION)
endif

ifdef MSYS2
  DEFINES += -D_GLIBCXX_USE_C99_MATH_TR1
endif

LDFLAGS += -lssl -lcrypto

all: $(TARGET) $(L10N) tests
	@
$(TARGET): $(ODIR) $(OBJS)
	+$(LD) $(SHAREDMARK) $(W32FLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
ifdef RELEASE
  ifndef DEBUG_SYMBOLS
	$(STRIP) $(TARGET)
  endif
endif

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(DEFINES) $(CXXFLAGS) -c $< -o $@

export ODIR _OBJS LDFLAGS CXX W32FLAGS DEFINES CXXFLAGS VERSION
clean: clean-tests
	rm src/org_gtdev_oc_server_JNInterface.h
	rm -rf *$(TARGET_NAME).dll *$(TARGET_NAME).so *$(TARGET_NAME).dylib *$(TARGET_NAME).a
	rm -rf *obj
astyle:
	$(ASTYLE_BINARY) --options=.astylerc -n $(shell cat astyled_whitelist)

astyle-all: $(SOURCES) $(HEADERS)
	$(ASTYLE_BINARY) --options=.astylerc -n $(SOURCES) $(HEADERS) $(HPPS)

astyle-all-tests:
	$(MAKE) -C tests astyle-all

tests:
	$(MAKE) -C tests

clean-tests:
	$(MAKE) -C tests clean

.PHONY: tests check clean astyle astyle-all
