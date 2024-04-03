all: clean SingleProcess BFS BFSSignal DFS
	echo "All files compiled"

SingleProcess: sProcess.c
	gcc -o sProcess sProcess.c

BFS: BreadthFS.c
	gcc -o BreadthFS BreadthFS.c -lm

BFSSignal: BreadthFSSignal.c
	gcc -o BreadthFSSignal BreadthFSSignal.c -lm

DFS: DepthFS.c
	gcc -o DepthFS DepthFS.c

clean:
	rm -f sProcess *.o BreadthFS BreadthFSSignal DepthFS keys.txt output.txt

pack:
	rm -f Project.tar
	tar -cvf Project.tar *.c *.h Makefile README.txt