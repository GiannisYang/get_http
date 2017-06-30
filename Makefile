defs = def0.h def1.h tools.h
obj = main.o tools.o

all : run
run : $(obj)
	g++ $(obj) -o run -lpcap

main.o : main.cc $(defs)
	g++ -c main.cc -lpcap

tools.o : main.cc $(defs)
	g++ -c tools.cc



clean:
	rm run *.o
