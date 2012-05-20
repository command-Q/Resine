## BUILD CONFIGURATION ##
HAS_FFTW = 1
# Valid options are SINGLE, DOUBLE, or QUAD
PRECISION = DOUBLE
THREADED = 0

## SYSTEM SETTINGS ##
ARCH = X86
SYS = MACOSX
RANLIB ?= ranlib
STRIP ?= strip
INSTALL ?= install

## PATHS ##
# Install destination
prefix = /usr/local
bindir = ${prefix}/bin
libdir = ${prefix}/lib
includedir = ${prefix}/include
# Dependencies
incl_prefix = /opt/local
incl_libdir = ${incl_prefix}/lib
incl_includedir = ${incl_prefix}/include

###################################
## DO NOT MODIFY BELOW THIS LINE ##
###################################

## PROJECT DEFINITION ##
PROJECT = resine
VER = 0.9.2

## BUILD FLAGS ##
DFLAGS = -DHAS_FFTW=$(HAS_FFTW) -DPRECISION=$(PRECISION) -DTHREADED=$(THREADED)
_CFLAGS = -Os -I$(incl_includedir)
_LDFLAGS = -lm
LDPROJ = -L. -l$(PROJECT)
EXELDFLAGS = -L$(incl_libdir) -lpng -ljpeg

ifeq ($(HAS_FFTW),1)
	_LDFLAGS += -L$(incl_libdir)
	ifeq ($(PRECISION),SINGLE)
		_LDFLAGS += -lfftw3f 
		STATICLDPROJ = $(incl_libdir)/libfftw3f.a 
	else ifeq ($(PRECISION),QUAD)
		_LDFLAGS += -lfftw3l
		STATICLDPROJ = $(incl_libdir)/libfftw3l.a 
	else 
		_LDFLAGS += -lfftw3
		STATICLDPROJ = $(incl_libdir)/libfftw3.a 
	endif
	ifeq ($(THREADED),1)
		_LDFLAGS += -lfftw3_threads
		STATICLDPROJ += $(incl_libdir)/libfftw3_threads.a 
	endif
endif
ifeq ($(THREADED),1)
	_LDFLAGS += -lpthread
endif

ifeq ($(ARCH),X86_64)
	_LDFLAGS += -fPIC
	_CFLAGS += -fPIC
endif

ifeq ($(SYS),MACOSX)
	DYLEXT = dylib
	DYLIB = lib$(PROJECT).$(VER).$(DYLEXT)
	SOFLAGS = -dynamiclib -Wl,-install_name,@executable_path/$(DYLIB),-headerpad_max_install_names,-compatibility_version,$(firstword $(subst ., ,$(VER))),-current_version,$(VER)
	STATICLDPROJ += ./libresine.a
	STATICEXELDFLAGS = $(incl_libdir)/libpng.a $(incl_libdir)/libz.a $(incl_libdir)/libjpeg.a 
else 
	ifeq ($(SYS),MINGW)
		DYLEXT = dll
		SOFLAGS = -Wl,--out-implib,lib$(PROJECT).$(DYLEXT).a -Wl,--enable-auto-image-base
		DYLIB = lib$(PROJECT)-$(VER).$(DYLEXT)
		EXEEXT = .exe
	else #linux
		DYLEXT = so
		DYLIB = lib$(PROJECT).$(DYLEXT).$(VER)
		SOFLAGS = -shared -Wl,-soname,lib$(PROJECT).$(DYLEXT).$(firstword $(subst ., ,$(VER)))
	endif
	STATICLDPROJ = -static $(LDPROJ) $(_LDFLAGS)
	STATICEXELDFLAGS = -static $(EXELDFLAGS) -lz
endif

ifneq (,$(filter gcc%,$(CC)))
	_CFLAGS += -ffast-math -std=gnu99
endif

_CFLAGS += $(CFLAGS)
LDFLAGS := $(_LDFLAGS) $(LDFLAGS)
EXELDFLAGS := $(EXELDFLAGS) $(LDFLAGS)

SRCS = lib/util.c lib/dsp.c lib/resine.c
HEADERS = lib/common.h $(SRCS:%.c=%.h)
OBJS = $(SRCS:%.c=%.o)
LIB = lib$(PROJECT).a
DYLN = lib$(PROJECT).$(DYLEXT)

SRCSEXE = image.c main.c
EXEOBJS = $(SRCSEXE:%.c=%.o)
EXECUTABLE = $(PROJECT)$(EXEEXT)

.PHONY: all lib static dynamic exe exe-static debug archive install uninstall tidy clean

all: lib exe
lib: static dynamic

debug: _CFLAGS = -O0 -g -Wall -I$(incl_includedir) $(CFLAGS)
debug: all

.c.o:
	$(CC) -c $(_CFLAGS) $(DFLAGS) $< -o $@

$(LIB): $(OBJS)
	$(AR) rcs $(LIB) $(OBJS)
static: $(LIB)

$(DYLIB): $(OBJS)
	$(CC) $(LDFLAGS) $(SOFLAGS) -o $(DYLIB) $(OBJS)
	ln -fs $(DYLIB) $(DYLN)
dynamic: $(DYLIB)

$(EXECUTABLE): $(EXEOBJS)
	$(CC) $(LDPROJ) $(EXELDFLAGS) -o $(EXECUTABLE) $(EXEOBJS)
exe: $(DYLIB) $(EXECUTABLE)
exe-static: LDPROJ = $(STATICLDPROJ)
exe-static: EXELDFLAGS = $(STATICEXELDFLAGS)
exe-static: $(LIB) $(EXECUTABLE)

archive:
	rm -f $(PROJECT).zip
	zip -q $(PROJECT).zip $(HEADERS) $(SRCS) $(SRCSEXE:%.c=%.h) $(SRCSEXE)

install: all
	$(INSTALL) $(LIB) $(libdir)
	$(INSTALL) $(DYLIB) $(libdir)
	ln -fs $(libdir)/$(DYLIB) $(libdir)/$(DYLN)
	$(INSTALL) -d $(includedir)/$(PROJECT)
	$(INSTALL) $(HEADERS) $(includedir)/$(PROJECT)
	$(INSTALL) $(EXECUTABLE) $(bindir)
ifeq ($(SYS),MACOSX)
	install_name_tool -id $(libdir)/$(DYLIB) $(libdir)/$(DYLIB)
	install_name_tool -change @executable_path/$(DYLIB) $(libdir)/$(DYLIB) $(bindir)/$(EXECUTABLE)
endif

uninstall:
	rm $(libdir)/$(LIB) $(libdir)/$(DYLIB) $(libdir)/$(DYLN)
	rm -r $(includedir)/$(PROJECT)
	rm $(bindir)/$(EXECUTABLE)

tidy:
	rm -f $(OBJS)
clean: tidy
	rm -f $(LIB) $(DYLIB) $(DYLN) $(EXECUTABLE)