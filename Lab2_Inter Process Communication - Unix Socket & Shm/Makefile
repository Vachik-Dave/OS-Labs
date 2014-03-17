########################################
#
#  Operating Systems
#
########################################

## Compiler, tools and options
CC      = gcc
LINK    = gcc


## FILES
OBJECTS1 = server.o
TARGET1  = server
OBJECTS2 = client.o
TARGET2  = client
OBJECTS3 = application.o
TARGET3  = app
OBJECTS4 = driver.o
TARGET4  = dri


## Implicit rules
.SUFFIXES: .c
.c.o:
	$(CC) -c $(CCFLAGS) $(INC) $<

## Build rules
all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)

$(TARGET1): $(OBJECTS1)
	$(LINK) -o $@ $(OBJECTS1) $(LDFLAGS) $(LIBS)

$(TARGET2): $(OBJECTS2)
	$(LINK) -o $@ $(OBJECTS2) $(LDFLAGS) $(LIBS)

$(TARGET3): $(OBJECTS3)
	$(LINK) -o $@ $(OBJECTS3) $(LDFLAGS) $(LIBS)

$(TARGET4): $(OBJECTS4)
	$(LINK) -o $@ $(OBJECTS4) $(LDFLAGS) $(LIBS)


clean:
	rm -f $(OBJECTS1) $(TARGET1) $(OBJECTS2) $(TARGET2) \
              $(OBJECTS3) $(TARGET3) $(OBJECTS4) $(TARGET4)
	rm -f *~ core

