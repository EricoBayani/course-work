# This makefile was modified by Erico Bayani as a template derived from Wesley Mackey's
# given makefiles.

# this makefile was adapted to work only with C, and thus gcc, not gpp

MKFILE     = Makefile
# DEPFILE    = ${MKFILE}.dep
NOINCL     = ci clean spotless
# NEEDINCL   = ${filter ${NOINCL}, ${MAKECMDGOALS}}
GMAKE      = ${MAKE} --no-print-directory

GCCWARN    = -Wall -Wextra -Wpedantic -Wshadow -O2
GCCOPTS    = ${GCCWARN} -fdiagnostics-color=always
COMPILEC     = gcc -g  ${GCCOPTS}
# MAKEDEPCPP = gcc -std=gnu++17 -MM ${GCCOPTS}
# LINKOPTS   =
#
# probably want to add my own bin some time, maybe add a linting script
# UTILBIN    = 

MODULES    = dog
CSOURCE    = $(wildcard ${MODULES:=.c})
# GENFILES   = colors.cppgen
MODFILES   = ${foreach MOD, ${MODULES}, ${MOD}.h}
SOURCES    = ${wildcard ${MODFILES}}
# OTHERS     = mk-colors.perl ${MKFILE} ${DEPFILE}
ALLSOURCES = ${SOURCES} ${OTHERS}
EXECBIN    = dog
OBJECTS    = ${CSOURCE:.c=.o}
# LINKLIBS   = -lGL -lGLU -lglut -ldrm -lm -lX11
# LISTING     = Listing.ps

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${COMPILEC} ${LINKOPTS} -o $@ ${OBJECTS} ${LINKLIBS}

%.o : %.cpp
	${COMPILEC} -c $<

ci : ${ALLSOURCES}

clean :
	- rm -f ${OBJECTS} ${DEPFILE} core ${GENFILES}

spotless : clean
	- rm -f ${EXECBIN}

${DEPFILE} :
	@ touch ${DEPFILE}
	${GMAKE} dep

again :
	${GMAKE} spotless all

