# For the record:
# $@ The name of the target file (the one before the colon)
# $< The name of the first (or only) prerequisite file (the first one after the colon)
# $^ The names of all the prerequisite files (space separated)
# $* The stem (the bit which matches the % wildcard in the rule definition.

# GCC environment variables
# http://gcc.gnu.org/onlinedocs/gcc-4.8.1/gcc/Environment-Variables.html#Environment-Variables



#============================================
# INSTRUCTIONS
#============================================
# 											\
GCC minimum version 4.7 required.			\
											\
It requires Yices 2.6 to be installed.		\
If yices is not install at the default		\
system paths, two environment variables have\
to be set to specify the path to Yices' 	\
shared binaries	both to the compiler and the\
system:										\
											\
>	export LIBRARY_PATH="path/to/lib"		\
>	export LD_LIBRARY_PATH="path/to/lib"	\
											\
and one to the headers:						\
											\
>	export CPATH="path/to/include"			\
											\
To compile run: 							\
											\
>	make 									\
											\
It can also be compiled without enabling	\
Yices as a backend solver, in which 		\
case is not necessary to specify the path	\
to the binaries and headers. Run:			\
											\
>	make YICES=0							\
											\
											\
When calling a backend solver as an external\
binary, some auxilliary files will 			\
be created, and might not be deleted if the	\
execution is aborted. 						\
By default the temp. files are created in	\
the directory where this application runs.	\
You can set a permanent path by setting 	\
the following variable:						\
											\
> 	export TMPFILESPATH="path/to/tmp"		\
#============================================



CC := g++
SHELL := /bin/bash
HOST := $(shell hostname)

SRCROOT := src
RELEASE_BUILDROOT := build/release
DEBUG_BUILDROOT := build/debug
RELEASE_BINROOT := bin/release
DEBUG_BINROOT := bin/debug

DEBUG := 0
YICES := 0
GLUCOSE :=1
MAPLE :=1
CUSTOMYICES := 0
CPOPTIMIZER := 0



DIRECTORIES := 	smtapi/src \
			smtapi/src/util \
			smtapi/src/MDD \
			smtapi/src/encoders \
			smtapi/src/optimizers \
			smtapi/src/controllers \
			smtapi/src/solvers \
		encodings \
			encodings/MRCPSP \
			encodings/PRCPSP \
			encodings/RCPSPMAX \
			encodings/MRCPSPMAX \
			encodings/RCPSPT \
			encodings/MSPSP \
			encodings/MMKP \
			encodings/clinics \
			encodings/test \
			encodings/Auction \
		parser \
		controllers

SOURCES := $(addprefix smtapi/src/util/, \
	util.cpp \
	errors.cpp \
	bipgraph.cpp \
	disjointset.cpp \
	predgraph.cpp \
)

SOURCES += $(addprefix smtapi/src/optimizers/, \
	optimizer.cpp \
	singlecheck.cpp \
	uboptimizer.cpp \
	buoptimizer.cpp \
	dicooptimizer.cpp \
	nativeoptimizer.cpp \
)


SOURCES += $(addprefix smtapi/src/MDD/, \
	mdd.cpp \
	mddbuilder.cpp \
	amopbmddbuilder.cpp \
	amopbbddbuilder.cpp \
)

SOURCES += $(addprefix smtapi/src/encoders/, \
	encoder.cpp \
	apiencoder.cpp \
	fileencoder.cpp \
	dimacsfileencoder.cpp \
	smtlib2fileencoder.cpp \
)

ifeq ($(YICES),1)
SOURCES += $(addprefix smtapi/src/encoders/, \
	yices2apiencoder.cpp \
)
endif

ifeq ($(GLUCOSE),1)
SOURCES += $(addprefix smtapi/src/encoders/, \
	glucoseapiencoder.cpp)

DIRECTORIES += $(addprefix smtapi/src/solvers/glucose/, \
	core \
	simp \
	utils \
)

SOURCES += $(addprefix smtapi/src/solvers/glucose/, \
	simp/SimpSolver.cc \
	utils/Options.cc \
	utils/System.cc \
	core/Solver.cc \
)

endif

ifeq ($(MAPLE),1)
SOURCES += $(addprefix smtapi/src/encoders/, \
	minisatapiencoder.cpp)

DIRECTORIES += $(addprefix smtapi/src/solvers/maple/, \
	core \
	simp \
	utils \
)

