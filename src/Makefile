all: plsa

plsa: Database.o Plsm.o plsa.o
	$(CXX) -o plsa plsa.o Database.o Plsm.o

Database.o: Database.cpp Database.h
	$(CXX) -c Database.cpp

Plsm.o: Plsm.cpp Plsm.h
	$(CXX) -c Plsm.cpp

plsa.o: plsa.cpp
	$(CXX) -c plsa.cpp

clean:
	rm *.o plsa