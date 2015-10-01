#include "Crypto.h"

int main(int argc, const char* argv[]) {
	SHA512 hash;
	byte buffer[2 * SHA512::DIGESTSIZE]; // Output size of the buffer

	cout << "Test1" << endl;
	 
	FileSource f(argv[1], true,
		     new HashFilter(hash,
		     new HexEncoder(new ArraySink(buffer,2 * SHA512::DIGESTSIZE))));
	
	cout << "Test2" << endl;

	cout << string((const char*)buffer,2 * SHA512::DIGESTSIZE)  << endl;
}



