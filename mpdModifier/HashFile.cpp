#include "Crypto.h"

int main(int argc, const char* argv[]) {
	SHA512 hash;
	byte buffer[2 * SHA512::DIGESTSIZE];

	cout << "Test1" << endl;
	streampos begin,end;
	ifstream file(argv[1], ios::in|ios::binary|ios::ate);

	long fileSize = file.tellg();
	cout << "Test1.5 " << fileSize << endl;
	byte fileBuffer[fileSize];
	file.seekg(0, ios::beg);
	cout << "Test2" << endl;
	file.read((char *) fileBuffer, fileSize);
	cout << "Test3" << endl;
	file.close();
	cout << string((const char*) fileBuffer, fileSize) << endl;
	ArraySource source(fileBuffer, fileSize, true,
		     new HashFilter(hash,
		     new HexEncoder(new ArraySink(buffer,2 * SHA512::DIGESTSIZE))));

	cout << string((const char*)buffer,2 * SHA512::DIGESTSIZE)  << endl;
}



