
#ifndef PRECOMPILEDHEADERS_H
#include "PreCompiledHeaders.h"
#endif

#ifdef COMPILE_WITH_OPEN_TRANSACTIONS

#include "OTX_WRAP.h"


static OTCaller           passwordCaller;
static MTPasswordCallback passwordCallback;

OTX_WRAP::OTX_WRAP(QWidget *parent)
	{
	pParentWidget= parent;
	this->LoadWallewithPassprhase();
	}

//=======================================UTILITIES==============================================================

void OTX_WRAP::SL_DownloadedURL()
	{
	QString qstrContents(m_pImgCtrl->downloadedData());
	std::cout << "\nSLOT Downloaded Data: \n";
	std::cout << qstrContents.toStdString();
	std::cout << "\n============DATA========\n";
	// ----------------------------
	int32_t nAdded;
	// OT Server relationship with Sopro Client
	if (!qstrContents.isEmpty())
		nAdded = OTAPI_Wrap::It()->AddServerContract(qstrContents.toStdString());
	else
	{
	std::cout << "Failed Importing Server Contract. Failed trying to import contract. Is it already in the wallet?";
	return;
	}


	}

// private compression and decompression functions to support encoded strings
/** Compress a STL string using zlib with given compression level and return
  * the binary data. */

std::string OTX_WRAP::compress_string(const std::string& str,
									  int compressionlevel)
	{
	z_stream zs;                        // z_stream is zlib's control structure
	memset(&zs, 0, sizeof(zs));

	if (deflateInit(&zs, compressionlevel) != Z_OK)
		throw(std::runtime_error("deflateInit failed while compressing."));

	zs.next_in = (Bytef*)str.data();
	zs.avail_in = str.size();           // set the z_stream's input

	int ret;
	char outbuffer[32768];
	std::string outstring;

	// retrieve the compressed bytes blockwise
	do {
	zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
	zs.avail_out = sizeof(outbuffer);

	ret = deflate(&zs, Z_FINISH);

	if (outstring.size() < zs.total_out) {
	// append the block to the output string
	outstring.append(outbuffer,
					 zs.total_out - outstring.size());
	}
	} while (ret == Z_OK);

	deflateEnd(&zs);

	if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
	std::ostringstream oss;
	oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
	throw(std::runtime_error(oss.str()));
	}

	return outstring;
	}

/** Decompress an STL string using zlib and return the original data. */
std::string OTX_WRAP::decompress_string(const std::string& str)
	{
	z_stream zs;                        // z_stream is zlib's control structure
	memset(&zs, 0, sizeof(zs));

	if (inflateInit(&zs) != Z_OK)
		throw(std::runtime_error("inflateInit failed while decompressing."));

	zs.next_in = (Bytef*)str.data();
	zs.avail_in = str.size();

	int ret;
	char outbuffer[32768];
	std::string outstring;

	// get the decompressed bytes blockwise using repeated calls to inflate
	do {
	zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
	zs.avail_out = sizeof(outbuffer);

	ret = inflate(&zs, 0);

	if (outstring.size() < zs.total_out) {
	outstring.append(outbuffer,
					 zs.total_out - outstring.size());
	}

	} while (ret == Z_OK);

	inflateEnd(&zs);

	if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
	std::ostringstream oss;
	oss << "Exception during zlib decompression: (" << ret << ") "
		<< zs.msg;
	throw(std::runtime_error(oss.str()));
	}

	return outstring;
	}


//====================================CRYPTO=======================================================================
//====================================SYMMETRIC (OPENSSL)================================================================
std::string OTX_WRAP::symmetricEncStr(std::string plainText)
	{

	unsigned char * plainTextUCH = (unsigned char *)plainText.c_str();
	unsigned char theEncryptedText[1024];
	int encrypted_len=this->symmetricEncrypt(plainTextUCH,theEncryptedText);
	//std::string encStr = std::string(reinterpret_cast<const char*>(theEncryptedText));
	// Encode with ascii ints the just encrypted text in order to made it compatible with Xmpp base85 requirements
	std::string encodedEncStr="";
	std::string tmpStr="";
	int ch;
	for (int i=0; i < encrypted_len; i++)
	{
	ch= theEncryptedText[i];
	tmpStr= to_string(ch);
	encodedEncStr=encodedEncStr+tmpStr;
	encodedEncStr+="-";
	}
	std::cout << "\n Compression stats:\n Original:";
	std::cout << encodedEncStr.length();

	/*std::string compressed=compress_string(encodedEncStr,1);
 std::cout << "\n Compressed:>>>>" +  compressed + "<<<<<<";

  return compressed;*/
	return encodedEncStr;
	}
