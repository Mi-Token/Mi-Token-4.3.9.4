using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BLE_API;
using OpenSSL.Crypto;
using OpenSSL.Crypto.EC;
using OpenSSL.Core;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;

namespace BLE_Provisioning_Tool
{
    class SecureBondService
    {
        static public readonly UInt16 SERV_UUID = 0xA200; // Secure Bond Service
        static public readonly UInt16 ATT_STATUS_UUID = 0xA201; // Status
        static public readonly UInt16 ATT_PUBKEY_UUID = 0xA202; // Public Key
        static public readonly UInt16 ATT_VERIBUFF_UUID = 0xA203; // Verification Buffer
        static public readonly UInt16 ATT_CFG_UUID = 0xA204; // Configuration
        static public readonly UInt16 ATT_ENCRCFG_UUID = 0xA205; // Encrypted Configuration

        //TODO replace this with length of OpenSSL Public Key length.. change to readonly
        static public readonly int PUBLIC_KEY_LENGTH = Constants.NUM_ECC_DIGITS * 2;
        static public readonly int VERIFICATION_RESPONSE_LENGTH = AES.IV_LENGTH + 1 + AES.BLOCK_LENGTH;

        static private byte[] VERI_CODE = { 0x45, 0x46, 0xfb, 0xb3, 0xf0, 0x74, 0x33, 0x9c, 0xe2, 0xdf, 0x1a, 0x61, 0x9e, 0x18, 0x39, 0x0a};

        // Verification Result Codes
        static public readonly byte VERI_SUCCESS = 0xaa;
        static public readonly byte VERI_FAIL = 0x55;

        public enum TokenState
	    {
	      INIT = 0,                    // Initial State - Creating pair of keys
	      PUBKEYRDY,                   // Public Key Ready
	      SECRETRDY,                   // Secret Ready  
	      PNDVERI,                     // Pending Verification
	      RDY,                         // Device Ready / Key Exchange successful
	      ERROR                        // Error occurred - invalid state
	    };

        public enum CentralState
	    {
	      INIT = 0,                    // Initial State - Creating pair of keys
	      PUBKEYRDY,                   // Public Key Ready
	      SECRETRDY,                   // Secret Ready
	      RDY,							// Device Ready / Verification complete
	      ERROR                        //!< Error occurred - invalid state
	    };

        public enum CmdId
        {
            ADD_BOND = 0x01,
            RMV_BOND = 0x02, // Note: This command is now Deprecated, please use EncrCmdId.RMV_BOND
            ERASE_ALL = 0x03,
            PING = 0x04,
            PING_V2 = 0x05,
            CHECK_BOND = 0x06
        };

        public enum EncrCmdId
        {
            RMV_BOND = 0x01,
            UPDATE_PIN = 0x02,
        };

        public class PingBurstCfg
        {
            private static readonly int TOTAL_PING_MAX = 3;
            public UInt16 DurationMs = 0;
            public UInt16 IntervalMs = 0;
            private List<UInt16> FrequenciesHz;

            public PingBurstCfg()
            {
                FrequenciesHz = new List<UInt16>(TOTAL_PING_MAX);
            }

            public bool AddPing(UInt16 freq)
            {
                if (FrequenciesHz.Count < TOTAL_PING_MAX)
                {
                    FrequenciesHz.Add(freq);
                    return true;
                }
                return false;
            }

