# RELEASE_FLAGS = -Werror
WARNINGS = -Wall -Wextra
ifeq ($(shell sh -c 'uname -o 2>/dev/null || echo not'),Cygwin)
  DEBUG =-g
else
  DEBUG =-g -D_GLIBCXX_DEBUG
endif
VERSION = 0.0.1
VERSION_STR=$(shell VERSION_STRING=$(VERSION) ; \
            [ -e ".git" ] && GITVERSION=$$( git describe --tags --always --dirty --match "[0-9A-Z]*.[0-9A-Z]*" ) && VERSION_STRING=$$GITVERSION ; \
            echo $$VERSION_STRING \
         )
DEFINES += -DVERSION=\"$(VERSION_STR)\"
TARGET_NAME = OCServer
BUILD_DIR = $(CURDIR)
SRC_DIR = src
ODIR = obj
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

ifdef CROSS
  CXXVERSION := $(shell $(CROSS)$(CXX) --version | grep -i gcc | sed 's/^.* //g')
  CXXMACHINE := $(shell $(CROSS)$(CXX) -dumpmachine)
endif

OS_COMPILER := $(CXX)
OS_LINKER := $(CXX)
ifdef CCACHE
  CXX = ccache $(CROSS)$(OS_COMPILER)
  LD  = ccache $(CROSS)$(OS_LINKER)
else
  CXX = $(CROSS)$(OS_COMPILER)
  LD  = $(CROSS)$(OS_LINKER)
endif
STRIP = $(CROSS)strip
RC  = $(CROSS)windres
AR  = $(CROSS)ar

CXXFLAGS += -ICore/src -IExternal/sysroot/include
LDFLAGS += $(PROFILE) -LCore -LExternal/sysroot/lib
# enable optimizations.
ifdef RELEASE
ifeq ($(NATIVE), osx)
	ifeq ($(shell $(CXX) -E -Os - < /dev/null > /dev/null 2>&1 && echo fos),fos)
		OPTLEVEL = -Os
	else
		OPTLEVEL = -O3
	endif
else
	# MXE ICE Workaround
	# known bad on 4.9.3 and 4.9.4, if it gets fixed this could include a version test too
	ifeq ($(CXXMACHINE), x86_64-w64-mingw32.static)
		OPTLEVEL = -O3
	else
		OPTLEVEL = -Os
	endif
endif
ifdef LTO
    ifdef CLANG
      # LLVM's LTO will complain if the optimization level isn't between O0 and
      # O3 (inclusive)
      OPTLEVEL = -O3
    endif
  endif
  CXXFLAGS += $(OPTLEVEL)

  ifdef LTO
    LDFLAGS += -fuse-ld=gold
    ifdef CLANG
      LTOFLAGS += -flto
    else
      LTOFLAGS += -flto=jobserver -flto-odr-type-merging
    endif
  endif
  CXXFLAGS += $(LTOFLAGS)

  # OTHERS += -mmmx -m3dnow -msse -msse2 -msse3 -mfpmath=sse -mtune=native
  # Strip symbols.
  OTHERS += $(RELEASE_FLAGS)
  DEBUG =
  ifndef DEBUG_SYMBOLS
    DEBUGSYMS =
  endif
  DEFINES += -DRELEASE
else
  DEFINES += -DDEBUG
endif
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
ifeq ($(NATIVE),)
  ifeq ($(CROSS),)
    ifeq ($(shell sh -c 'uname -o 2>/dev/null || echo not'),Cygwin)
      DEFINES += -DNO_CPP11_STRING_CONVERSIONS
      TARGETSYSTEM=CYGWIN
    else
      TARGETSYSTEM=LINUX
    endif
  endif
endif
# Linux 64-bit
ifeq ($(NATIVE), linux64)
  CXXFLAGS += -m64
  LDFLAGS += -m64
  TARGETSYSTEM=LINUX
else
  # Linux 32-bit
  ifeq ($(NATIVE), linux32)
    CXXFLAGS += -m32
    LDFLAGS += -m32
    TARGETSYSTEM=LINUX
  endif
endif

ifeq ($(NATIVE), osx)
  ifdef CLANG
    OSX_MIN = 10.7
  else
    OSX_MIN = 10.5
  endif
  DEFINES += -DMACOSX
	#OTHERS += -stdlib=libc++
  #CXXFLAGS += -mmacosx-version-min=$(OSX_MIN)
  #LDFLAGS += -mmacosx-version-min=$(OSX_MIN)
  TARGETSYSTEM=OSX

endif

# Win32 (MinGW32 or MinGW-w64(32bit)?)
ifeq ($(NATIVE), win32)
# Any reason not to use -m32 on MinGW32?
  TARGETSYSTEM=WINDOWS
else
  # Win64 (MinGW-w64? 64bit isn't currently working.)
  ifeq ($(NATIVE), win64)
    CXXFLAGS += -m64
    LDFLAGS += -m64
    TARGETSYSTEM=WINDOWS
  endif