std::string OTX_WRAP::symmetricDecStr(std::string encText)
	{
	//decode the ascii ints
	/*  std::cout << "\nAbout to decompress...\n>>>>" +encText+"<<<<";
	std::string decompressed= this->decompress_string(encText);*/
	std::string decompressed= encText;

	std::cout << "decompressed...\n";
	std::cout << decompressed;

	unsigned char * encTextUCH = (unsigned char *) decompressed.c_str();
	std::cout << "About to decrypt:";
	std::cout << encTextUCH;
	std::cout << "\n Lenght: ";

	// Get all asciis and convert it to string
	int encTextLen= strlen((char *)encTextUCH);

	int ch;
	char chr;
	std::string buff;
	unsigned char  encBuffText[1024];
	//initialize buffer
	for(int i=0; i < 1024; i++)
	{  encBuffText[i]='\0';}

	std::cout << "BUFFER INITILIZED";
	int c=0; //buffer counter
	int ascii;
	// Rebuild the origninal string
	for (int i = 0; i < encTextLen; i++)
	{
	ch=encTextUCH[i];
	chr=(char)ch;

	// 45 is - in ASCII
	buff="";

	while (ch != 45)
	{

	buff=buff+chr;
	ch=encTextUCH[++i];
	chr=(char)ch;

	}
	//std::cout << "\n buff:"+buff + "\n";
	ascii=std::stoi(buff);

	encBuffText[c]=ascii;
	c++;

	}
	std::cout << "\n===== Buffer out: \n";
	std::cout << encBuffText;
	std::cout << "\n===== lenght:\n";
	unsigned char thePlainText[1024];
	std::cout << strlen((const char *) encBuffText);
	std::cout << encTextUCH;
	std::cout << "\n";

	int decrypted_len=this->symmetricDecrypt(encBuffText,thePlainText);
	std::string decStr = std::string(reinterpret_cast<const char*>(thePlainText));

	return decStr;

	}

int  OTX_WRAP::symmetricEncrypt(unsigned char * plainText, unsigned char (&encrypted)[1024])
	{
	EVP_cleanup();
	ERR_free_strings();
	/* A 256 bit key */
	unsigned char *key = (unsigned char*) "$!C3NT@LS3rv1c3sW3lc0m3t0P@nt30n!$";

	/* A 128 bit IV */
	unsigned char *iv =  (unsigned char*) "01234567890123456";



	/* Buffer for ciphertext. Ensure the buffer is long enough for the
   * ciphertext which may be longer than the plaintext, dependant on the
   * algorithm and mode
   */
	unsigned char ciphertext[1024];



	int  ciphertext_len;

	/* Initialise the library */
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);

	/* Encrypt the plaintext */
	ciphertext_len =  encrypt(plainText, strlen((char *)plainText), key, iv,ciphertext);
	ciphertext[ciphertext_len] = '\0';
	std::cout << "\n ================ENCRYPT OTX_WRAP================ \n";
	std::cout << "\n PLAIN TEXT: \n";
	std::cout << plainText;
	std::cout << "\n ENCRYPTED TEXT :\n";
	std::cout << ciphertext;
	std::cout << "\n LENGTH ENCRYPTED:\n";
	std::cout << ciphertext_len;
	encrypted[0]== '\0';
	memcpy(encrypted,ciphertext,ciphertext_len);
	encrypted[ciphertext_len]='\0';
	return ciphertext_len;
	}

