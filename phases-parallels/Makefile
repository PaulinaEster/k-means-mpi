SHELL=/bin/sh
CCOMPILER=mpicxx
CFLAGS = -Wall -O3 -mcmodel=large -lm
# WORKLOAD
WORKLOAD=A

# DEBUG FLAG
DEBUG=OFF
DEBUG_FLAG=NO_DEBUG
ifeq ($(DEBUG),ON)
	DEBUG_FLAG=DEBUG
endif

# TIMER FLAG
TIMER=OFF
TIMER_FLAG=NO_TIMER
ifeq ($(TIMER),ON)
	TIMER_FLAG=TIMER
endif

# include ../config/make.def

all: data_generator k_means

data_generator:
	$(CCOMPILER) data_generator.c $(CFLAGS) -DWORKLOAD_$(WORKLOAD) -o data_generator.$(WORKLOAD).exe

k_means:
	$(CCOMPILER) k_means.cpp $(CFLAGS) -DWORKLOAD_$(WORKLOAD) -D$(DEBUG_FLAG) -D$(TIMER_FLAG) -o k_means.$(WORKLOAD).exe

clean:
	- rm -f *.o *~ data_generator.*.exe k_means.*.exe
