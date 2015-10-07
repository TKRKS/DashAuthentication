#include "Crypto.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_utils.hpp"

using namespace rapidxml;

string hashFile(string fileName) {
	SHA512 hash;
	byte buffer[2 * SHA512::DIGESTSIZE];

	FileSource f(fileName.c_str(), true,
		     new HashFilter(hash,
		     new HexEncoder(new ArraySink(buffer,2 * SHA512::DIGESTSIZE))));

	return string((const char*)buffer,2 * SHA512::DIGESTSIZE);
}

void processNodeChildren(xml_node<> * node, xml_document<> &mpd, string baseDir) {
	for (xml_node<> *child = node->first_node(); child; child = child->next_sibling()) {
	    processNodeChildren(child, mpd, baseDir);
	}
	//If initialization or segmenturl node, append hash.  Otherwise, do nothing
	if (node->name() == string("Initialization")) {
		string fileName(node->first_attribute(string("sourceURL").c_str())->value());
		char *attrName = mpd.allocate_string("hash");
		xml_attribute<> *attr = mpd.allocate_attribute(attrName, mpd.allocate_string(hashFile(string(baseDir + fileName)).c_str()));
		node->append_attribute(attr);
	} else if (node->name() == string("SegmentURL")) {
		string fileName(node->first_attribute(string("media").c_str())->value());
		char *attrName = mpd.allocate_string("hash");
		xml_attribute<> *attr = mpd.allocate_attribute(attrName, mpd.allocate_string(hashFile(string(baseDir + fileName)).c_str()));
		node->append_attribute(attr);
	}
}

string signMpd(string filePath, string privateKeyLocation) {
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

	return signatureString;
}

int main (int argc, const char* argv[]) {
	//Load XML file
	rapidxml::file<> mpdFile(argv[1]);
	xml_document<> mpd;
	mpd.parse<0>(mpdFile.data());

	//Hash video segments
	processNodeChildren(mpd.first_node(), mpd, string(argv[2]));

	//Write hashed mpd (so the hashes are included in the signature)
    	std::ofstream outputFile;
	string modifiedFileName(string(argv[2]) + "/authenticatedMpd.mpd");
    	outputFile.open(modifiedFileName);
	outputFile << mpd;
	outputFile.close();

	//Sign hashed mpd
	string signature = signMpd(modifiedFileName, string(argv[3]));
	rapidxml::file<> hashedMpdFile(modifiedFileName.c_str());
	xml_document<> hashedMpd;
	hashedMpd.parse<0>(hashedMpdFile.data());
	xml_node<> *mpdNode = hashedMpd.first_node();
	xml_node<> *signatureNode = hashedMpd.allocate_node(node_element, hashedMpd.allocate_string(string("Signature").c_str()));
	signatureNode->value(hashedMpd.allocate_string(signature.c_str()));
	mpdNode->append_node(signatureNode);

	//Write signed mpd
    	std::ofstream finalOutputFile;
    	finalOutputFile.open(string(string(argv[2]) + "/authenticatedMpd.mpd"));
	finalOutputFile << hashedMpd;
	finalOutputFile.close();
}