int OTX_WRAP::symmetricDecrypt(unsigned char ciphertext[1024], unsigned char (&plainText)[1024])
{

	/* Buffer for the decrypted text */
	unsigned char decryptedtext[1024];

	unsigned char *key = (unsigned char*) "$!C3NT@LS3rv1c3sW3lc0m3t0P@nt30n!$";

	/* A 128 bit IV */
	unsigned char *iv =  (unsigned char*) "01234567890123456";
	int ciphertext_len = strlen((char *)ciphertext);
	std::cout << "\n SYMETRIC DECRYPT ENCRIPTED LEN\n";
	std::cout << ciphertext_len;
	int decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv,
									decryptedtext);

	/* Add a NULL terminator. We are expecting printable text */
	decryptedtext[decryptedtext_len] = '\0';
	plainText[0]== '\0';
	memcpy(plainText,decryptedtext,decryptedtext_len);
	plainText[decryptedtext_len]='\0';

	std::cout << "\n ================DECRYPT OTX_WRAP================ \n";
	std::cout << "\n ENCRYPTED TEXT: \n";
	std::cout << ciphertext;
	std::cout << "\n DECRYPTED TEXT :\n";
	std::cout << decryptedtext;
	std::cout << "\n LENGTH ENCRYPTED:\n";
	std::cout << decryptedtext_len;

	/* Clean up */
	EVP_cleanup();
	ERR_free_strings();
	return decryptedtext_len;

	}


void OTX_WRAP::handleErrors(void)
	{
	ERR_print_errors_fp(stderr);
	abort();
	}
int OTX_WRAP::encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
					  unsigned char *iv, unsigned char *ciphertext)
	{
	EVP_CIPHER_CTX *ctx;

	int len;

	int ciphertext_len;

	/* Create and initialise the context */
	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	/* Initialise the encryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits */
	if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
		handleErrors();

	/* Provide the message to be encrypted, and obtain the encrypted output.
	 * EVP_EncryptUpdate can be called multiple times if necessary
	 */
	if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		handleErrors();
	ciphertext_len = len;

	/* Finalise the encryption. Further ciphertext bytes may be written at
	 * this stage.
	 */
	if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
	ciphertext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return ciphertext_len;
	}

int OTX_WRAP::decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
					  unsigned char *iv, unsigned char *plaintext)
	{
	EVP_CIPHER_CTX *ctx;

	int len;

	int plaintext_len;

	/* Create and initialise the context */
	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	/* Initialise the decryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits */
	if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
		handleErrors();

	/* Provide the message to be decrypted, and obtain the plaintext output.
	 * EVP_DecryptUpdate can be called multiple times if necessary
	 */
	if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		handleErrors();
	plaintext_len = len;

	/* Finalise the decryption. Further plaintext bytes may be written at
	 * this stage.
	 */
	if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
	plaintext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return plaintext_len;
	}





//====================================CRYPTO===================================================================
//====================================ASYMMETRIC (OT)================================================================
QString OTX_WRAP::signText(QString s_nymId,QString qstrText)
	{

	std::string  str_nym    (s_nymId.toStdString());
	OTString     strNym     (str_nym.c_str());
	OTIdentifier nym_id     (strNym);
	std::string  str_text   (qstrText.toStdString());
	OTString     strText    (str_text.c_str());


	QString signedText;

	OTPasswordData thePWData("Signer passphrase");

	OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(nym_id,
																  false, //bChecking=false
																  "DlgEncrypt::on_pushButtonEncrypt_clicked",
																  &thePWData);



	if (NULL == pNym) //signer not loaded
	{
	/* QString qstrErrorMsg = QString("%1: %2").arg("Failed loading the signer; unable to continue. NymID").arg(s_nymId);
		QMessageBox::warning(pParentWidget, "Failed Loading Signer", qstrErrorMsg);*/
	return "Failed to load signer";
	}
	else //signer loaded
	{

	OTString     strSignedOutput;
	OTSignedFile theSignedFile;

	theSignedFile.SetFilePayload(strText);
	theSignedFile.SignContract(*pNym, &thePWData);
	theSignedFile.SaveContract();
	theSignedFile.SaveContractRaw(strSignedOutput);



	if (!strSignedOutput.Exists()) // The text´s signature fail
	{
	QMessageBox::warning(pParentWidget, "Signing Failed",
						 "Failed trying to sign, using the selected identity.");
	return "Signing Failed";
	}
	else if (!theSignedFile.VerifySignature(*pNym)) //sign verification
	{
	QMessageBox::warning(pParentWidget, "Test Verification Failed",
						 "Failed trying to test verify, immediately after signing. Trying authentication key...");

	if (!theSignedFile.VerifySigAuthent(*pNym))
	{
	QMessageBox::warning(pParentWidget, "Authent Test Also Failed",
						 "Failed trying to verify signature with authentication key as well.");
	return "Failed veryfiing Signature";
	}
	else
		QMessageBox::information(pParentWidget, "SUCCESS USING AUTHENTICATION KEY", "Tried authent key instead of signing key, and it worked!");
	}
	else
	{
	signedText=strSignedOutput.Get();
	}
	} // else (we have pNym.)

	return signedText;
	}



