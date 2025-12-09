run:	a
	
a:	fileSystem.cpp
	g++ fileSystem.cpp -o build 

clean:	
	rm -rf ./build *.out