SOURCES += $(addprefix smtapi/src/solvers/maple/, \
	simp/SimpSolver.cc \
	utils/Options.cc \
	utils/System.cc \
	core/Solver.cc \
)

endif


SOURCES += $(addprefix smtapi/src/controllers/, \
 	solvingarguments.cpp \
 	basiccontroller.cpp \
 	arguments.cpp \
)

SOURCES += $(addprefix smtapi/src/, \
	smtapi.cpp \
	smtformula.cpp \
	encoding.cpp \
)

SOURCES += $(addprefix encodings/MRCPSP/,\
	mrcpsp.cpp \
	mrcpspencoding.cpp \
	smttimeencoding.cpp \
	smttaskencoding.cpp \
	omtsatpbencoding.cpp \
	omtsoftpbencoding.cpp \
	mrcpspsatencoding.cpp \
	rcpspencoding.cc \
	order.cpp \
	doubleorder.cpp \
)

SOURCES += $(addprefix encodings/PRCPSP/,\
	prcpsp.cpp \
	prcpspencoding.cpp \
	prcpspsatencoding.cpp \
)

SOURCES += $(addprefix encodings/RCPSPMAX/,\
	rcpspmax.cpp \
	rcpspmaxencoding.cpp \
)

SOURCES += $(addprefix encodings/MRCPSPMAX/,\
	mrcpspmax.cpp \
	mrcpspmaxrelaxedencoding.cpp \
	mrcpspmaxencoding.cpp \
)

SOURCES += $(addprefix encodings/RCPSPT/,\
	rcpspt.cpp \
	rcpsptencoding.cpp \
	rcpspttimeencoding.cpp \
	rcpspttaskencoding.cpp \
)

SOURCES += $(addprefix encodings/MSPSP/,\
	nbdd.cpp \
	mspsp.cpp \
	mspspencoding.cpp \
	mspsptimeencoding.cpp \
	mspsptimegerarquicencoding.cpp \
	mspsptimecombinationsencoding.cpp \
	mspsptimecombinationsencoding2.cpp \
	mspsptaskcombinationsencoding2.cpp \
	mspspnbddtimeencoding.cpp \
)

ifeq ($(CPOPTIMIZER),1)
SOURCES += $(addprefix encodings/MSPSP/,\
 	cpoptmspspencoder.cpp \
)
endif

SOURCES += $(addprefix encodings/MMKP/,\
 	mmkp.cpp \
 	mmkpencoding.cpp \
)

SOURCES += $(addprefix encodings/Auction/,\
	auction.cpp \
	auctionencoding.cpp \
)

SOURCES += $(addprefix encodings/clinics/,\
 	clinics.cpp \
 	clinicstest.cpp \
 	clinicsencoding.cpp \
	clinicstimeencoding.cpp \
	clinicstaskencoding.cpp \
)

SOURCES += $(addprefix encodings/test/,\
	testencoding.cpp \
)

SOURCES += $(addprefix parser/, \
 	parser.cpp \
)


# ----------------------------------------------------
# GCC Compiler flags
# ----------------------------------------------------
CFLAGS := -w -std=c++11 -Wall -Wextra

ifeq ($(DEBUG),1)
CFLAGS+= -g -O0 -fbuiltin -fstack-protector-all
else
CFLAGS+= -O3
endif

ifeq ($(DEBUG),0)
DEFS+= -DNDEBUG
endif

ifeq ($(CPOPTIMIZER),1)
CFLAGS+= -fPIC -fstrict-aliasing -pedantic -fexceptions -frounding-math -Wno-long-long -m64
DEFS+= -DCPOPTIMIZER -DIL_STD -DILOUSEMT -D_REENTRANT -DILM_REENTRANT
LFLAGS+= -lcp -lcplex -lconcert -lpthread -lm -ldl
endif

ifeq ($(CUSTOMYICES),1)
DEFS+= "-DCUSTOMYICES"
endif

ifeq ($(YICES),1)
DEFS+= "-DUSEYICES"
endif

ifeq ($(GLUCOSE),1)
DEFS+= "-DUSEGLUCOSE"
endif

ifeq ($(MAPLE),1)
DEFS+= "-DUSEMINISAT"
endif

ifneq ($(TMPFILESPATH),"")
DEFS+= "-DTMPFILESPATH=\"$(TMPFILESPATH)\""
endif