QString OTX_WRAP::encryptText(QString e_nymId, QString plainText)
	{
	QString encryptedText="Failed to encrypt : \n "+ plainText;// if encryption fails notify and return plain text
	OTString strContactNymID(e_nymId.toStdString().c_str());
	OTIdentifier contact_nym_id(strContactNymID);
	setOfNyms setRecipients;

	if (!contact_nym_id.IsEmpty())
	{
	// OTPasswordData thePWData("Load Credentials");

	OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadPublicNym(contact_nym_id);
	if (NULL == pNym)
	{
	std::cout  << "\n Failed to load Public Nym: " +e_nymId.toStdString() +". Check if exists in this OT server. \n";
	//QString qstrErrorMsg = QString("%1: %2").
	//        arg("Failed trying to load the signer ").arg(e_nymId);
	//QMessageBox::warning(pParentWidget, "Failed Loading Signer", qstrErrorMsg);
	}
	else
	{
	setRecipients.insert(setRecipients.begin(), pNym);

	OTEnvelope theEnvelope;
	OTString   strInput(plainText.toStdString().c_str());

	if (!theEnvelope.Seal(setRecipients, strInput))
	{
	//QMessageBox::warning(pParentWidget, "Encryption Failed",
	//                     "Failed trying to encrypt message.");
	return encryptedText;//return plain text with failure message
	}
	else
	{
	// Success encrypting!
	//
	OTString     strOutput;
	OTASCIIArmor ascCiphertext(theEnvelope);

	if (ascCiphertext.WriteArmoredString(strOutput, "ENVELOPE")) // -----BEGIN OT ARMORED ENVELOPE-----
	{
	std::string str_output(strOutput.Get());
	encryptedText = QString::fromStdString(str_output);
	}
	}

	}


	} else {std::cout  << "\n Failed to load Contact Nym! Check if the contact has a valid nym.  Try /f inside chat window \n";}


	return encryptedText;
	}

QString OTX_WRAP::signAndEncrypt(QString signerNymId,QString recipientNymId,QString plainText)
	{
	QString signedAndEncrypted=encryptText(recipientNymId,signText(signerNymId,plainText));
	return signedAndEncrypted;
	}

// Decryption Functions
bool OTX_WRAP::verifySignature(QString s_nymId, QString signedPlainText,QString &messagePayload)
	{
	std::string str_input=signedPlainText.toStdString();
	OTString strInput=signedPlainText.toStdString();
	bool bSuccessVerifying = false;

	if (strInput.Contains("-----BEGIN SIGNED FILE-----"))
	{
	OTSignedFile theSignedFile;

	if (theSignedFile.LoadContractFromString(strInput))
	{


	OTString     strNym     (s_nymId.toStdString().c_str());
	OTIdentifier nym_id     (strNym);


	if (s_nymId.length() > 0)
	{
	OTPasswordData thePWData("load credentials");

	OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadPublicNym(nym_id);
	if (NULL != pNym)
	{
	if (theSignedFile.VerifySignature(*pNym, &thePWData))
	{
	bSuccessVerifying = true;

	OTString strContents = theSignedFile.GetFilePayload();

	if (strContents.Exists())
	{
	strInput  = strContents;
	str_input = strInput.Get();
	// armored payload


	if(strInput.DecodeIfArmored(false)  ){

	std::string str_decoded(strInput.Get());
	QString messagePayloadDecoded(str_decoded.c_str());
	messagePayload=messagePayloadDecoded;
	} else // not armored
	{

	strInput  = strContents;
	str_input = strInput.Get();
	messagePayload=QString::fromStdString(str_input);
	}


	}
	} // signature verified
	} // pNym exists
	} // if str_signer_nym exists


	} // signed file: load contract from string.
	} // "BEGIN SIGNED FILE"
	return bSuccessVerifying;
	}

