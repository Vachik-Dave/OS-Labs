########################################
#
#  Operating Systems
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
OBJECTS1 = PC.o
TARGET1  = PC
OBJECTS2 = MatrixMul.o
TARGET2  = MM



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

