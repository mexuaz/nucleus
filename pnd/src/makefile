include makefile.in

INCLUDES = -I/global/homes/a/aerdem/sparsehash-master/src # REPLACE THIS WITH YOUR PATH TO SPARSEHASH library

TARGET = pnd 

SRCS	= $(CFILES:%=%.c) $(CXXFILES:%=%.cpp)

all: $(TARGET) 

pnd: main.o local12.o local23.o local34.o bucket.o graph.o
	mkdir -p ../../bin/pnd
	$(LD) $(LDFLAGS) -o ../../bin/pnd/$@ main.o local12.o local23.o local34.o bucket.o graph.o $(LIBS)
	/bin/rm *.o  

clean: 
	/bin/rm -f *.o ../../bin/pnd/$(TARGET) *~ $(TARGET) 

depend: $(SRCS)
	makedepend -Y -- $(INCLUDES) -- $(SRCS) 


#********************************************************************
# Dependencies
#********************************************************************
# DO NOT DELETE THIS LINE -- make depend depends on it.