ifeq ($(DEBUG),1)
BUILDROOT := $(DEBUG_BUILDROOT)
BINROOT := $(DEBUG_BINROOT)
else
BUILDROOT := $(RELEASE_BUILDROOT)
BINROOT := $(RELEASE_BINROOT)
endif



# -----------------------------------------------------
# Links
# -----------------------------------------------------
LFLAGS += -L./$(BUILDROOT)
ifeq ($(YICES),1)
LFLAGS += -lyices
endif
LFLAGS += -lpython3.10


# -----------------------------------------------------
# Include directories
# -----------------------------------------------------
INCLUDES += -I./$(SRCROOT)
INCLUDES += $(addprefix -I./$(SRCROOT)/,$(DIRECTORIES))





OBJS := $(SOURCES:%.cpp=$(BUILDROOT)/%.o)
OBJS := $(OBJS:%.cc=$(BUILDROOT)/%.o)
SOURCES := $(addprefix src/, $(SOURCES))




.PHONY: all mrcpsp2smt mmkp2smt rcpspt2smt rcpspmax2smt mrcpspmax2smt mspsp2smt auction2smt checkmrcpsp checkprcpsp checkmspsp checkrcpspmax checkmrcpspmax checkmmkp checkrcpspt test clinics2smt

.SECONDARY: $(OBJS)


all: mrcpsp2smt mmkp2smt rcpspt2smt rcpspmax2smt mrcpspmax2smt mspsp2smt auction2smt checkmrcpsp checkprcpsp checkmspsp checkrcpspmax checkmrcpspmax checkmmkp checkrcpspt test clinics2smt

clean:
	@rm -rf build
	@rm -rf bin

mrcpsp2smt: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/mrcpsp2smt.o $(BINROOT)/mrcpsp2smt

rcpspmax2smt:  $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/rcpspmax2smt.o $(BINROOT)/rcpspmax2smt

mrcpspmax2smt:  $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/mrcpspmax2smt.o $(BINROOT)/mrcpspmax2smt

rcpspt2smt: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/rcpspt2smt.o $(BINROOT)/rcpspt2smt

mspsp2smt:  $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/mspsp2smt.o $(BINROOT)/mspsp2smt

mmkp2smt: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/mmkp2smt.o $(BINROOT)/mmkp2smt

prcpsp: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/prcpsp2dimacs.o $(BINROOT)/prcpsp2dimacs


auction2smt: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/auction2smt.o $(BINROOT)/auction2smt

checkmrcpsp: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/checkmrcpsp.o $(BINROOT)/checkmrcpsp

checkprcpsp: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/checkprcpsp.o $(BINROOT)/checkprcpsp

checkrcpspmax: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/checkrcpspmax.o $(BINROOT)/checkrcpspmax

checkmrcpspmax: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/checkmrcpspmax.o $(BINROOT)/checkmrcpspmax

checkrcpspt: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/checkrcpspt.o $(BINROOT)/checkrcpspt

checkmspsp: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/checkmspsp.o $(BINROOT)/checkmspsp

checkmmkp: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/checkmmkp.o $(BINROOT)/checkmmkp


test: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/test.o $(BINROOT)/test

clinics2smt: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/clinics2smt.o $(BINROOT)/clinics2smt


# Compile the binary by calling the compiler with cflags, lflags, and any libs (if defined) and the list of objects.
$(BINROOT)/%: $(OBJS) $(BUILDROOT)/%.o
	@printf "Linking $@ ... "
	@$(CC) $(OBJS) $(@:$(BINROOT)/%=$(BUILDROOT)/%.o) $(CFLAGS) $(INCLUDES) $(LFLAGS) -o $@
	@echo "DONE"


# Get a .o from a .cpp by calling compiler with cflags and includes (if defined)
$(BUILDROOT)/%.o: $(SRCROOT)/%.cpp
	@printf "Compiling $< ... "
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -c $< -o $@
	@echo "DONE"

$(BUILDROOT)/%.o: $(SRCROOT)/%.cc
	@printf "Compiling $< ... "
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -c $< -o $@
	@echo "DONE"

$(BUILDROOT):
	@mkdir -p $@


$(BINROOT):
	@mkdir -p $@


$(addprefix $(BUILDROOT)/, $(DIRECTORIES)): % :
	@mkdir -p $@

