## BUILD CONFIGURATION ##
HAS_FFTW = 1
# Valid options are SINGLE, DOUBLE, or QUAD
PRECISION = DOUBLE
THREADED = 0

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
VER = 0.9.3

## BUILD FLAGS ##
DFLAGS = -DHAS_FFTW=$(HAS_FFTW) -DRSN_PRECISION=$(PRECISION) -DTHREADED=$(THREADED) -DSKIP_CONFIG
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

all: lib exe
lib: static dynamic

debug: _CFLAGS = -O0 -g -Wall -I$(incl_includedir) $(CFLAGS)
debug: all

resine_config.h:
	@echo "#define RSN_PRECISION $(RSN_PRECISION)" > resine_config.h
	@echo "#define HAS_FFTW $(HAS_FFTW)" >> resine_config.h

.c.o:
	$(CC) -c $(_CFLAGS) $(DFLAGS) $< -o $@

$(LIB): resine_config.h $(OBJS)
	$(AR) rcs $(LIB) $(OBJS)
static: $(LIB)

$(DYLIB): resine_config.h $(OBJS)
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
clean: tidy
	rm -f $(LIB) $(DYLIB) $(DYLN) $(EXECUTABLE) resine_config.h