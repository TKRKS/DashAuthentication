all:
	make generateKey mpdParser validationTest

hashFile: 
	g++ -g -O0 -DDEBUG -Wall -Wno-unused -std=c++11 -o hashFile HashFile.cpp -lcryptopp -lpthread

signFile: 
	g++ -g -O0 -DDEBUG -Wall -Wno-unused -std=c++11 -o signFile SignFile.cpp -lcryptopp -lpthread

generateKey: 
	g++ -g -O0 -DDEBUG -Wall -Wno-unused -std=c++11 -o generateKey GenerateKey.cpp -lcryptopp -lpthread

mpdParser: 
	g++ -g -O0 -DDEBUG -Wall -Wno-unused -std=c++11 -o mpdParser MpdParser.cpp -lcryptopp -lpthread

validationTest:
	g++ -g -O0 -DDEBUG -Wall -Wno-unused -std=c++11 -o validationTest ValidationTest.cpp -lcryptopp -lpthread

clean:
	rm -f generateKey mpdParser validationTest hashFile
