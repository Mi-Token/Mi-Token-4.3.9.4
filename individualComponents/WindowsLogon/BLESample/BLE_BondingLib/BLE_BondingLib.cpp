#include "BLE_BondingLib.h"
#include "MiTokenBLE_UUIDs.h"

#include "openssl\bn.h"
#include "openssl\ec.h"
#include <Windows.h>
#include <assert.h>
#include <openssl\evp.h>
#include <openssl\ec.h>
#include "openssl\ecdh.h"
#include "openssl\aes.h"


#define printBytesWithInfo(prefix, bytes, length, suffix) printf(prefix); printBytes(bytes, length); printf(suffix);

BLE_BondingLib::BLE_BondingLib()
{
	this->bondData = nullptr;
	this->bondLength = 0;
}

BLE_BondingLib::~BLE_BondingLib()
{
	if(this->bondData != nullptr)
	{
		delete[] this->bondData;
	}
	this->bondData = nullptr;
	this->bondLength = 0;
}


void printBytes(uint8* data, int len)
{
	for(int i = 0 ; i < len ; ++i)
	{
		printf("%02lX ", data[i]);
	}
}
const uint8 VERI_SUCCESS = 0xAA;
const uint8 VERI_FAILURE = 0x55;
const uint8 AES_IV_LEN = 0x10;
const uint8 AES_BLOCK_LENGTH = 0x10;
const uint8 AES_KEY_LENGTH = 0x10;

enum TokenState
{
	TokenState_Init = 0,
	TokenState_PubKeyRdy = 1,
	TokenState_SecRdy = 2,
	TokenState_PendVeri = 3,
	TokenState_Ready = 4,
	TokenState_Error = 5
};

enum CentralState
{
	CentralState_Init = 0,
	CentralState_PubKeyRdy = 1,
	CentralState_SecRdy = 2,
	CentralState_Ready = 3,
	CentralStaet_Error = 4,
};

enum CommandID
{
	CmdID_Add_Bond = 0x01,
	CmdID_Rem_Bond = 0x02,
	CmdID_Erase_All = 0x03,
	CmdID_Ping = 0x04,
};

#define SECP128R1 (706)
#define X9_62_prime_field_NID (406)

const EC_GROUP* GetKeysGroup(const EC_KEY* key)
{
	return EC_KEY_get0_group(key);
}

const bool GroupIsX9_62_prime_field(const EC_GROUP* group)
{
	auto method = EC_GROUP_method_of(group);
	auto fieldType = EC_METHOD_get_field_type(method);

	return (fieldType == X9_62_prime_field_NID);
}
const int BnBytes(BIGNUM* bn)
{
	int bits = BN_num_bits(bn);
	return (bits + 7) / 8;
}

DWORD WINAPI GenerateKey(LPVOID key)
{
	EC_KEY* k = reinterpret_cast<EC_KEY*>(key);
	EC_KEY_generate_key(k);
	return 0;
}

uint8* __stdcall dataAllocator(uint8 requiredLength)
{
	return new uint8[requiredLength];
}

void __stdcall dataDeallocator(uint8* buffer)
{
	delete[] buffer;
}

void reverseArray(uint8* data, int length)
{
	//printBytesWithInfo("Reversing Array \t: " , data, length, "\r\n");
	for(int i = 0 ; i < (length / 2) ; ++i)
	{
		uint8 temp = data[i];
		data[i] = data[length - i - 1];
		data[length - i - 1] = temp;
	}
	//printBytesWithInfo("New Array \t\t: ", data, length, "\r\n");
}

void KDF1_SHA1(uint8* inBuffer, uint8 inLen, uint8*& outBuffer, int& outLen)
{
	auto SHA1 = EVP_sha1();
	auto MDC = EVP_MD_CTX_create();
	EVP_MD_CTX_init(MDC);
	reverseArray(inBuffer, inLen);

	unsigned int len = SHA1->md_size;
	uint8* digest = new byte[len];
	EVP_DigestInit_ex(MDC, SHA1, nullptr);
	EVP_DigestUpdate(MDC, inBuffer, inLen);
	EVP_DigestFinal_ex(MDC, digest, &len);
	outBuffer = digest;
	outLen = len;

	EVP_MD_CTX_destroy(MDC);
}

