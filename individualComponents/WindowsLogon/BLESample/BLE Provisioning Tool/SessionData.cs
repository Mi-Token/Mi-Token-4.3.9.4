using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;
using System.IO;

namespace BLE_Provisioning_Tool
{
    class SessionData
    {
        // Constants
        static public readonly int SESSION_ID_LEN = 16;
        static public readonly int SESSION_NONCE_LEN = 16;
        static public readonly int SESSION_MAC_LEN = 20;
        static public readonly int SESSION_SESSIONKEY_LEN = 20;
        static public readonly int SESSION_DERIVEDKEY_LEN = 16;
        static public readonly string SESSION_TOKEN_MAC_DATA = "TOKEN\0"; // Include Null terminator
        static public readonly string SESSION_PHONE_MAC_DATA = "PHONE\0"; // Include Null terminator

        // Enumerations
        public enum EnrcryptionKeyId : byte
        { 
		    SESSION_AUTHEN_KEY_ID = 0x01,
		    SESSION_ENCR_CMD_KEY_ID,
		    SESSION_ENCR_DATA_KEY_ID,
		    SESSION_ENCR_PIN_KEY_ID
	    };

        // Properties
        public byte[] DeviceNonce { get; private set; }
        public byte[] DeviceMac { get; private set; }
        public byte[] SelfNonce { get; private set; }
        public byte[] SelfMac { get; private set; }
        public byte[] AuthenKey { get; private set; }
        public byte[] EncrCmdKey { get; private set; }
        public byte[] EncrDataKey { get; private set; }
        public byte[] EncrPinKey { get; private set; }
        public bool IsAuthenticated { get; set; }

        // Constructors
        public SessionData()
        {
            DeviceNonce = new byte[SESSION_NONCE_LEN];
            DeviceMac = new byte[SESSION_MAC_LEN];
            SelfNonce = new byte[SESSION_NONCE_LEN];
            SelfMac = new byte[SESSION_MAC_LEN];
            AuthenKey = new byte[SESSION_DERIVEDKEY_LEN];
            EncrCmdKey = new byte[SESSION_DERIVEDKEY_LEN];
            EncrDataKey = new byte[SESSION_DERIVEDKEY_LEN];
            EncrPinKey = new byte[SESSION_DERIVEDKEY_LEN];
        }

        // Functions
        public void Init()
        {
            // Create self nonce
            Helper.CreateRandomBytes(SelfNonce);
            IsAuthenticated = false;
        }

        // Assumes Device Nonce has already been set
        public bool CreateKeys(ref SharedSecret sharedSecretKey)
        {
            byte[] nullIv = new byte[AES.IV_LENGTH];

            // Create session key data from Self Nonce appended with Device Nonce
            byte[] sessionKeyData = Helper.BuildByte(new byte[][]{
                SelfNonce,
                DeviceNonce
            });

            // Create session key used for creating derived keys
            byte[] sessionKey = new byte[SESSION_SESSIONKEY_LEN];             
            using (HMACSHA1 hmac = new HMACSHA1(sharedSecretKey.Value))
            {
                sessionKey = hmac.ComputeHash(sessionKeyData);
            }

            try
            {
                byte[] tempBuff;
                byte[] derivedKeyData = new byte[AES.BLOCK_LENGTH];
                byte[] truncatedSessionKey = sessionKey.Take(AES.KEY_LENGTH).ToArray();

                // Create authentication key from session key                
                derivedKeyData[0] = (byte)EnrcryptionKeyId.SESSION_AUTHEN_KEY_ID;                
                AES.Encrypt(ref nullIv, ref truncatedSessionKey, ref derivedKeyData, out tempBuff, PaddingMode.None);
                AuthenKey = tempBuff;

                // Create encryption command key from session key  
                derivedKeyData[0] = (byte)EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID;
                AES.Encrypt(ref nullIv, ref truncatedSessionKey, ref derivedKeyData, out tempBuff, PaddingMode.None);
                EncrCmdKey = tempBuff;

                // Create encryption data key from session key  
                derivedKeyData[0] = (byte)EnrcryptionKeyId.SESSION_ENCR_DATA_KEY_ID;
                AES.Encrypt(ref nullIv, ref truncatedSessionKey, ref derivedKeyData, out tempBuff, PaddingMode.None);
                EncrDataKey = tempBuff;

                // Create encryption PIN key
                derivedKeyData[0] = (byte)EnrcryptionKeyId.SESSION_ENCR_PIN_KEY_ID;
                AES.Encrypt(ref nullIv, ref truncatedSessionKey, ref derivedKeyData, out tempBuff, PaddingMode.None);
                EncrPinKey = tempBuff;

                return true;
            }
            catch
            {
                return false;
            }            
        }

        public void CreateSelfMac()
        {
            byte[] data = Encoding.UTF8.GetBytes(SESSION_PHONE_MAC_DATA);
            using (HMACSHA1 hmac = new HMACSHA1(AuthenKey))
            {
                SelfMac = hmac.ComputeHash(data);
            }
        }

        public bool VerifyDevMAC()
        {
            // Create intended device MAC
            byte[] data = Encoding.UTF8.GetBytes(SESSION_TOKEN_MAC_DATA);
            byte[] intendedDevMac = new byte[SESSION_MAC_LEN];
            using (HMACSHA1 hmac = new HMACSHA1(AuthenKey))
            {
                intendedDevMac = hmac.ComputeHash(data);
            }

            // Verify received device MAC against that which was created
            return Helper.ArraysEqual(intendedDevMac, DeviceMac);
        }

