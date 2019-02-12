exe: main.o CDR_List.o Key_List.o heap.o
	g++ -o exe main.o CDR_List.o Key_List.o heap.o
	rm *.o

main.o: main.cc
	g++ -c main.cc

CDR_List.o: CDR_List.cc
	g++ -c CDR_List.cc

Key_List.o: Key_List.cc
	g++ -c Key_List.cc

heap.o: heap.cc
	g++ -c heap.cc

clean:
	rm exe