QString OTX_WRAP::decryptText(QString nymId,QString encryptedText)
	{
	QString decryptedText="<Failed>";
	std::string str_input(encryptedText.toStdString());
	OTString  strInput (str_input.c_str());
	if (strInput.Exists())
	{
	if (strInput.Contains("-----BEGIN OT ARMORED ENVELOPE-----"))
	{
	OTEnvelope theEnvelope;

	if (theEnvelope.SetFromBookendedString(strInput))
	{
	OTString strOutput;
	// -------------------------
	// First we'll try the default nym, if one is available.
	//
	QString qstrTempID = nymId;

	if (!qstrTempID.isEmpty())
	{
	std::string  str_nym    (qstrTempID.toStdString());
	OTString     strNym     (str_nym.c_str());
	OTIdentifier nym_id     (strNym);

	if (!nym_id.IsEmpty())
	{
	OTPasswordData thePWData("Recipient passphrase");

	OTPseudonym * pNym = OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(nym_id,
																  false, //bChecking=false
																  "DlgEncrypt::on_pushButtonDecrypt_clicked",
																  &thePWData);
	if (NULL != pNym)
	{
	if (theEnvelope.Open(*pNym, strOutput) && strOutput.Exists())
	{
	strInput  = strOutput;
	str_input = strInput.Get();
	decryptedText  = QString::fromStdString(str_input);
	}
	}
	}
	}
	// ------------

	// -----------------------
	} // if (theEnvelope.SetFromBookendedString(strInput))

	} // if (strInput.Contains("-----BEGIN OT ARMORED ENVELOPE-----"))
	}
	return decryptedText;
	}

bool OTX_WRAP::decryptAndVerify(QString signerNymId, QString recipientNymId,
								QString signedEncryptedText, QString &decryptedText)
	{
	std::cout << "\nOTX_WRAP: decryptAndVerify: \n";
	std::cout << signedEncryptedText.toStdString()+"\n recipientNymId:";
	std::cout << recipientNymId.toStdString();

	QString signedDecrypted=decryptText(recipientNymId,signedEncryptedText);
	QString messagePayload;
	if (!signedDecrypted.contains("<Failed>")){
	if (verifySignature(signerNymId,signedDecrypted,messagePayload))
		decryptedText=messagePayload;
	else
		decryptedText="Signature Invalid for decrypted message: "+messagePayload;

	}
	else
	{decryptedText="Failed to Decrypt the Text with NymId "+recipientNymId;
	return false;}

	return true;
	}

//====================================OPEN TRANSACTIONS===================================================================
//====================================ACCESS TO CORE=========================================================================

bool OTX_WRAP::SetupPasswordCallback(OTCaller & passwordCaller, OTCallback & passwordCallback)
	{
	passwordCaller.setCallback(&passwordCallback);

	bool bSuccess = OT_API_Set_PasswordCallback(passwordCaller);

	if (!bSuccess)
	{
	qDebug() << QString("Error setting password callback!");
	return false;
	}

	return true;
	}

bool OTX_WRAP::SetupAddressBookCallback(OTLookupCaller & theCaller, OTNameLookup & theCallback)
	{
	theCaller.setCallback(&theCallback);

	bool bSuccess = OT_API_Set_AddrBookCallback(theCaller);

	if (!bSuccess)
	{
	qDebug() << QString("Error setting address book callback!");
	return false;
	}

	return true;
	}

void OTX_WRAP::LoadWallewithPassprhase()
	{
	QString mc_app_name = "SoproOTX";
	QString mc_version = "v0.0.1";

	//Compiled details


	if (!SetupPasswordCallback(passwordCaller, passwordCallback))
	{
	qDebug() << "Failure setting password callback in MTApplicationMC";
	abort();
	}
	// ----------------------------------------
	// Set Address Book Callback.
	//
	static OTLookupCaller theCaller;
	static MTNameLookupQT theCallback;

	if (!SetupAddressBookCallback(theCaller, theCallback))
	{
	qDebug() << "Failure setting address book callback in MTApplicationMC";
	abort();
	}
	// ----------------------------------------
	// Load OTAPI Wallet
	//
	OTAPI_Wrap::It()->LoadWallet();
	// ----------------------------------------




	}
// Publish the nym in all OT servers added