endif

# Cygwin
ifeq ($(NATIVE), cygwin)
  TARGETSYSTEM=CYGWIN
endif

# MXE cross-compile to win32
ifneq (,$(findstring mingw32,$(CROSS)))
  DEFINES += -DCROSS_LINUX
  TARGETSYSTEM=WINDOWS
endif

ifeq ($(TARGETSYSTEM),WINDOWS)
  BACKTRACE = 0
  EXTENSION = .exe
  TARGET := $(TARGET)$(EXTENSION)
  GETR = $(W32ODIR)
  ifdef DYNAMIC_LINKING
    # Windows isn't sold with programming support, these are static to remove MinGW dependency.
    #LDFLAGS += -static-libgcc -static-libstdc++
  else
    LDFLAGS += -static
  endif
  #W32FLAGS += -Wl,-stack,12000000,-subsystem,windows
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
    #LDFLAGS += -ljvm
    #LDFLAGS += -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion
    CORE_LIB := libOCSCore.dll
endif

ifeq ($(BACKTRACE),1)
  DEFINES += -DBACKTRACE
endif

ifeq ($(TARGETSYSTEM), LINUX)
  BACKTRACE=1
  ifneq ($(PREFIX),)
    DEFINES += -DPREFIX="$(PREFIX)"
  endif
  JAVA_HOME:=$(shell echo $$(dirname $$(dirname $$(readlink -f $$(which javac)))))
  JVM_PATH:=$(shell dirname $$(find $(JAVA_HOME) -name "libjvm*"))
  CXXFLAGS += -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
  EXTENSION =
  TARGET := $(TARGET)$(EXTENSION)
  CORE_LIB := libOCSCore.so
endif

ifeq ($(TARGETSYSTEM), OSX)
  BACKTRACE=1
  ifneq ($(PREFIX),)
    DEFINES += -DPREFIX="$(PREFIX)"
  endif
  JAVA_HOME:=$(shell /usr/libexec/java_home)
  JVM_PATH:=$(shell dirname $$(find $(JAVA_HOME) -name "libjvm*"))
  CXXFLAGS += -I/usr/local/opt/openssl/include -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/darwin
  LDFLAGS += -L/usr/local/opt/openssl/lib
  EXTENSION =
  TARGET := $(TARGET)$(EXTENSION)
  CORE_LIB := libOCSCore.dylib
endif

ifeq ($(TARGETSYSTEM), CYGWIN)
  ifneq ($(PREFIX),)
    DEFINES += -DPREFIX="$(PREFIX)"
  endif
  DEFINES += -D_GLIBCXX_USE_C99_MATH_TR1
  EXTENSION = .exe
  TARGET := $(TARGET)$(EXTENSION)
  CORE_LIB := libOCSCore.dll
endif

ifdef MSYS2
  DEFINES += -D_GLIBCXX_USE_C99_MATH_TR1
endif

ifeq ($(BACKTRACE),1)
  DEFINES += -DBACKTRACE
endif

LDFLAGS += -lssl -lcrypto

all: $(TARGET) $(L10N)
	@
$(TARGET): $(ODIR) $(OBJS) Core/$(CORE_LIB)
	+$(LD) $(W32FLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) -lOCSCore -luv
ifdef RELEASE
  ifndef DEBUG_SYMBOLS
	$(STRIP) $(TARGET)
  endif
endif

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(DEFINES) $(CXXFLAGS) -c $< -o $@

export ODIR _OBJS LDFLAGS CXX LD AR STRIP W32FLAGS DEFINES CXXFLAGS VERSION_STR TARGETSYSTEM
clean: clean-Core
	rm -rf *$(TARGET_NAME) *$(TARGET_NAME).a *$(TARGET_NAME).exe
	rm -rf doxygen_doc/html
	rm -rf *obj

astyle:
	$(ASTYLE_BINARY) --options=.astylerc -n $(shell cat astyled_whitelist)

astyle-all: $(SOURCES) $(HEADERS)
	$(ASTYLE_BINARY) --options=.astylerc -n $(SOURCES) $(HEADERS) $(HPPS)

clean-Core:
	$(MAKE) -C Core clean

Core/$(CORE_LIB): Core/src/org_gtdev_oc_server_JNInterface.h
	$(MAKE) -C Core

Core: Core/src/org_gtdev_oc_server_JNInterface.h
	$(MAKE) -C Core

Core/src/org_gtdev_oc_server_JNInterface.h:
	javac -h Core/src Java/src/org/gtdev/oc/server/JNInterface.java

unexport LDFLAGS
unexport CXXFLAGS

external:
	@for dir in External; do make -C $$dir ; echo; done

docs:
	doxygen doxygen_doc/doxygen_conf

.PHONY: clean-Core Core clean astyle astyle-all external docs