void* KeyDerivationFunction(const void* in, size_t inlen, void* out, size_t* outlen)
{
	uint8* buffer;
	int bufferLen;
	KDF1_SHA1((uint8*)in, inlen, buffer, bufferLen);
	if(bufferLen > *outlen)
	{
		return nullptr;
	}
	memcpy(out, buffer, bufferLen);
	*outlen = bufferLen;

	delete buffer;
	return out;
}

void computeKey(EC_KEY* key, const EC_POINT* point, uint8* buffer, uint32 bufferLength)
{
	ECDH_compute_key(buffer, bufferLength, point, key, KeyDerivationFunction);

}

bool verifyResponse(uint8* plainTextVerificationBuffer)
{
	uint8 veriBuffer[] = { 0x45, 0x46, 0xfb, 0xb3, 0xf0, 0x74, 0x33, 0x9c, 0xe2, 0xdf, 0x1a, 0x61, 0x9e, 0x18, 0x39, 0x0a};
	return (memcmp(veriBuffer, plainTextVerificationBuffer, sizeof(veriBuffer)) == 0);
}


BLE_BOND_RET BLE_BondingLib::addBondToToken(IMiTokenBLEV2_2* v2_2Interface, IMiTokenBLEConnectionV2_2* v2_2Connection, uint8* sha1Hash)
{
	/*
	printf("Debug find service 0xFFE0\r\n");
	v2_2Interface->EnableNonUniqueAttributeUUIDMode();

	v2_2Connection->SyncScanServiesInRange(BLE_UUID_SERVICE_PUSH_BUTTON, 0xFF, CONN_TIMEOUT_INFINITE);
	uint16 charHandle1 = -1, charHandle2 = -1;
	BLE_CONN_RET r1, r2;
	r1 = v2_2Connection->GetCharacteristicHandleEx(BLE_UUID_SERVICE_PUSH_BUTTON, 0x2902, charHandle1);
	r2 = v2_2Connection->GetCharacteristicHandleEx(BLE_UUID_SERVICE_AUTHENTICATION, 0x2902, charHandle2);

	printf("Results from GetCharHandleEx\r\n\t[1]\r\n\t\tRet : %d\r\n\t\tVal : %d\r\n\t[2]\r\n\t\tRet : %d\r\n\t\tVal : %d\r\n", r1, charHandle1, r2, charHandle2);

	v2_2Connection->SyncScanServicesInRangeOfGUIDService(0xFFC0, 0xFF, CONN_TIMEOUT_INFINITE);
	charHandle1 = charHandle2 = -1;
	r1 = v2_2Connection->SyncGetCharacteristicHandleEx(0xFFC0, 0x2803, charHandle1, CONN_TIMEOUT_INFINITE, true, 0);
	r2 = v2_2Connection->SyncGetCharacteristicHandleEx(0xFFC0, 0x2803, charHandle2, CONN_TIMEOUT_INFINITE, true, 1);

	printf("Results from GetCharHandleEx [GUID Mode]\r\n\t[1]\r\n\t\tRet : %d\r\n\t\tVal : %04lX\r\n\t[2]\r\n\t\tRet : %d\r\n\t\tVal : %04lX\r\n", r1, charHandle1, r2, charHandle2);
	*/

	BLE_BOND_RET returnCode = BOND_RET_FAILED;
	auto ctx = BN_CTX_new();
	auto myKey = EC_KEY_new_by_curve_name(SECP128R1);
	auto tokenKey = EC_KEY_new_by_curve_name(SECP128R1);
	auto myPubKeyBnX = BN_new();
	auto myPubKeyBnY = BN_new();
	auto tokenPubKeyBnX = BN_new();
	auto tokenPubKeyBnY = BN_new();
	uint8* sharedSecTruncated = new byte[AES_KEY_LENGTH];
	uint8* myPubKeyBufferWhole = new uint8[32];

	try
	{
		auto tokenPubKey = EC_POINT_new(GetKeysGroup(tokenKey));
		printf("Sync : Scan Services in range of SERVICE_SECURE_BOND\r\n");
		if(v2_2Connection->SyncScanServiesInRange(BLE_UUID_SERVICE_SECURE_BOND, 0xFF, 5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}
		uint8 cmdAddBond[17] = {CmdID_Add_Bond, 0x00};

		mac_address addr;
		v2_2Connection->GetAddress(addr);
		for(int i = 0 ; i < 16 ; i += 6)
		{
			for(int j = 0 ; (j < 6) && ((i + j) < 16) ; ++j)
			{
				cmdAddBond[i + j + 1] = addr.addr[j];
			}
		}

		cmdAddBond[16]++;
		printBytesWithInfo("Command Bytes : ", cmdAddBond, sizeof(cmdAddBond), "\r\n");

		printf("Sync: Set Attribute BOND_CONFIG\r\n");
		if(v2_2Connection->SyncSetAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_CONFIG, sizeof(cmdAddBond), cmdAddBond, 5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		// Check if previous bond is set or maximum bonds have been reached
		if (v2_2Connection->GetLastError() == 0x85) //ATT_ERR_SECUREBOND_CFG_ADD_FAIL
		{
			printf("Previous bond set or maximum bonds reached\r\n");
			throw BOND_RET_ERR_PREVBOND_MAXBOND;
		}

		HANDLE thread = CreateThread(nullptr, 0, GenerateKey, myKey, 0, nullptr);
		WaitForSingleObject(thread, INFINITE);


		uint8 neededState[] = {TokenState_PubKeyRdy, TokenState_SecRdy};
		neededState[0] = TokenState_PubKeyRdy;
		neededState[1] = TokenState_SecRdy;

		printf("Sync : Waiting for State of TokenState_PubKeyReady or TokenState_SecReady\r\n");

		if(v2_2Connection->SyncWaitForState(BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, false, neededState, 2, 1, dataAllocator, dataDeallocator, 5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		uint8 tokenPubKeyBufferLen = 0;
		uint8* tokenPubKeyBuffer = nullptr;

		printf("Sync : Reading Public Key\r\n");
		if(v2_2Connection->SyncReadAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_PUBLIC_KEY, true, tokenPubKeyBufferLen, tokenPubKeyBuffer, dataAllocator, dataDeallocator, 5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		printBytesWithInfo("Public Key Data is : ", tokenPubKeyBuffer, tokenPubKeyBufferLen, "\r\n");
		uint8 *tokenPubKeyBufferX, *tokenPubKeyBufferY;
		tokenPubKeyBufferX = tokenPubKeyBuffer;
		tokenPubKeyBufferY = (tokenPubKeyBuffer + 16);
		reverseArray(tokenPubKeyBufferX, 16);
		reverseArray(tokenPubKeyBufferY, 16);
		BN_bin2bn(tokenPubKeyBufferX, 16, tokenPubKeyBnX);
		BN_bin2bn(tokenPubKeyBufferY, 16, tokenPubKeyBnY);
		if(GroupIsX9_62_prime_field(GetKeysGroup(tokenKey)))
		{
			EC_POINT_set_affine_coordinates_GFp(GetKeysGroup(tokenKey), tokenPubKey, tokenPubKeyBnX, tokenPubKeyBnY, ctx);
		}
		else
		{
			EC_POINT_set_affine_coordinates_GF2m(GetKeysGroup(tokenKey), tokenPubKey, tokenPubKeyBnX, tokenPubKeyBnY, ctx);
		}
		EC_KEY_set_public_key(tokenKey, tokenPubKey);

		auto myKeyPoint = EC_KEY_get0_public_key(myKey);
		if(GroupIsX9_62_prime_field(GetKeysGroup(myKey)))
		{
			EC_POINT_get_affine_coordinates_GFp(GetKeysGroup(myKey), myKeyPoint, myPubKeyBnX, myPubKeyBnY, ctx);
		}
		else
		{
			EC_POINT_get_affine_coordinates_GF2m(GetKeysGroup(myKey), myKeyPoint, myPubKeyBnX, myPubKeyBnY, ctx);
		}
		//EC_KEY_set_public_key(myKey, myKeyPoint);

		uint8 *myPubKeyBufferX, *myPubKeyBufferY;

		assert(BnBytes(myPubKeyBnX) == 16);
		assert(BnBytes(myPubKeyBnY) == 16);
	
		myPubKeyBufferX = myPubKeyBufferWhole;
		myPubKeyBufferY = myPubKeyBufferWhole + 16;

		BN_bn2bin(myPubKeyBnX, myPubKeyBufferX);
		BN_bn2bin(myPubKeyBnY, myPubKeyBufferY);

		reverseArray(myPubKeyBufferX, 16);
		reverseArray(myPubKeyBufferY, 16);

		printBytesWithInfo("My Public Key is : ", myPubKeyBufferWhole, 32, "\r\n");
		printf("Sync : Setting BOND_PUBLIC_KEY\r\n");
		if(v2_2Connection->SyncSetAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_PUBLIC_KEY, 32, myPubKeyBufferWhole, 5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		uint8 centralStateNowPubKeyReady[] = {CentralState_PubKeyRdy};
		printf("Sync : Setting CentralState_PubKeyReady\r\n");
		if(v2_2Connection->SyncSetAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, sizeof(centralStateNowPubKeyReady), centralStateNowPubKeyReady, 5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		uint8 sharedSec[32] = {0x00};

		printf("Computing Key... : ");
		computeKey(myKey, tokenPubKey, sharedSec, 32);
		printf("Done.\r\nWaiting for Disconnect... :");
		if(v2_2Connection->SyncWaitForDisconnection(5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			// Assume that token has already restarted
			printf("Did not receive disconnect");
		}
		printf("Done.\r\nReconnecting... :");
		if(v2_2Connection->IsConnected() == BLE_CONN_ERR_NOT_CONNECTED)
		{
			if(v2_2Connection->SyncWaitForConnection(5000) == BLE_CONN_ERR_SYNC_TIMEOUT)
			{
				if (BLE_CONN_ERR_CONNECTION_TIMED_OUT == v2_2Connection->TimeoutRequest())
				{
					//we have successfully requested the timeout
					v2_2Connection->Disconnect();
				}

				// Refresh connection
				REQUEST_ID req;
				v2_2Interface->StartProfile(addr, req);
			}

			// TODO: Should wait for procedure completed event here due to automatic scanning
			Sleep(2000);

			printf("Done\r\n");
		}

		neededState[0] = TokenState_SecRdy;
		printf("Sync : Waiting for TokenState_SecReady\r\n");
		if(v2_2Connection->SyncWaitForState(BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, false, neededState, 1, 1, dataAllocator, dataDeallocator, 5000) ==  BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		uint8 centralStateNowSecReady[] = {CentralState_SecRdy};
		printf("Sync : Setting state CentralState_SecReady\r\n");
		if(v2_2Connection->SyncSetAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, sizeof(centralStateNowSecReady), centralStateNowSecReady,5000) ==  BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		neededState[0] = TokenState_PendVeri;
		printf("Sync : Waiting for TokenState_PendingVerification\r\n");
		if(v2_2Connection->SyncWaitForState(BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, false, neededState, 1, 1, dataAllocator, dataDeallocator, 5000) ==  BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		uint8 *veriResponseBuffer = nullptr;
		uint8 veriResponseLength = 0;
		printf("Sync : Reading verification buffer\r\n");
		if(v2_2Connection->SyncReadAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_VERIFICATION_BUFFER, true, veriResponseLength, veriResponseBuffer, dataAllocator, dataDeallocator, 5000) ==  BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}
		printBytesWithInfo("Verification Buffer : ", veriResponseBuffer, veriResponseLength, "\r\n");

		uint8 * iv = veriResponseBuffer;
		uint8 aesBlockCount = *(veriResponseBuffer + AES_IV_LEN);
		uint8* cipherText = (veriResponseBuffer + AES_IV_LEN + 1);
		uint8 cipherTextLen = AES_BLOCK_LENGTH * aesBlockCount;
	
		memcpy(sharedSecTruncated, sharedSec, AES_KEY_LENGTH);
		uint8* plainText = new byte[cipherTextLen];

		printf("AES Info : \r\n\t");
		printBytesWithInfo("Key : ", sharedSecTruncated, AES_KEY_LENGTH, "\r\n\t");
		printBytesWithInfo("IV : ", iv, AES_IV_LEN, "\r\n\t");
		printf("AES Block Count : %d\r\n\t", aesBlockCount);
		printBytesWithInfo("Cipher Text : ", cipherText, cipherTextLen, "\r\n\t");
		printf("Calculating Padded Block Data...");

		AES_KEY aes_key;
		AES_set_decrypt_key(sharedSecTruncated, 128, &aes_key);
		AES_set_encrypt_key(sharedSecTruncated, 128, &aes_key);
		/*
		uint8 backupDecIV[AES_IV_LEN];
		memcpy(backupDecIV, iv, AES_IV_LEN);
		uint8 block[1] = {0};
		uint8 paddedBlock[AES_BLOCK_LENGTH] = {0};
		AES_cbc_encrypt(block, paddedBlock, 0, &aes_key, backupDecIV, 1);
		printBytesWithInfo("Done\r\n\tPadded Block : ", paddedBlock, AES_BLOCK_LENGTH, "\r\n\t");
		uint8* newCipher = new uint8[cipherTextLen + AES_BLOCK_LENGTH];
		memcpy(newCipher, cipherText, cipherTextLen);
		memcpy(newCipher + cipherTextLen, paddedBlock, AES_BLOCK_LENGTH);
		printBytesWithInfo("Entire Block : ", newCipher, cipherTextLen + AES_BLOCK_LENGTH, "\r\n\tDecrypting...");
		*/
		//AES_cbc_encrypt(cipherText, plainText, cipherTextLen, &aes_key, iv, 0);

		auto evp_ctx = EVP_CIPHER_CTX_new();
		EVP_CIPHER_CTX_init(evp_ctx);
		EVP_DecryptInit_ex(evp_ctx, EVP_aes_128_cbc(), nullptr, sharedSecTruncated, iv);
		int outlen = AES_BLOCK_LENGTH;
		EVP_DecryptUpdate(evp_ctx, plainText, &outlen, cipherText, cipherTextLen);
		printf("Done\r\n\t");
		printBytesWithInfo("PlainText : ", plainText, cipherTextLen, "\r\n");

		uint8 plainTextTrunc[AES_BLOCK_LENGTH];
		memcpy(plainTextTrunc, plainText, AES_BLOCK_LENGTH);
		bool isVeriSuccess = verifyResponse(plainTextTrunc);

		EVP_DecryptFinal_ex(evp_ctx, plainText, &outlen);
		delete[] plainText;
		printf("Verification was : %s\r\n", isVeriSuccess ? "SUCCESS" : "FAILURE");

		uint8* encryptedBuffer = nullptr;
		uint8 plainTextVeriResponse[] = { isVeriSuccess ? VERI_SUCCESS : VERI_FAILURE};
		uint8 outputBuffer[AES_BLOCK_LENGTH];

		uint8 newIV[AES_IV_LEN];
		srand(time(nullptr));

		for(int i = 0 ; i < AES_IV_LEN; ++i)
		{
			newIV[i] = rand();
		}

		uint8 backupIV[AES_IV_LEN];
		memcpy(backupIV, newIV, AES_IV_LEN);

		printf("AES Info : \r\n\t");
		printBytesWithInfo("Key : ", sharedSecTruncated, AES_KEY_LENGTH, "\r\n\t");
		printBytesWithInfo("IV : ", newIV, AES_IV_LEN, "\r\n\t");
		printf("AES Block Count : %d\r\n\t", 1);
		printBytesWithInfo("Plain Text : ", plainTextVeriResponse, 1, "\r\n\t");
		printf("Encrypting...");
		AES_cbc_encrypt(plainTextVeriResponse, outputBuffer, 1, &aes_key, newIV, 1);
		printf("Done\r\n\t");
		printBytesWithInfo("CipherText : ", outputBuffer, AES_BLOCK_LENGTH, "\r\n");

		uint8 encBlockCount = 1;
		uint8 encBuffer[AES_BLOCK_LENGTH + AES_IV_LEN + 1];
		memcpy(encBuffer, backupIV, AES_IV_LEN);
		encBuffer[AES_IV_LEN] = 1;
		memcpy(encBuffer + AES_IV_LEN + 1, outputBuffer, AES_BLOCK_LENGTH);

		printBytesWithInfo("Sync : Set BOND_VERIFICATION : ", encBuffer, sizeof(encBuffer), "\r\n");
		if(v2_2Connection->SyncSetAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_VERIFICATION_BUFFER, sizeof(encBuffer), encBuffer, 5000) ==  BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		uint8 centralStateNowReady[] = {CentralState_Ready};
		printf("Sync : Set Status CentralState_Ready\r\n");
		if(v2_2Connection->SyncSetAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, sizeof(centralStateNowReady), centralStateNowReady, 5000) ==  BLE_CONN_ERR_SYNC_TIMEOUT)
		{
			throw BOND_RET_ERR_TIMEOUT;
		}

		printBytesWithInfo("Shared Sec Is : ", sharedSec, 20, "\r\n");
		if(isVeriSuccess)
		{
			bondData = new byte[32];
			bondLength = 32;
			memcpy(bondData, sharedSec, bondLength);
			returnCode = BOND_RET_SUCCESS;
		}
		else
		{
			returnCode = BOND_RET_FAILED;
		}
	}
	catch(BLE_BOND_RET e)
	{
		returnCode = e;
		printf("Exiting with Error : %d\r\n\t", e);
	}

	BN_CTX_free(ctx);
	EC_KEY_free(myKey);
	EC_KEY_free(tokenKey);
	BN_clear_free(myPubKeyBnX);
	BN_clear_free(myPubKeyBnY);
	BN_clear_free(tokenPubKeyBnX);
	BN_clear_free(tokenPubKeyBnY);
	delete[] sharedSecTruncated;
	delete[] myPubKeyBufferWhole;
	return returnCode;
}

bool BLE_BondingLib::removeAllBondsFromToken(IMiTokenBLEV2_2* v2_2Interface, IMiTokenBLEConnectionV2_2* v2_2Connection)
{
	if(v2_2Connection->SyncScanServiesInRange(BLE_UUID_SERVICE_SECURE_BOND, 0xFFFF, 1000) == BLE_CONN_ERR_SYNC_TIMEOUT)
	{
		return false;
	}
	byte commandData[] = { (byte)CmdID_Erase_All };
	if(v2_2Connection->SyncSetAttribute(BLE_UUID_ATTRIBUTE_SECURE_BOND_CONFIG, sizeof(commandData), commandData, 1000) == BLE_CONN_ERR_SYNC_TIMEOUT)
	{
		return false;
	}

	return true;
}