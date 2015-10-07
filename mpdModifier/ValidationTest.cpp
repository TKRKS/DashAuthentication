#include <string>
#include <sstream>
#include <iostream>

#include <gmpxx.h>
#include <cryptopp/base64.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>
#include <cryptopp/sha.h>

using namespace std;
using namespace CryptoPP;

bool verifyFile(string sig, string fileLocation, string publicKeyLocation) {
   CryptoPP::ByteQueue publicKeyBytes;
   FileSource publicKeyFile(publicKeyLocation.c_str(), true, new Base64Decoder);
	cout << "Public key file loaded" << endl;
   publicKeyFile.TransferTo(publicKeyBytes);
	cout << "Public key file bytes transfered" << endl;
   publicKeyBytes.MessageEnd();
	cout << "Public key bytes end" << endl;
   RSA::PublicKey publicKey;
   publicKey.Load(publicKeyBytes);

	cout << "Public key loaded" << endl;

   RSASSA_PKCS1v15_SHA_Verifier verifier(publicKey);

   //Read signed message
   std::string mpdText;
   FileSource(fileLocation.c_str(), true, new StringSink(mpdText));
   std::string signatureText;

      std::cout << "Hex Encoded: " << sig.c_str() << std::endl;
   HexDecoder decoder;
   decoder.Attach(new StringSink(signatureText));
   decoder.Put( (const byte*) sig.c_str(), sig.size() );
   decoder.MessageEnd();

   std::cout << "Decoded: " << signatureText.c_str() << std::endl;

   std::string combined(mpdText);
   combined.append(signatureText);

   //Verify signature
   try{
       StringSource(mpdText + signatureText, true,
           new SignatureVerificationFilter(
               verifier, NULL,
               SignatureVerificationFilter::THROW_EXCEPTION
          )
       );
       return true;
   } catch(SignatureVerificationFilter::SignatureVerificationFailed &err) {
	cout << err.what() << endl;
       return false;
   }
}

string signFile(string filePath, string privateKeyLocation) {
	string toSign;
	FileSource(filePath.c_str(), true, new StringSink(toSign));

	AutoSeededRandomPool rng;

	//Read private key
	CryptoPP::ByteQueue bytes;
	FileSource file(privateKeyLocation.c_str(), true, new Base64Decoder);
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

	cout << "Signature: " << signature.data() << endl;
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

	cout << "Hex signature string: " << signatureString << endl;

	return signatureString;
}

int main (int argc, const char* argv[]) {

	string publicKey(argv[1]);
	string privateKey(argv[2]);
	string fileName(argv[3]);

	string signature(signFile(fileName, privateKey));
	cout << signature << endl;
	bool validation = verifyFile(signature, fileName, publicKey); 
	cout << validation << endl;
}
