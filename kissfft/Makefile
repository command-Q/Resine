CFLAGS = $(_CFLAGS)
ifeq ($(PRECISION),SINGLE)
	CFLAGS += -Dkiss_fft_scalar=float
else
	CFLAGS += -Dkiss_fft_scalar=double
endif

ifeq ($(THREADED),1)
	CFLAGS += -fopenmp
endif

.PHONY: all clean

all: $(patsubst %.c,%.o,$(wildcard *.c))
	
clean:
	rm -f *.o