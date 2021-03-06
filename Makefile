## BUILD CONFIGURATION ##
HAS_FFTW ?= 1
HAS_KISS ?= 1
PRECISION ?= DOUBLE
# Valid options are SINGLE, DOUBLE, LONG, or QUAD (requires libquadmath)
THREADED ?= 0

## SYSTEM SETTINGS ##
ARCH = X86
SYS = MACOSX
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
VER = 0.9.4

## BUILD FLAGS ##
DFLAGS = -DHAS_FFTW=$(HAS_FFTW) -DHAS_KISS=$(HAS_KISS) -DRSN_PRECISION=$(PRECISION) -DRSN_IS_THREADED=$(THREADED) -DSKIP_CONFIG
_CFLAGS = -Os -I$(incl_includedir)
_LDFLAGS = -lm
LDPROJ = -L. -l$(PROJECT)
EXELDFLAGS = -L$(incl_libdir) -lpng -ljpeg

ifeq ($(PRECISION),QUAD)
	_LDFLAGS += -L$(incl_libdir) -lquadmath
	STATICLDPROJ = $(incl_libdir)/libquadmath.a
endif

ifeq ($(HAS_KISS),1)
	KISS = $(patsubst %.c,%.o,$(wildcard kissfft/*.c))
endif
ifeq ($(HAS_FFTW),1)
	ifeq ($(PRECISION),SINGLE)
		PSUF=f
	else ifeq ($(PRECISION),LONG)
		PSUF=l
	else ifeq ($(PRECISION),QUAD)
		PSUF=q
	endif
	_LDFLAGS += -L$(incl_libdir) -lfftw3$(PSUF)
	STATICLDPROJ = $(incl_libdir)/libfftw3$(PSUF).a
	ifeq ($(THREADED),1)
		_LDFLAGS += -lfftw3$(PSUF)_threads
		STATICLDPROJ += $(incl_libdir)/libfftw3$(PSUF)_threads.a
	endif
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
		SOFLAGS = -shared -Wl,--out-implib,lib$(PROJECT).$(DYLEXT).a -Wl,--enable-auto-image-base
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
else
	_CFLAGS += -std=c99
endif

_CFLAGS += $(CFLAGS) -I./lib -I./kissfft
LDFLAGS := $(_LDFLAGS) $(LDFLAGS)
EXELDFLAGS := $(EXELDFLAGS) $(LDFLAGS)

SRCS = lib/util.c lib/dsp.c lib/core.c
HEADERS = lib/resine.h
PRIV_HEADERS = lib/dsp.h lib/fftwapi.h
OBJS = $(SRCS:%.c=%.o)
LIB = lib$(PROJECT).a
DYLN = lib$(PROJECT).$(DYLEXT)

SRCSEXE = image.c resine.c
EXEOBJS = $(SRCSEXE:%.c=%.o)
EXECUTABLE = $(PROJECT)$(EXEEXT)

.PHONY: all lib static dynamic exe exe-static debug archive install uninstall tidy clean
.EXPORT_ALL_VARIABLES: $(KISS)

all: lib exe
lib: static dynamic

debug: _CFLAGS = -O0 -g -Wall -I$(incl_includedir) $(CFLAGS) -I./lib -I./kissfft
debug: all

resine_config.h:
	@echo "#define RSN_PRECISION $(PRECISION)" > resine_config.h
	@echo "#define HAS_FFTW $(HAS_FFTW)" >> resine_config.h
	@echo "#define HAS_KISS $(HAS_KISS)" >> resine_config.h
	@echo "#define RSN_IS_THREADED $(THREADED)" >> resine_config.h

.c.o:
	$(CC) -c $(_CFLAGS) $(DFLAGS) $< -o $@

$(KISS):
	$(MAKE) -C kissfft

$(LIB): resine_config.h $(OBJS) $(KISS)
	$(AR) rcs $(LIB) $(OBJS) $(KISS)
static: $(LIB)

$(DYLIB): resine_config.h $(OBJS) $(KISS)
	$(CC) $(LDFLAGS) $(SOFLAGS) -o $(DYLIB) $(OBJS) $(KISS)
	ln -fs $(DYLIB) $(DYLN)
dynamic: $(DYLIB)

$(EXECUTABLE): $(EXEOBJS)
	$(CC) $(LDPROJ) $(EXELDFLAGS) -o $(EXECUTABLE) $(EXEOBJS)
exe: $(DYLIB) $(EXECUTABLE)
exe-static: LDPROJ := $(STATICLDPROJ)
exe-static: EXELDFLAGS := $(STATICEXELDFLAGS)
exe-static: $(LIB) $(EXECUTABLE)

archive:
	rm -f $(PROJECT).zip
	zip -q $(PROJECT).zip $(HEADERS) $(PRIV_HEADERS) $(SRCS) $(SRCSEXE:%.c=%.h) $(SRCSEXE)

install: all
	$(INSTALL) $(LIB) $(libdir)
	$(INSTALL) $(DYLIB) $(libdir)
	ln -fs $(libdir)/$(DYLIB) $(libdir)/$(DYLN)
	$(INSTALL) $(HEADERS) resine_config.h $(includedir)
	$(INSTALL) -s $(EXECUTABLE) $(bindir)
ifeq ($(SYS),MACOSX)
	install_name_tool -id $(libdir)/$(DYLIB) $(libdir)/$(DYLIB)
	install_name_tool -change @executable_path/$(DYLIB) $(libdir)/$(DYLIB) $(bindir)/$(EXECUTABLE)
endif

uninstall:
	rm $(libdir)/$(LIB) $(libdir)/$(DYLIB) $(libdir)/$(DYLN)
	rm $(includedir)/$(HEADERS) $(includedir)/resine_config.h
	rm $(bindir)/$(EXECUTABLE)

tidy:
	rm -f $(OBJS) $(EXEOBJS)
ifeq ($(HAS_KISS),1)
	$(MAKE) -C kissfft clean
endif
clean: tidy
	rm -f $(LIB) $(DYLIB) $(DYLN) $(EXECUTABLE) resine_config.h