            public byte[] Serialize()
            {
                List<byte> freqBytes = new List<byte>();
                foreach (UInt16 freq in FrequenciesHz)
                {
                    foreach (byte b in BitConverter.GetBytes(Convert.ToUInt16(freq)))
                    {
                        freqBytes.Add(b);
                    }
                }
                
                return Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)FrequenciesHz.Count },
                        BitConverter.GetBytes(Convert.ToUInt16(DurationMs)),
                        BitConverter.GetBytes(Convert.ToUInt16(IntervalMs)),
                        freqBytes.ToArray()
                    });
            }
        }

        static public async Task<bool> InitiatePing(BLE.V2_2.Device connectedDevice)
        {
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_CFG_UUID, new byte[] { (byte)CmdId.PING });
        }

        static public async Task<bool> InitiatePingV2(BLE.V2_2.Device connectedDevice, PingBurstCfg data)
        {
            byte[] b = Helper.BuildByte(new byte[][] { 
                new byte[] { (byte)CmdId.PING_V2 }, 
                data.Serialize() 
            });
            
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_CFG_UUID, b);
        }

        static public byte[] KDF1_SHA1(byte[] msg)
        {
            using (MessageDigestContext mdc = new MessageDigestContext(MessageDigest.SHA1))
            {
                Array.Reverse(msg); // Convert from big to little endian before doing 
                return mdc.Digest(msg);
            }
        }

        static public bool VerifyResponse(ref byte[] veriBuff)
        {
            return Helper.ArraysEqual(veriBuff, VERI_CODE);
        }

        static public async Task<bool> CreateBond(BLE.V2_2.Device connectedDevice, DeviceAddress devAddress, SharedSecret sharedSecret, IProgress<int> progressBarStatus, ManualResetEventSlim procedureCompletedEvent)
        {
#if DEBUG_DIAGNOSTICS
            Stopwatch sw = new Stopwatch();
            sw.Start();
#endif

            BigNumber.Context ctx = new BigNumber.Context();
            Key myKey = Key.FromCurveName(Objects.NID.secp128r1);
            Key tokenKey = Key.FromCurveName(Objects.NID.secp128r1);
            BigNumber myPubKeyBnX = new BigNumber();
            BigNumber myPubKeyBnY = new BigNumber();
            BigNumber tokenPubKeyBnX = new BigNumber();
            BigNumber tokenPubKeyBnY = new BigNumber();
            OpenSSL.Crypto.EC.Point tokenPubKey = new OpenSSL.Crypto.EC.Point(tokenKey.Group);

            try
            {
                int progressVal = 1;
                int totalProgressVal = 11;

                // Ensure Secure Bond Service Exists
                await BLE_AdvFunctions.FindServiceAttributes(connectedDevice, SecureBondService.SERV_UUID);

                //*** Initiate Bond with Token
                Debug.WriteLine(String.Format("{0}\tCreateBond:Initiate Bond with Token", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                // Create data to send i.e. CMD + DEVICE_ID                        
                byte[] cmdData = Helper.BuildByte(new byte[][] {
                    new byte[] { (byte)SecureBondService.CmdId.ADD_BOND },
                    Helper.CreateDeviceID(devAddress)
                });                

                // Send command data
                await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_CFG_UUID, cmdData);

                //*** Create key pair
                Debug.WriteLine(String.Format("{0}\tCreateBond:Creating key pair", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                // Create as task as generating key is long enough to show as blocking the UI thread
                Task t = Task.Factory.StartNew(() =>
                {
                    myKey.GenerateKey();
                });
                while (!t.Wait(15))
                    Application.DoEvents();

                //*** Read Token Status for public key ready state
                Debug.WriteLine(String.Format("{0}\tCreateBond:Read Token Status for public key ready state", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.ReadRequestUntilEqual(connectedDevice, SecureBondService.ATT_STATUS_UUID, new byte[] { (byte)SecureBondService.TokenState.PUBKEYRDY }, new byte[] { (byte)SecureBondService.TokenState.SECRETRDY });

                //*** Read Token public key
                Debug.WriteLine(String.Format("{0}\tCreateBond:Read Token public key", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                byte[] tokenPublicKey;
                {
                    var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, SecureBondService.ATT_PUBKEY_UUID, SecureBondService.PUBLIC_KEY_LENGTH);
                    if (result.Item1)
                    {
                        tokenPublicKey = result.Item2.ToArray();
                    }
                    else
                    {
                        return false;
                    }
                }
                byte[] tokenPublicKeyXBytes = tokenPublicKey.Take(16).ToArray();
                byte[] tokenPublicKeyYBytes = tokenPublicKey.Skip(16).Take(16).ToArray();
                Array.Reverse(tokenPublicKeyXBytes); // Convcert to big endian for Open SSL
                Array.Reverse(tokenPublicKeyYBytes);
                tokenPubKeyBnX = BigNumber.FromArray(tokenPublicKeyXBytes);
                tokenPubKeyBnY = BigNumber.FromArray(tokenPublicKeyYBytes);
                tokenPubKey.SetAffineCoordinates(tokenPubKeyBnX, tokenPubKeyBnY, ctx);
                tokenKey.SetPublicKey(tokenPubKey);

                //*** Write Public key to Token
                Debug.WriteLine(String.Format("{0}\tCreateBond:Write Public key to Token", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                // Retrieve Public Key coordinates                    
                myKey.PublicKey.GetAffineCoordinates(myPubKeyBnX, myPubKeyBnY, ctx);
                byte[] myPubKeyX = new byte[myPubKeyBnX.Bytes];
                byte[] myPubKeyY = new byte[myPubKeyBnY.Bytes];
                myPubKeyBnX.ToBytes(myPubKeyX);
                myPubKeyBnY.ToBytes(myPubKeyY);

                // Create public key buffer i.e myPubKeyX + myPubKeyY
                byte[] myPubKeyBuff = new byte[myPubKeyX.Length + myPubKeyY.Length];
                Array.Reverse(myPubKeyX); // Change buffers to little endian
                Array.Reverse(myPubKeyY); // Change buffers to little endian
                Buffer.BlockCopy(myPubKeyX, 0, myPubKeyBuff, 0, myPubKeyX.Length);
                Buffer.BlockCopy(myPubKeyY, 0, myPubKeyBuff, myPubKeyX.Length, myPubKeyY.Length);

                // Write public key to Central device
                await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_PUBKEY_UUID, myPubKeyBuff);

                // Write Status update to Central device
                await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_STATUS_UUID, new byte[] { (byte)SecureBondService.CentralState.PUBKEYRDY });

                //*** Generate shared secret while waiting for token to generate secret
                Debug.WriteLine(String.Format("{0}\tCreateBond:Generate shared secret while waiting for token to generate secret", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await Task.Run(() =>
                {
                    myKey.ComputeKey(tokenKey, sharedSecret.Value, SecureBondService.KDF1_SHA1);
                });

                //*** Continue to wait for token to generate secret (Signified by a disconnection)
                Debug.WriteLine(String.Format("{0}\tCreateBond:Waiting for disconnect", DateTime.Now));
                if (!await BLE_AdvFunctions.WaitForDisconnection(connectedDevice, BLE_AdvFunctions.PROCESSING_TIMEOUT_LONG_MS))
                {
                    Debug.WriteLine(String.Format("{0}\tCreateBond:Did not receive disconnect", DateTime.Now));
                }

                //*** Attempt to reconnect to token if required
                Debug.WriteLine(String.Format("{0}\tCreateBond:Attempt to reconnect to token", DateTime.Now));
                if (!await BLE_AdvFunctions.CheckConnection(connectedDevice, procedureCompletedEvent))
                {
                    await BLE_AdvFunctions.RefreshConnection(connectedDevice);
                }

                //*** Once connected, ensure token's secret ready
                Debug.WriteLine(String.Format("{0}\tCreateBond:Check token's secret is ready", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.ReadRequestUntilEqual(connectedDevice, SecureBondService.ATT_STATUS_UUID, new byte[] { (byte)SecureBondService.TokenState.SECRETRDY });

                //*** Update token that Cenetral device secret is ready
                // Write Status update to Central device
                Debug.WriteLine(String.Format("{0}\tCreateBond:Update token that central device's secret is ready", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_STATUS_UUID, new byte[] { (byte)SecureBondService.CentralState.SECRETRDY });

                //*** Wait for ready for verification pending state
                Debug.WriteLine(String.Format("{0}\tCreateBond:Check token's is waiting for verification", DateTime.Now));
                await BLE_AdvFunctions.ReadRequestUntilEqual(connectedDevice, SecureBondService.ATT_STATUS_UUID, new byte[] { (byte)SecureBondService.TokenState.PNDVERI });

                //*** Read verification response
                Debug.WriteLine(String.Format("{0}\tCreateBond:Read verification response", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                byte[] veriResp;                
                {
                    var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, SecureBondService.ATT_VERIBUFF_UUID, SecureBondService.VERIFICATION_RESPONSE_LENGTH);
                    if (result.Item1)
                    {
                        veriResp = result.Item2;
                    }
                    else
                    {
                        return false;
                    }
                }

                // Extract encryption components
                byte[] iv = veriResp.Take(AES.IV_LENGTH).ToArray();
                byte aesBlockCount = veriResp.ElementAt(AES.IV_LENGTH);
                byte[] cipherText = veriResp.Skip(AES.IV_LENGTH + 1).Take(aesBlockCount * AES.BLOCK_LENGTH).ToArray();
                byte[] sharedSecretTruncated = sharedSecret.Truncated; //  Truncate shared secret
                byte[] plaintext;

                // Important Note: Ignore zero padded block as token does not currently support this
                AES.Decrypt(ref iv, ref sharedSecretTruncated, ref cipherText, out plaintext, System.Security.Cryptography.PaddingMode.None);

                // Check response
                byte[] plaintextTruncated = plaintext.Take(AES.BLOCK_LENGTH).ToArray();
                bool isSuccess = SecureBondService.VerifyResponse(ref plaintextTruncated);

                //*** Send encrypted result
                Debug.WriteLine(String.Format("{0}\tCreateBond:Send encrypted result isSuccess={1}", DateTime.Now, isSuccess));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                // Create encrypted buffer to be sent
                byte[] encryptedBuffer;
                byte[] plaintextVeriResponse = { isSuccess ? SecureBondService.VERI_SUCCESS : SecureBondService.VERI_FAIL };
                AES.CreateEncryptedBuffer(ref sharedSecretTruncated, ref plaintextVeriResponse, out encryptedBuffer);

                // Write verification result to token
                await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_VERIBUFF_UUID, encryptedBuffer);

                //*** Send update status to token i.e. Central device is ready
                Debug.WriteLine(String.Format("{0}\tCreateBond:Update token that central device is ready", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_STATUS_UUID, new byte[] { (byte)SecureBondService.CentralState.RDY });
#if false
                //*** Wait for ready or initial state before disconnecting
                Debug.WriteLine(String.Format("{0}\tCreateBond:Wait token state to be ready", DateTime.Now));
                byte[] status4 = null;
                while ((status4 == null) || (((status4[0] != (byte)SecureBondService.TokenState.RDY) && (status4[0] != (byte)SecureBondService.TokenState.INIT))))
                {
                    status4 = await BLE_AdvFunctions.ReadRequest(connectedDevice, SecureBondService.ATT_STATUS_UUID, 1);
                }
#endif
                return isSuccess;
            }
            finally
            {
                myKey.Dispose();
                tokenKey.Dispose();
                myPubKeyBnX.Dispose();
                myPubKeyBnY.Dispose();
                tokenPubKeyBnX.Dispose();
                tokenPubKeyBnY.Dispose();
                tokenPubKey.Dispose();
                ctx.Dispose();

#if DEBUG_DIAGNOSTICS
                sw.Stop();
                Debug.WriteLine(String.Format("CreateBond:ElapsedTime=[{0}]", sw.Elapsed));
#endif
            }
        }

        static public async Task<bool> RemoveBond(BLE.V2_2.Device connectedDevice, SessionData sessionData, BondInfo bondInfo)
        {
            // Ready plaintext
            byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)SecureBondService.EncrCmdId.RMV_BOND },
                        Helper.CreateDeviceID(bondInfo.DeviceData.Address)
                    });

            // Create encrypted buffer
            byte[] encryptedCmd;
            sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedCmd);

            // Write encrypted command
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, ATT_ENCRCFG_UUID, encryptedCmd);
        }

        static public async Task<bool> EraseAllBonds(BLE.V2_2.Device connectedDevice)
        {
            // Create data to send i.e. CMD
            byte[] cmdData = { (byte)SecureBondService.CmdId.ERASE_ALL };

            // Send command data
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, SecureBondService.ATT_CFG_UUID, cmdData);
        }
    }
}
