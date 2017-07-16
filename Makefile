CC = g++
LDFLAGS = -lpcap -lz -lpthread
CCFLAGS =

defs = def0.h \
	   def1.h \
	   tools.h \
	   html_content.h

objs = main.o \
	   tools.o \
	   html_content.o

all : run
run : $(objs)
	$(CC) $(objs) -o run $(LDFLAGS)

$(objs) : %.o: %.cc $(defs)
	$(CC) -c $(CCFLAGS) $< -o $@


.PHONY : clean
clean:
	-rm run *.o
