#include "Crypto.h"

int main (int argc, const char* argv[]) {
	string toSign;
	FileSource(argv[1], true, new StringSink(toSign));
	   
	AutoSeededRandomPool rng;
	   
	//Read private key
	CryptoPP::ByteQueue bytes;
	FileSource file(argv[2], true, new Base64Decoder);
	file.TransferTo(bytes);
	bytes.MessageEnd();
	RSA::PrivateKey privateKey;
	privateKey.Load(bytes);
	   
	//Sign message
	RSASSA_PKCS1v15_SHA_Signer signer(privateKey);
	SecByteBlock signature(signer.SignatureLength());
	signer.SignMessage(
		rng,
		(byte const*) toSign.data(),
		toSign.size(),
		signature);

	//Print string of signature
	HexEncoder encoder;
	string signatureString;
	encoder.Put(signature.data(), signature.size());
	encoder.MessageEnd();
	word64 signatureSize = encoder.MaxRetrievable();
	if (signatureSize) {
		signatureString.resize(signatureSize);		
		encoder.Get((byte*) signatureString.data(), signatureString.size());
	}
	   
	cout << signatureString << endl;
}
