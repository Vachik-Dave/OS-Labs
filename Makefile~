########################################
#
#  IUPUI CSCI 503 -- Operating Systems
#
########################################

## Compiler, tools and options
CC      = gcc
LINK    = gcc
#OPT     = -g

#CCFLAGS = $(OPT) -Wall
#LDFLAGS = $(OPT)

## Libraries

#LIBS =
#INC  = -I.

## FILES
OBJECTS1 = Lab1_2.o
TARGET1  = Lab1_2


## Implicit rules
.SUFFIXES: .c
.c.o:
	$(CC) -c $(CCFLAGS) $(INC) $<

## Build rules
all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)

$(TARGET1): $(OBJECTS1)
	$(LINK) -o $@ $(OBJECTS1) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJECTS1) $(TARGET1)
	rm -f *~ core

