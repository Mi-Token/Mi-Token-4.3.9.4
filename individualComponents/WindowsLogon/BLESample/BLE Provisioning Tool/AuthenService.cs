using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using BLE_API;
using OpenSSL.Crypto;

namespace BLE_Provisioning_Tool
{
    class SharedSecret
    {
        public SharedSecret()
        {
            Value = new byte[MessageDigest.SHA1.Size];
        }

        public SharedSecret(byte[] sharedSecret)
        {
            if (sharedSecret == null)
            {
                throw new ArgumentNullException();
            }

            if (sharedSecret.Length != MessageDigest.SHA1.Size)
            {
                throw new ArgumentOutOfRangeException();
            }

            Value = sharedSecret;
        }

        // Big endian
        public byte[] Value { get; private set; }

        public byte[] Truncated
        {
            get
            {
                return Value.Take(AES.KEY_LENGTH).ToArray();
            }
        }

        public string GetExportString()
        {
            // Make sure this is in little endian by reversing byte order
            byte[] reverseData = new byte[Value.Length];
            reverseData = (byte[])Value.Clone();
            Array.Reverse(reverseData);
            return Helper.GetExportHexString(reverseData);
        }
    }

    class AuthenService
    {
        // Service Attributes
        static public readonly UInt16 SERV_UUID = 0xA400; // Authentication        
        static private readonly UInt16 ATT_MODE_UUID = 0xA401; // Mode
        static private readonly UInt16 ATT_STATUS_UUID = 0xA402; // Status
        static private readonly UInt16 ATT_BUFFER_UUID = 0xA403; // Buffer

        static private readonly int CHALLENGE_LENGTH = SessionData.SESSION_MAC_LEN + SessionData.SESSION_NONCE_LEN;
            
        private enum TokenState
        {
            Idle = 0,
            PndChal,
            PndChalRsp,
            PndPin,
            Ready
        };

        private enum Event
        {
            ChalReady = 0,
            ChalRspReady,
            PinReady
	    };