int OTX_WRAP::publishNymAllServers(std::string nymId)
	{
	std::string serverId;
	int c=0;
	int serverCount=OTAPI_Wrap::It()->GetServerCount();
	for (int i=0; i < serverCount; i++) // verify all servers
	{
	serverId=OTAPI_Wrap::It()->GetServer_ID(i);
	if (!OTAPI_Wrap::It()->IsNym_RegisteredAtServer(nymId,serverId))
	{ // the nym is not published in this server.. publish it

	OT_ME       madeEasy;
	std::string response;
	{
	response = madeEasy.register_nym(serverId, nymId);
	}

	qDebug() << QString("Nym Creation Response: %1").arg(QString::fromStdString(response));

	int32_t nReturnVal = madeEasy.VerifyMessageSuccess(response);

	if (1 != nReturnVal)
	{
	qDebug() << QString::fromStdString("Nym : "+nymId +" not published in server: "+serverId);
	}
	else
	{
	qDebug() << QString::fromStdString("Nym : "+nymId +" was published successfully in server: "+serverId);
	c++;
	}
	}   // if nym is registered into the server
	} // loop

	return c; // return the number of servers where the nym was published
	}

std::string OTX_WRAP::createNym(std::string name,int keysize)
	{
	int serverCount=OTAPI_Wrap::It()->GetServerCount();

	if (serverCount > 0)
	{
	//create the role inside OT
	std::string nymId=OTAPI_Wrap::It()->CreateNym(keysize, "","");

	if (OTAPI_Wrap::It()->SetNym_Name(nymId,nymId,name))
	{
	// before return the created nym, publish it in all OT servers loaded
	publishNymAllServers(nymId);
	return nymId;
	}
	else
	{
	return "NoNymCreated";
	}
	}
	else
	{
	// No server loaded (maybe deleted in the ui?) load the available servers from internet and display and inform

	this->addOTServerContracts();
	return "NoNymCreated";
	}
	}

std::string OTX_WRAP::getNymID(int32_t nymIndex)
	{
	return OTAPI_Wrap::GetNym_ID(nymIndex);
	}

std::string OTX_WRAP::getNym_name(std::string nymId)
	{

	return OTAPI_Wrap::GetNym_Name(nymId);

	}

std::string OTX_WRAP::getNymPublicKey(std::string nymid)
	{


	return OTAPI_Wrap::GetNym_SourceForID(nymid);

	}

//====================================OPEN TRANSACTIONS===================================================================
//=======================================WIDGETS=======================================================================

// send message to nym (send encrypted and store into a nymbox in server)
bool OTX_WRAP::sendMessageToNymBox(std::string str_serverId, std::string str_fromNymId, std::string str_toNymId, std::string contents)
{
    OT_ME madeEasy;

    std::string strResponse;
    {


        strResponse = madeEasy.send_user_msg(str_serverId, str_fromNymId, str_toNymId, contents);
    }

    int32_t nReturnVal = madeEasy.VerifyMessageSuccess(strResponse);

    if (1 != nReturnVal)
    {
        qDebug() << "OT send_message: Failed.";



        return false;

    }
 }

// Widget that handles the
void OTX_WRAP::openContractOTServerScreen()
	{
	OTX::It(pParentWidget)->mc_defaultserver_slot();

	}

