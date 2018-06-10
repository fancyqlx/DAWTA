/*
 * MyRSA.cpp
 *
 *  Created on: 2013-3-7
 *      Author: hust
 */
#include "MyRSA.hpp"
#include <time.h>
MyRSA::MyRSA()
{

}

MyRSA::~MyRSA()
{

}
/*
 * Description: this function is used to calcuate the string 'message' 's hash value
 * Input:
 * 	message: the init string to be hashed
 * Output:
 * 	return the hash of the parameter
 */
string MyRSA::MD5(const char * message)
{
	string digest;
	Weak::MD5 md5;
	StringSource(message, true,
			new HashFilter(md5, new HexEncoder(new StringSink(digest))));
	return digest;
}
/*
 * Description: to calculate the hash of the file and return the hash value(string)
 * Input:
 * 	filename: the file to be calculated the hash value
 * Output:
 *  return the hash value of the file and its type is string
 */
string MyRSA::MD5File(const char * filename)
{
	string digest;
	Weak::MD5 md5;
	FileSource(filename, true,
			new HashFilter(md5, new HexEncoder(new StringSink(digest))));
	return digest;

}

/*
 * Description: generate the RSA public key and private key in separate file
 * Input:
 *  KeyLength: the length of the key, such as 1024...
 *  privFilename: private key file name you want to store the private key
 *  pubFilename: public key file name you want to store the public key
 * Output:
 * 	nothing
 */
void MyRSA::GenerateRSAKey(unsigned int keyLength, const char *privFilename,
		const char *pubFilename)
{
	RSAES_OAEP_SHA_Decryptor priv(_rng, keyLength);
	HexEncoder privFile(new FileSink(privFilename));
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	RSAES_OAEP_SHA_Encryptor pub(priv);
	HexEncoder pubFile(new FileSink(pubFilename));
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}

/*
 * Description: this function is used to encrypt the string 'plainText' with the
 * 				private key, and return the cipher
 * Input:
 * 	pubFilename: the public key
 * 	message: the string to be encrypted
 * OutPut:
 *  return the cipher
 */
string MyRSA::Encrypt(const char * pubFilename, const char * message)
{
	FileSource pubFile(pubFilename, true, new HexDecoder);

	RSAES_OAEP_SHA_Encryptor pub(pubFile);
	string result;
	StringSource(message, true,
			new PK_EncryptorFilter(_rng, pub,
					new HexEncoder(new StringSink(result))));
	return result;
}
/*
 * Description: decrypt the cipher with the private key
 * Input:
 * 	privFilename: the private key file
 * 	ciphertext: the string to be decrypted
 * Output:
 * 	return the decrypted string
 */
string MyRSA::Decrypt(const char * privFilename, const char * ciphertext)
{
	FileSource privFile(privFilename, true, new HexDecoder);

	RSAES_OAEP_SHA_Decryptor priv(privFile);
	string result;
	StringSource(ciphertext, true,
			new HexDecoder(
					new PK_DecryptorFilter(_rng, priv,
							new StringSink(result))));
	return result;
}

