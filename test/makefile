INC=-I..\..\..\..\boost-sandbox -I..\..\..\..\..\lib\boost_1_29_0 


all: bin\test1.exe bin\test2.exe bin\test3.exe bin\test5.exe bin\test6.exe

run: all
	bin\test1.exe
	bin\test2.exe
	bin\test3.exe
	bin\test5.exe
	bin\test6.exe


clean:
	del bin\*.*



bin\test1.exe: bin\test1.o bin\class_a.o
	gxx bin\test1.o bin\class_a.o -o bin\test1.exe

bin\test1.o: bin test1.cpp
	gxx -c test1.cpp -o bin\test1.o $(INC)

bin\class_a.o: bin class_a.cpp class_a.h
	gxx -c class_a.cpp -o bin\class_a.o $(INC)

bin\test2.exe: bin test2.cpp
	gxx test2.cpp -o bin\test2.exe $(INC)

bin\test3.exe: bin test3.cpp
	gxx test3.cpp -o bin\test3.exe $(INC)

bin\test5.exe: bin test5.cpp
	gxx test5.cpp -o bin\test5.exe $(INC)

bin\test6.exe: bin test6.cpp
	gxx test6.cpp -o bin\test6.exe $(INC)


bin:
	mkdir bin




bin\s.exe: bin s.cpp
	gxx s.cpp -o bin\s.exe $(INC)