// Widget that handle the role nym creation integrated with sopro
void OTX_WRAP::openRoleCreationScreen()
	{
	// Get the current profile in Cambrian

	TProfile * pProfile = new TProfile(&g_oConfiguration);	// At the moment, there is only one configuration object, so use the global variable
	// Get the count of current profiles in Cambrian
	int cProfiles;
	std::string nymId ="";// empty nymId means that the default nym must be selected in RoleCredentialScreen

	//Count all profiles in cambrian
	cProfiles=g_oConfiguration.m_arraypaProfiles.GetSize();


	if (cProfiles > 0 ) // There is at least 1 Role created (select current)
	{
	// get the current nym selected
	nymId = pProfile->m_strNymID;

	}
	int pRCS_Operation=0;
	std::string pRCS_nymId_Current=nymId;
	std::string pRCS_nymId_Operation=nymId;
	//OTX::It(pParentWidget)->mc_encrypt_show_dialog(false,true);
	OTX::It(pParentWidget)->mc_show_nym(QString::fromStdString(nymId),pRCS_Operation,pRCS_nymId_Current,pRCS_nymId_Operation);


	// Here, RCS_options holds the values of the nym selected or created (see the meaning of the array in OTX.hpp)

	// After any operation in CREDENTIAL SCREEN
	// Perform the requested operations in RoleCredentials Screen in Cambrian XML

	if (pRCS_Operation == RCS_ACTION_NYM_CREATED)
	{
	// Create a new register in Tprofile
	g_oConfiguration.m_arraypaProfiles.Add(PA_CHILD pProfile);

	std::string RoleName = pOTX->getNym_name(pRCS_nymId_Operation);
	std::string PK=pOTX->getNymPublicKey(pRCS_nymId_Operation);
	pProfile->m_strNameProfile.InitFromStringQ(QString::fromStdString(RoleName));
	pProfile->m_strNymID.InitFromStringQ(QString::fromStdString(pRCS_nymId_Operation));
	pProfile->m_strKeyPublic.InitFromStringQ(QString::fromStdString(PK));
	//Save the new role in xml (force)
	pProfile->m_pConfigurationParent->XmlConfigurationSaveToFile();
	NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(pProfile);
	}
	else if (pRCS_Operation ==RCS_ACTION_NYM_SWITCH) {}
	else if (pRCS_Operation ==RCS_ACTION_NYM_DELETED)  // Delete the selected nym and cambrian role acording to nynm
	{
	// Get All the profiles and select the profile who matches with the deleted nymid
	TProfile ** prgpProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfiles(OUT &cProfiles);
	TProfile * deleteProfile;
	for (int iProfile = 0; iProfile < cProfiles; iProfile++)
	{
	deleteProfile = prgpProfiles[iProfile];
	QString str_NymID =deleteProfile->m_strNymID.ToQString();
	if (pRCS_nymId_Operation.compare(str_NymID.toStdString())==0)
		break; // finish the loop when found  nymID
	}

	if (deleteProfile != NULL)
	{

	if(deleteProfile->m_arraypaAccountsXmpp.GetSize()|deleteProfile->m_arraypaApplications.GetSize())
	{

	std::string nymid_d=OTAPI_Wrap::CreateNym(1024,"","");
	OTAPI_Wrap::SetNym_Name(nymid_d,nymid_d,deleteProfile->m_strNameProfile.ToQString().toStdString());
	deleteProfile->m_strNymID.InitFromStringQ(QString::fromStdString(nymid_d));
	EMessageBoxInformation("Before deleting your " d_sza_profile " '$S', you must manually delete all its accounts.", &deleteProfile->m_strNameProfile);
	} else
	{
	deleteProfile->m_pConfigurationParent->m_arraypaProfiles.DeleteTreeItem(PA_DELETING deleteProfile);

	}
	//Save Changes
	Assert(deleteProfile !=NULL);
	//deleteProfile->m_pConfigurationParent->XmlConfigurationSaveToFile();
	NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(NULL);	// After deleting a profile, display all the remaining profiles

	}

	}


	}


void OTX_WRAP::addOTServerContract(QString Url)
	{


	// --------------------------------
	if (Url.isEmpty())
	{

	std::cout <<"No URL was provided to dowload the contract.";

	return;
	}

	QUrl theURL(Url);

	std::cout << "\nUrl about to download:"+Url.toStdString();
	m_pImgCtrl = new FileDownloader(theURL,this);


	// Download the contract

	//connect(m_pImgCtrl, SIGNAL(downloaded()),SIGNAL(this->SL_DownloadedURL()));
	// connect(&m_pImgCtrl, &FileDownloader::downloaded,this,&OTX_WRAP::SL_DownloadedURL);
	}
bool OTX_WRAP::addOTServerContracts()
	{
	int maxContracts=1;
	// List all available OT server contracts
	const QString availableOTServerContracts[1] =
	{"http://localhost/societyProOT.otc", //for testing propose
	 // the contracts must reside in Sopro OT servers
	};

	//Load all available servers
	for (int i=0; i < maxContracts;i++ )
	{
	this->addOTServerContract(availableOTServerContracts[i]);
	}
	int srvCount=OTAPI_Wrap::GetServerCount();
	std::cout << "\To Log: All contract and server loaded. Number of server loaded: " ;
	std::cout << srvCount;

	}

OTX_WRAP::~OTX_WRAP()
	{
	std::cout << "Destroying otx wrap";
	OTX::It(pParentWidget,true);
	}
#endif


