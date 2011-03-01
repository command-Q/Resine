## BUILD CONFIGURATION ##
HAS_FFTW = 1
# Valid options are SINGLE, DOUBLE, or QUAD
PRECISION = DOUBLE
THREADED = 0

## SYSTEM SETTINGS ##
ARCH = X86
SYS = MACOSX
CC = clang
AR = ar
RANLIB = ranlib
STRIP = strip
INSTALL = install

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

## BUILD FLAGS ##
DFLAGS = -DHAS_FFTW=$(HAS_FFTW) -DPRECISION=$(PRECISION) -DTHREADED=$(THREADED)
CFLAGS = -std=c99 -Wall -Os -I$(incl_includedir) $(DFLAGS)
LDFLAGS = -lm
LDPROJ = -L. -l$(PROJECT)
EXELDFLAGS = -L$(incl_libdir) -lpng -ljpeg

###################################
## DO NOT MODIFY BELOW THIS LINE ##
###################################

## PROJECT DEFINITION ##
PROJECT = resine
VER = 0.9.2

ifeq ($(HAS_FFTW),1)
	LDFLAGS += -L$(incl_libdir)
	ifeq ($(PRECISION),SINGLE)
		LDFLAGS += -lfftw3f 
	else ifeq ($(PRECISION),QUAD)
		LDFLAGS += -lfftw3l
	else 
		LDFLAGS += -lfftw3
	endif
	ifeq ($(THREADED),1)
		LDFLAGS += -lfftw3_threads
	endif
endif
ifeq ($(THREADED),1)
	LDFLAGS += -lpthread
endif

ifeq ($(ARCH),X86_64)
	LDFLAGS += -fPIC -march=core2
	CFLAGS += -fPIC -march=core2
else ifeq ($(ARCH),X86)
	CFLAGS += -march=i686
endif

ifeq ($(SYS),MACOSX)
	DYLEXT = dylib
	DYLIB = lib$(PROJECT).$(VER).$(DYLEXT)
	SOFLAGS = -dynamiclib -Wl,-install_name,@executable_path/$(DYLIB),-compatibility_version,$(firstword $(subst ., ,$(VER))),-current_version,$(VER)
else ifeq ($(SYS),MINGW)
	DYLEXT = dll
	SOFLAGS = -Wl,--out-implib,lib$(PROJECT).$(DYLEXT).a -Wl,--enable-auto-image-base
	DYLIB = lib$(PROJECT)-$(VER).$(DYLEXT)
	EXEEXT = .exe
else #linux
	DYLEXT = so
	DYLIB = lib$(PROJECT).$(DYLEXT).$(VER)
	SOFLAGS = -shared -Wl,-soname,$(DYLIB)
endif

ifeq ($(CC),gcc)
	CFLAGS += -ffast-math
endif

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
	$(CC) $(CFLAGS) $(LDPROJ) $(EXELDFLAGS) -o $(EXECUTABLE) $+
		
debug: 
	$(CC) -std=c99 -O0 -g -Wall -I$(incl_includedir) $(DFLAGS) $(LDFLAGS) $(SOFLAGS) -o $(DYLIB) $(SRCS)
	ln -fs $(DYLIB) $(DYLN)
	$(CC) -std=c99 -O0 -g -Wall -I$(incl_includedir) $(DFLAGS) $(LDFLAGS) $(EXELDFLAGS) -o $(EXECUTABLE) $(SRCS) $(SRCSEXE)

$(SRCS):
	$(CC) -c $(CFLAGS)

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

uninstall:
	rm $(libdir)/$(LIB) $(libdir)/$(DYLIB) $(libdir)/$(DYLN)
	rm -r $(includedir)/$(PROJECT)
	rm $(bindir)/$(EXECUTABLE)

tidy:
	rm -f $(OBJS)

clean: tidy
	rm -f $(LIB) $(DYLIB) $(DYLN) $(EXECUTABLE)