        public void CreateEncryptedBuffer(EnrcryptionKeyId keyId, ref byte[] plaintext, out byte[] encryptedBuffer)
        {            
            byte[] aeskey = getEncryptionKey(keyId); // Select session data key
            AES.CreateEncryptedBuffer(ref aeskey, ref plaintext, out encryptedBuffer);
        }

        private byte[] getEncryptionKey(EnrcryptionKeyId keyId)
        {
            switch (keyId)
            {
                case EnrcryptionKeyId.SESSION_AUTHEN_KEY_ID:
                    return AuthenKey;
                case EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID:
                    return EncrCmdKey;
                case EnrcryptionKeyId.SESSION_ENCR_DATA_KEY_ID:
                    return EncrDataKey;
                case  EnrcryptionKeyId.SESSION_ENCR_PIN_KEY_ID:
                    return EncrPinKey;
                default:
                    return AuthenKey; // Just use authentication key as default
            }
        }
    }

    class AES
    {
        static public readonly int IV_LENGTH = 16;
        static public readonly int KEY_LENGTH = 16;
        static public readonly int BLOCK_LENGTH = 16;

        static public void Encrypt(ref byte[] iv, ref byte[] key, ref byte[] plaintext, out byte[] ciphertext, PaddingMode paddingMode)
        {
            if (plaintext == null || plaintext.Length <= 0) // plaintext doesn't have to block size
                throw new ArgumentException("PlainText");
            if (key == null || key.Length <= 0 || (key.Length != AES.KEY_LENGTH))
                throw new ArgumentException("Key");
            if (iv == null || iv.Length <= 0 || iv.Length != AES.IV_LENGTH)
                throw new ArgumentException("IV");

            using (AesManaged aesAlg = new AesManaged())
            {
                aesAlg.KeySize = KEY_LENGTH * 8;
                aesAlg.BlockSize = BLOCK_LENGTH * 8;

                aesAlg.Mode = CipherMode.CBC;
                aesAlg.IV = iv;
                aesAlg.Key = key;
                if (plaintext.Length % AES.BLOCK_LENGTH == 0)
                {
                    aesAlg.Padding = PaddingMode.None; // Since Zero Padding Block not supported on token
                }
                else
                {
                    aesAlg.Padding = paddingMode;
                }                

                // Create a encryptor to perform the stream transform.
                ICryptoTransform encryptor = aesAlg.CreateEncryptor(aesAlg.Key, aesAlg.IV);

                // Create the streams used for encryption. 
                using (MemoryStream msEncrypt = new MemoryStream())
                {
                    using (CryptoStream csEncrypt = new CryptoStream(msEncrypt, encryptor, CryptoStreamMode.Write))
                    {
                        csEncrypt.Write(plaintext, 0, plaintext.Length);
                        csEncrypt.FlushFinalBlock();

                        // Save encrypted bytes from the memory stream. 
                        ciphertext = msEncrypt.ToArray();
                    }
                }
            }
        }

        static public void Decrypt(ref byte[] iv, ref byte[] key, ref byte[] ciphertext, out byte[] plaintext, PaddingMode paddingMode)
        {
            if (ciphertext == null || ciphertext.Length <= 0 || (ciphertext.Length % BLOCK_LENGTH != 0))
                throw new ArgumentException("CipherText");
            if (key == null || key.Length <= 0 || (key.Length != AES.KEY_LENGTH))
                throw new ArgumentException("Key");
            if (iv == null || iv.Length <= 0 || iv.Length != AES.IV_LENGTH)
                throw new ArgumentException("IV");

            using (AesManaged aesAlg = new AesManaged())
            {
                aesAlg.KeySize = KEY_LENGTH * 8;
                aesAlg.BlockSize = BLOCK_LENGTH * 8;

                aesAlg.Mode = CipherMode.CBC;
                aesAlg.IV = iv;
                aesAlg.Key = key;
                aesAlg.Padding = paddingMode;

                // Create a decryptor to perform the stream transform
                ICryptoTransform decryptor = aesAlg.CreateDecryptor(aesAlg.Key, aesAlg.IV);

                // Create the streams used for encryption. 
                using (MemoryStream msDecrypt = new MemoryStream(ciphertext))
                {
                    using (CryptoStream csDecrypt = new CryptoStream(msDecrypt, decryptor, CryptoStreamMode.Read))
                    {
                        plaintext = new byte[ciphertext.Length]; // Same size as PlainText
                        csDecrypt.Read(plaintext, 0, plaintext.Length);
                    }
                }
            }
        }

        static public void CreateEncryptedBuffer(ref byte[] key, ref byte[] plaintext, out byte[] encryptedBuffer)
        {
            // Create IV
            byte[] iv = new byte[AES.IV_LENGTH];
            Helper.CreateRandomBytes(iv);

            // Select key and encrypt plaintext
            byte[] ciphertext;
            AES.Encrypt(ref iv, ref key, ref plaintext, out ciphertext, PaddingMode.PKCS7);

            // Populate encrypted buffer
            byte blockCount = (byte)Math.Ceiling((float)ciphertext.Length / AES.BLOCK_LENGTH);
            encryptedBuffer = Helper.BuildByte(new byte[][]{
                iv,                         // Add IV
                new byte[] { blockCount },  // Add Block Count
                ciphertext,                 // Add Ciphertext
            });
        }
    }
}
