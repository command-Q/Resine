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
_CFLAGS = -Os -I$(incl_includedir) $(DFLAGS)
DBGFLAGS = -O0 -g -Wall
_LDFLAGS = -lm
LDPROJ = -L. -l$(PROJECT)
EXELDFLAGS = -L$(incl_libdir) -lpng -ljpeg

ifeq ($(HAS_FFTW),1)
	_LDFLAGS += -L$(incl_libdir)
	ifeq ($(PRECISION),SINGLE)
		_LDFLAGS += -lfftw3f 
	else ifeq ($(PRECISION),QUAD)
		_LDFLAGS += -lfftw3l
	else 
		_LDFLAGS += -lfftw3
	endif
	ifeq ($(THREADED),1)
		_LDFLAGS += -lfftw3_threads
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
else ifeq ($(SYS),MINGW)
	DYLEXT = dll
	SOFLAGS = -Wl,--out-implib,lib$(PROJECT).$(DYLEXT).a -Wl,--enable-auto-image-base
	DYLIB = lib$(PROJECT)-$(VER).$(DYLEXT)
	EXEEXT = .exe
else #linux
	DYLEXT = so
	DYLIB = lib$(PROJECT).$(DYLEXT).$(VER)
	SOFLAGS = -shared -Wl,-soname,lib$(PROJECT).$(DYLEXT).$(firstword $(subst ., ,$(VER)))
endif

ifeq ($(CC),clang)
	DBGFLAGS += -std=c99
	_CFLAGS += -std=c99
else
	DBGFLAGS += -std=gnu99
	_CFLAGS += -std=gnu99 -ffast-math
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
EXECUTABLE = $(PROJECT)$(EXEEXT)

all: lib exe
lib: static dynamic

static: $(SRCS) $(OBJS)
	$(AR) rcs $(LIB) $(OBJS)

dynamic: $(SRCS) $(OBJS)
	$(CC) $(LDFLAGS) $(SOFLAGS) -o $(DYLIB) $(OBJS)
	ln -fs $(DYLIB) $(DYLN)

exe: $(SRCSEXE)
	$(CC) $(_CFLAGS) $(LDPROJ) $(EXELDFLAGS) -o $(EXECUTABLE) $+
		
debug: 
	$(CC) $(DBGFLAGS) -I$(incl_includedir) $(DFLAGS) $(LDFLAGS) $(SOFLAGS) -o $(DYLIB) $(SRCS)
	ln -fs $(DYLIB) $(DYLN)
	$(CC) $(DBGFLAGS) -I$(incl_includedir) $(DFLAGS) $(LDFLAGS) $(EXELDFLAGS) -o $(EXECUTABLE) $(SRCS) $(SRCSEXE)

$(SRCS):
	$(CC) -c $(_CFLAGS)

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