        // Establish an authenticated session
        static public async Task<SessionData> EstablishSecureSession(BLE.V2_2.Device connectedDevice, ManualResetEventSlim resultEvent, SemaphoreSlim resultSemaphore, DeviceAddress address, SharedSecret sharedSecret, IProgress<int> progressBarStatus, ManualResetEventSlim procedureCompletedEvent)
        {
#if DEBUG_DIAGNOSTICS
            Stopwatch sw = new Stopwatch();
            sw.Start();
#endif
            try
            {
                int progressVal = 1;
                int totalProgressVal = 17;

                //*** Generate "PHONE" Nonce
                SessionData sessionData = new SessionData();
                sessionData.Init();

                // Important to set to Connection Parameters to Slow
                if (!await BLE_AdvFunctions.UpdateConnectionParameters(connectedDevice, resultEvent, resultSemaphore, false))
                {
                    return sessionData;
                }

                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Finding Services", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                // Ensure Authentication Service Exists
                await BLE_AdvFunctions.FindServiceAttributes(connectedDevice, AuthenService.SERV_UUID);
                
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Check ready to receive Challenge", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                //*** Read Pending Challenge State
                await BLE_AdvFunctions.ReadRequestUntilEqual(connectedDevice, AuthenService.ATT_STATUS_UUID, new byte[] { (byte)AuthenService.TokenState.PndChal });

                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Send Challenge", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                //*** Request Challenge (IDp + Np)
                // Write challenge to buffer            
                byte[] requestChallengeBuffer = Helper.BuildByte(new byte[][] {
                    Helper.CreateDeviceID(address),
                    sessionData.SelfNonce
                });
                await BLE_AdvFunctions.WriteRequest(connectedDevice, AuthenService.ATT_BUFFER_UUID, requestChallengeBuffer);

                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Update that Challenge has been sent", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                // Write event status
                await BLE_AdvFunctions.WriteRequest(connectedDevice, AuthenService.ATT_STATUS_UUID, new byte[] { (byte)AuthenService.Event.ChalReady });

                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Check Challenge Response ready to be read", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                //*** Read Challenge (MACt + Nt)
                // Wait for Challenge Response to be ready
                await BLE_AdvFunctions.ReadRequestUntilEqual(connectedDevice, AuthenService.ATT_STATUS_UUID, new byte[] { (byte)AuthenService.TokenState.PndChalRsp });

                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Check Connection", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                if (!await BLE_AdvFunctions.CheckConnection(connectedDevice, procedureCompletedEvent))
                {
                    bool bRefreshConnection = await BLE_AdvFunctions.RefreshConnection(connectedDevice);
                    Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Refresh Connection=[{1}]", DateTime.Now, bRefreshConnection));
                }

                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Read Challenge Response", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                // Read Challenge
                byte[] challengeBuffer;
                {
                    var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, AuthenService.ATT_BUFFER_UUID, CHALLENGE_LENGTH);
                    if (result.Item1)
                    {
                        challengeBuffer = result.Item2;
                    }
                    else
                    {
                        sessionData.IsAuthenticated = false;
                        return sessionData;
                    }
                }
                Buffer.BlockCopy(challengeBuffer, 0, sessionData.DeviceMac, 0, sessionData.DeviceMac.Length); // Copy Device MAC
                Buffer.BlockCopy(challengeBuffer, sessionData.DeviceMac.Length, sessionData.DeviceNonce, 0, sessionData.DeviceNonce.Length); // Copy Device Nonce

                // Generate Session Key and Derived Keys
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Generate Session and Derived Keys", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                if (!sessionData.CreateKeys(ref sharedSecret))
                {
                    return sessionData;
                }

                // Verify MACt
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Verify MAC", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                if (!sessionData.VerifyDevMAC())
                {
                    return sessionData;
                }

                // Generate MACp
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Generate MAC", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                sessionData.CreateSelfMac();

                //*** Send Challenge Response (MACp)
                // Write Challenge Response
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Write Challenge Response", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.WriteRequest(connectedDevice, AuthenService.ATT_BUFFER_UUID, sessionData.SelfMac);

                // Write event status
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Update that Challenge Response has been sent", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.WriteRequest(connectedDevice, AuthenService.ATT_STATUS_UUID, new byte[] { (byte)AuthenService.Event.ChalRspReady });

                //*** Send PIN (IV + Encypted PIN) default
                // Wait for Challenge Response to be ready
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Wait for Challenge Response to be ready", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.ReadRequestUntilEqual(connectedDevice, AuthenService.ATT_STATUS_UUID, new byte[] { (byte)AuthenService.TokenState.PndPin });

                // Create encrypted PIN buffer
                byte[] plaintextPin = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }; // Use same fixed PIN to make authentication process happy
                byte[] encrPinBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_PIN_KEY_ID, ref plaintextPin, out encrPinBuff);

                // Write encrypted PIN
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Write default PIN data", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.WriteRequest(connectedDevice, AuthenService.ATT_BUFFER_UUID, encrPinBuff);

                // Write event status
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Update that PIN data has been set", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.WriteRequest(connectedDevice, AuthenService.ATT_STATUS_UUID, new byte[] { (byte)AuthenService.Event.PinReady });

                // Wait for Token to be ready
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Wait for token to be ready", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                await BLE_AdvFunctions.ReadRequestUntilEqual(connectedDevice, AuthenService.ATT_STATUS_UUID, new byte[] { (byte)AuthenService.TokenState.Ready });

                sessionData.IsAuthenticated = true;

                // Important to set to Connection Parameters back to Fast
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Token ready", DateTime.Now));
                progressBarStatus.Report((int)((double)(progressVal++ / (double)totalProgressVal) * 100.0)); // Update Progress
                if (!await BLE_AdvFunctions.UpdateConnectionParameters(connectedDevice, resultEvent, resultSemaphore, true))
                {
                    sessionData.IsAuthenticated = false; // Reset authentication if parameters cannot be set
                }

                return sessionData;
            }
            finally
            {
#if DEBUG_DIAGNOSTICS
                Debug.WriteLine(String.Format("{0}\tEstablishSecureSession:Elapsed={0}", DateTime.Now, sw.Elapsed));
#endif
            }
        }

    }    
}
