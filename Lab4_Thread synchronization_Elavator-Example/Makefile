########################################
#
#  IUPUI CSCI 503 -- Operating Systems
#
########################################

## Compiler, tools and options
CC      = gcc
LINK    = gcc
OPT     = -g


## Libraries
LIBS = -lpthread
INC  = -I.

## FILES
OBJECTS1 = Elavator.o
TARGET1  = ele
OBJECTS2 = Improved_Elavator.o
TARGET2  = imp_ele



## Implicit rules
.SUFFIXES: .c
.c.o:
	$(CC) -c $(CCFLAGS) $(INC) $<

## Build rules
all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJECTS1)
	$(LINK) -o $@ $(OBJECTS1) $(LDFLAGS) $(LIBS)

$(TARGET2): $(OBJECTS2)
	$(LINK) -o $@ $(OBJECTS2) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(OBJECTS1) $(TARGET1) $(OBJECTS2) $(TARGET2) $(OBJECTS3) $(TARGET3)
	rm -f *~ core

