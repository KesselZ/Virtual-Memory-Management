main: main.o Process.o Pager.o 
	g++ -std=c++11 main.o Process.o Pager.o -o mmu

main.o: main.cpp Pager.h Pager.cpp Process.h Process.cpp PTE.h
	g++ -std=c++11 -c main.cpp

Process.o: Process.cpp PTE.h Process.h
	g++ -std=c++11 -c Process.cpp

Pager.o: Process.h Process.cpp Pager.cpp Pager.h PTE.h
	g++ -std=c++11 -c Pager.cpp 


clean:
	rm -f mmu *.o