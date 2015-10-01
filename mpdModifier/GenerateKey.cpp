#include "Crypto.h"

int main (int argc, const char* argv[]) {
	AutoSeededRandomPool rng;
	InvertibleRSAFunction privateKey;
	privateKey.Initialize(rng, 1024);
	
	string privateKeyFile(string(argv[1]) + ".priv");
	 
	//Private KEy
	Base64Encoder privateKeySink(new FileSink(privateKeyFile.c_str()));
	privateKey.DEREncode(privateKeySink);
	//Call to save file correctly.
	privateKeySink.MessageEnd();
	 

	string publicKeyFile(string(argv[1]) + ".pub");
	//Public Key
	RSAFunction publicKey(privateKey);
	 
	Base64Encoder publicKeySink(new FileSink(publicKeyFile.c_str()));
	publicKey.DEREncode(publicKeySink);
	//Call to save file correctly.
	publicKeySink.MessageEnd();
}
