include makefile.in

INCLUDES = -I/global/homes/a/aerdem/sparsehash-master/src # REPLACE THIS WITH YOUR PATH TO SPARSEHASH library

TARGET     = binucleus

SRCS	= $(CFILES:%=%.c) $(CXXFILES:%=%.cpp) 
  	
binucleus: main.o hierarchy.o util.o bucket.o graph.o tip.o wing.o kcore.o ktruss.o
	$(LD) $(LDFLAGS) -o ../$@ main.o hierarchy.o util.o bucket.o graph.o tip.o wing.o kcore.o ktruss.o $(LIBS)
	/bin/rm *.o
  	
clean: 
	/bin/rm -f ../$(TARGET) *.o 
