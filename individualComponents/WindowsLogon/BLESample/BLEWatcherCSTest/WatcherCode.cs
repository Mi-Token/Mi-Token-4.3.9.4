#define EVENT_LOG
//#define DEBUG_MODE

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

#if DEBUG_MODE
#else
using BLE_API;
#endif
using System.Threading;
using System.Threading.Tasks;
using System.Security.Cryptography;


using System.Diagnostics;

using System.Runtime.InteropServices;

using Microsoft.Win32;

namespace BLEWatcherCSTest
{
    class WatcherCodeV2
    {
        public enum StateMachine
        {
            Initializing, //Service is starting up
            WaitingForWakeup, //Service is waiting for the CP to announce that it is online
            LookingForToken, //Service is searching for a token that is within range
            AuthenticatingToken, //Service is authenticating a token that it found
            CheckingForUserPassDB, //Service is checking for any user/pass that belongs in the DB associated with this token
            SendingUserPassDB, //Service is sending User/Pass information down to the CP
            WaitingForCPResponse, //Service is waiting for the CP's response
            StoringUserPassDB, //Service is storing the User/Pass from CP in the DB
            WatchingToken, //Service is connected to and is watching a token
            RequestingLock, //Service is requesting a lock occur from BLELocker.exe

        };

        enum DATA_FLAG : byte
        {
            LOGIN_ON = 0x01,
            LOGIN_DONE = 0x02,
            CHECK_FOR_MAC = 0x03,
            REQUEST_LOCK = 0x04,
            REQUEST_QUIT = 0x05,
            STORE_PASSWORD = 0x06,
            REQUEST_PASSWORD = 0x07,
            PASSWORD_VAL = 0x08,

            CP_ONLINE = 0x09,

        };

        StateMachine _myState;
        BLE.V2_2.MiTokenBLE ble;
        BLE.V2_2.Device device;

        string COMPort = "";
        int logoffRSSI = 0;
        int logonRSSI = 0;
        bool verboseLogging = false;

        public void eventLogMessage(string message, bool requireRegKey = false)
        {
#if EVENT_LOG

            if ((requireRegKey) && (!verboseLogging))
            {
                return;
            }
            const string source = "Mi-Token BLE Watcher";
            const string log = "Application";

            if (!EventLog.SourceExists(source))
            {
                EventLog.CreateEventSource(source, log);
            }
            EventLog.WriteEntry(source, message);
            return;
#else
            return;
#endif
        }

        internal bool readConfig()
        {
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\BLE", false))
            {
                if (key != null)
                {
                    COMPort = (string)key.GetValue("COMPort", null);
                    logoffRSSI = (int)((byte[])key.GetValue("LogoffRSSI", 0))[0];
                    logonRSSI = (int)((byte[])key.GetValue("LogonRSSI", 0))[0];
                    object o = key.GetValue("servicelog", null);
                    if (o != null)
                    {
                        if (((byte[])o)[0] == 1)
                        {
                            verboseLogging = true;
                        }
                    }
                }
            }

            if (COMPort == null)
            {
                eventLogMessage("Error - COM Port not set in registry!");
                return false;
            }

            return true;
        }

        AutoResetEvent _namedPipeDataEvent;
        AutoResetEvent _hasDeviceEvent;

        static internal WatcherCodeV2 _watcher;

        internal byte[] bestDeviceMac = null;
        internal sbyte bestDeviceRSSI = SByte.MinValue;
        internal string verifiedUsername = "";
        internal Dictionary<string, byte[]> internalPasswordDB = new Dictionary<string, byte[]>();

        static byte[] volatileKey;
        
        static byte[] encryptPassword(string password)
        {
            byte[] IV = new byte[32];
            RNGCryptoServiceProvider crng = new RNGCryptoServiceProvider();
            crng.GetBytes(IV);
            byte[] plainTextBytes = Encoding.UTF8.GetBytes(password);
            byte[] encryptedData;
            using (AesManaged aes = new AesManaged())
            {
                aes.Mode = CipherMode.CBC;
                using (ICryptoTransform encryptor = aes.CreateEncryptor(volatileKey, IV))
                {
                    using (System.IO.MemoryStream ms = new System.IO.MemoryStream())
                    {
                        using (CryptoStream cs = new CryptoStream(ms, encryptor, CryptoStreamMode.Write))
                        {
                            cs.Write(plainTextBytes, 0, plainTextBytes.Length);
                            cs.FlushFinalBlock();
                            encryptedData = ms.ToArray();
                        }
                    }
                }
                aes.Clear();
            }

            byte[] ret = new byte[IV.Length + encryptedData.Length];
            Array.Copy(IV, 0, ret, 0, IV.Length);
            Array.Copy(encryptedData, 0, ret, IV.Length, encryptedData.Length);

            return ret;
        }

        static string decryptPassword(byte[] encPassword)
        {
            byte[] IV = new byte[32];
            Array.Copy(encPassword, 0, IV, 0, IV.Length);
            byte[] encText = new byte[encPassword.Length - IV.Length];
            Array.Copy(encPassword, IV.Length, encText, 0, encText.Length);
            byte[] plainBytes = null;
            string ret = "";
            using (AesManaged aes = new AesManaged())
            {
                aes.Mode = CipherMode.CBC;
                using(ICryptoTransform decryptor = aes.CreateDecryptor(volatileKey, IV))
                {
                    using (System.IO.MemoryStream ms = new System.IO.MemoryStream(encText))
                    {
                        using (CryptoStream cs = new CryptoStream(ms, decryptor, CryptoStreamMode.Read))
                        {
                            plainBytes = new byte[encText.Length];
                            int pbc = cs.Read(plainBytes, 0, plainBytes.Length);
                            ret = Encoding.UTF8.GetString(plainBytes, 0, pbc);
                        }
                    }
                }
            }
            return ret;
        }

        public bool getPasswordForUsername(string username, out string password)
        {
            password = "";
            if (internalPasswordDB.ContainsKey(username))
            {
                byte[] encPass = internalPasswordDB[username];
                password = decryptPassword(encPass);
                return true;
            }
            return false;
        }

        public bool setPasswordForUsername(string username, string password)
        {
            byte[] encPass = encryptPassword(password);
            if (internalPasswordDB.ContainsKey(username))
            {
                internalPasswordDB[username] = encPass;
            }
            else
            {
                internalPasswordDB.Add(username, encPass);
            }
            return true;
        }


        static BLE.CommandChain.CommandChainCall ccc_evt_gap_scan_response = (IntPtr caller, IntPtr data) =>
            {
                BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE(data);
                if (_watcher._myState == StateMachine.LookingForToken)
                {
                    if (msg.rssi > _watcher.logonRSSI)
                    {
                        if (msg.rssi > _watcher.bestDeviceRSSI)
                        {
                            _watcher.bestDeviceRSSI = msg.rssi;
                            _watcher.bestDeviceMac = msg.sender;
                            _watcher._hasDeviceEvent.Set();
                        }

                    }
                }

                return;
            };
        public WatcherCodeV2()
        {
            _watcher = this;
            _myState = StateMachine.Initializing;
            _namedPipeDataEvent = new AutoResetEvent(false);
            _hasDeviceEvent = new AutoResetEvent(false);
            
            //Create our volatile key, this key is only stored in RAM and is used to encrypt passwords for our password DB.
            RNGCryptoServiceProvider crng = new RNGCryptoServiceProvider();
            volatileKey = new byte[32];
            crng.GetBytes(volatileKey);

            ble = new BLE.V2_2.MiTokenBLE();
            if (readConfig())
            {
                if (ble.Initialize(COMPort, "\\\\.\\pipe\\Mi-TokenBLE", true) == BLE.Core.API_RET.BLE_API_SUCCESS)
                {
                    eventLogMessage("Initialized DLL");
                    BLE.V2_1.MiTokenBLE_GotMessageCallback += V2_1_MiTokenBLE_GotMessageCallback;
                    ble.Pipe_SetMessageCallback();
                }
            }
            BLE.CommandChain CC = new BLE.CommandChain();
            CC.evt.gap.scan_response = ccc_evt_gap_scan_response;

            ble.AppendCommandChain(CC, true);
        }

        void V2_1_MiTokenBLE_GotMessageCallback(BLE.V2_1.MiTokenBLE instance, byte[] data)
        {
            DATA_FLAG df = (DATA_FLAG)data[0];
            switch (df)
            {
                case DATA_FLAG.CHECK_FOR_MAC:
                    break;
                case DATA_FLAG.CP_ONLINE:
                    SwitchToState(StateMachine.LookingForToken);
                    _namedPipeDataEvent.Set();
                    break;
                case DATA_FLAG.LOGIN_DONE:
                    break;
                case DATA_FLAG.LOGIN_ON:
                    break;
                case DATA_FLAG.PASSWORD_VAL:
                    break;
                case DATA_FLAG.REQUEST_LOCK:
                    break;
                case DATA_FLAG.REQUEST_PASSWORD:
                    break;
                case DATA_FLAG.REQUEST_QUIT:
                    break;
                case DATA_FLAG.STORE_PASSWORD:
                    break;
            }
        }

        enum DeviceVersion
        {
            DevVersion_Unknown,
            DevVersion1_0,
            DevVersion1_1,
        };

        public void SwitchToState(StateMachine newState)
        {
            switch (newState)
            {
                case StateMachine.AuthenticatingToken:
                    goto default;
                    break;
                case StateMachine.CheckingForUserPassDB:
                    switch (_myState)
                    {
                        //valid _myState cases where no processing is required
                        case StateMachine.LookingForToken:
                            _myState = newState;
                            break;
                        //valid _myState cases where some processing is required

                        //invalid _myState cases
                        case StateMachine.AuthenticatingToken:
                        case StateMachine.CheckingForUserPassDB:
                        case StateMachine.Initializing:
                        case StateMachine.RequestingLock:
                        case StateMachine.SendingUserPassDB:
                        case StateMachine.StoringUserPassDB:
                        case StateMachine.WaitingForCPResponse:
                        case StateMachine.WaitingForWakeup:
                        case StateMachine.WatchingToken:
                        default:
                            eventLogMessage("Error : Unhandled SwitchToState.[LookingForToken] from State of " + _myState.ToString());
                            break;
                    }
                    break;
                case StateMachine.Initializing:
                    goto default;
                    break;
                case StateMachine.LookingForToken:
                    switch(_myState)
                    {
                        //valid _myState cases where no processing is required
                        case StateMachine.WaitingForWakeup: 
                            _myState = newState;
                            break;
                        //valid _myState cases where some processing is required

                        //invalid _myState cases
                        case StateMachine.AuthenticatingToken:
                        case StateMachine.CheckingForUserPassDB:
                        case StateMachine.Initializing:
                        case StateMachine.LookingForToken:
                        case StateMachine.RequestingLock:
                        case StateMachine.SendingUserPassDB:
                        case StateMachine.StoringUserPassDB:
                        case StateMachine.WaitingForCPResponse:
                        case StateMachine.WatchingToken:
                        default:
                            eventLogMessage("Error : Unhandled SwitchToState.[LookingForToken] from State of " + _myState.ToString());
                            break;
                    }
                    break;
                case StateMachine.RequestingLock:
                    goto default;
                    break;
                case StateMachine.SendingUserPassDB:
                    goto default;
                    break;
                case StateMachine.StoringUserPassDB:
                    goto default;
                    break;
                case StateMachine.WaitingForCPResponse:
                    goto default;
                    break;
                case StateMachine.WaitingForWakeup:
                    switch (_myState)
                    {
                        //valid _myState cases where no processing is required
                        case StateMachine.Initializing:
                            _myState = newState;
                            break;
                        //valid _myState cases where some processing is required

                        //invalid _myState cases
                        case StateMachine.AuthenticatingToken:
                        case StateMachine.CheckingForUserPassDB:
                        case StateMachine.LookingForToken:
                        case StateMachine.RequestingLock:
                        case StateMachine.SendingUserPassDB:
                        case StateMachine.StoringUserPassDB:
                        case StateMachine.WaitingForCPResponse:
                        case StateMachine.WaitingForWakeup:
                        case StateMachine.WatchingToken:
                        default:
                            eventLogMessage("Error : Unhandled SwitchToState.[WaitingForWakeup] from State of " + _myState.ToString());
                            break;
                    }
                    break;
                case StateMachine.WatchingToken:
                    goto default;
                    break;
                default:
                    eventLogMessage("Error : Unhandled SwitchToState.NewState value of " + newState.ToString());
                    break;
            }

        }

        public void Process()
        {
            switch (_myState)
            {
                case StateMachine.AuthenticatingToken:
                    State_AuthenticatingToken();
                    break;
                case StateMachine.CheckingForUserPassDB:
                    State_CheckingForUserPassDB();
                    break;
                case StateMachine.Initializing:
                    State_Initializing();
                    break;
                case StateMachine.LookingForToken:
                    State_LookingForToken();
                    break;
                case StateMachine.RequestingLock:
                    State_RequestingLock();
                    break;
                case StateMachine.SendingUserPassDB:
                    State_SendingUserPassDB();
                    break;
                case StateMachine.StoringUserPassDB:
                    State_StoringUserPassDB();
                    break;
                case StateMachine.WaitingForCPResponse:
                    State_WaitingForCPResponse();
                    break;
                case StateMachine.WaitingForWakeup:
                    State_WaitingForWakeup();
                    break;
                case StateMachine.WatchingToken:
                    State_WatchingToken();
                    break;
                    
            }
        }

        internal void State_AuthenticatingToken()
        { }

        internal void State_CheckingForUserPassDB()
        {
            string pass;
            if (getPasswordForUsername(verifiedUsername, out pass))
            {
                //send message to the CP so they can auto-login
                SwitchToState(StateMachine.SendingUserPassDB);
            }
            else
            {
                //send message to the CP saying that we don't have their pass
                SwitchToState(StateMachine.WaitingForCPResponse);
                
            }
        }

        internal void State_Initializing()
        {
            if (ble.ConnectedToCOM() == false)
            {
                if (ble.COMOwnerKnown())
                {
                    ble.WaitForCOMToBeFree();
                    ble.ReinitCOM();
                }
                else
                {
                    Thread.Sleep(1000);
                }
            }
            else
            {
                SwitchToState(StateMachine.WaitingForWakeup);
            }
        }
        internal void State_LookingForToken()
        {
            _hasDeviceEvent.WaitOne();

            if ((bestDeviceMac != null) && (bestDeviceRSSI >= logonRSSI))
            {
                int devCount = ble.GetDeviceFoundCount();
                BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                for (int i = 0; i < devCount; ++i)
                {
                    ble.GetDeviceInfo(i, ref devInfo);
                    bool sameAddress = true;
                    for (int j = 0; (j < 6) && sameAddress; ++j)
                    {
                        if (devInfo.macAddress[j] != bestDeviceMac[j])
                        {
                            sameAddress = false;
                        }
                    }

                    if (sameAddress)
                    {
                        ManualResetEvent gotHash = new ManualResetEvent(false);
                        string serverName = "";
                        string hash, UID = "";
                        byte[] binhash = null;
                        bool useAPI = false;
                        //let's add some ASYNC
                        Thread t = new Thread(new ThreadStart(() =>
                            {
                                if (Thread.CurrentThread.Name == null)
                                {
                                    Thread.CurrentThread.Name = "ASYNC API Call GetChallenge";
                                }

                                using (RegistryKey key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\BLE"))
                                {
                                    if (key != null)
                                    {
                                        String[] vals = key.GetValueNames();
                                        foreach (string val in vals)
                                        {
                                            if (key.GetValueKind(val) == RegistryValueKind.String)
                                            {
                                                serverName = (string)key.GetValue(val);
                                                string resp = MiToken.API.AnonGenerateBLEHash(serverName, devInfo.macString());
                                                try
                                                {
                                                    Dictionary<string, string> json = MiToken.API.ParseAPIJSON(resp);
                                                    if (json.ContainsKey("result") && json["result"] == "success")
                                                    {
                                                        if (json.ContainsKey("hash") && json.ContainsKey("UID"))
                                                        {
                                                            hash = json["hash"];
                                                            hash = hash.Replace('-', '+').Replace('_', '/');
                                                            UID = json["UID"];
                                                            binhash = Convert.FromBase64String(hash);
                                                            useAPI = true;
                                                            break;
                                                        }
                                                    }
                                                }
                                                catch (FormatException)
                                                {
                                                    //invalid json string
                                                    continue;
                                                }
                                            }
                                        }
                                    }
                                }
                                gotHash.Set();
                            }));

                        t.Start();
                        
                        bool hashVersion1_1 = false;
                        if (ble.Pipe_RequestExclusiveAccess() != BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                        {
                            //TODO handle this
                            return;
                        }
                        bool waitForExclusive = true;
                        //TODO create function
                        ble.Pipe_SyncWaitForExclusive();
                        if (ble.Pipe_HasExclusiveAccess() == BLE.Core.API_NP_RET.BLE_API_EXCLUSIVE_REJECTED)
                        {
                            //TODO handle this
                            return;
                        }

                        device = ble.connectToDevice(devInfo);
                        device.SyncWaitForConnection(BLE.Core.CONN_TIMEOUT_INFINITE);


                        DeviceVersion versionID = DeviceVersion.DevVersion_Unknown;
                        getDeviceVersion(device, ref versionID);
                        byte[] readHash;

                        if (versionID == DeviceVersion.DevVersion_Unknown)
                        {
                            //TODO handle this
                            return;
                        }
                        else
                        {
                            UInt16 uuid_hash_service = (versionID == DeviceVersion.DevVersion1_0 ? MiTokenBLEWatcher.MiTokenBLEUUID.SERVICE_UUID.HASH_OLD : MiTokenBLEWatcher.MiTokenBLEUUID.SERVICE_UUID.HASH);
                            UInt16 uuid_hash_buffer = (versionID == DeviceVersion.DevVersion1_0 ? MiTokenBLEWatcher.MiTokenBLEUUID.ATTRIBUTE_UUID.HASH_OLD.BUFFER : MiTokenBLEWatcher.MiTokenBLEUUID.ATTRIBUTE_UUID.HASH.BUFFER);
                            UInt16 uuid_hash_status = (versionID == DeviceVersion.DevVersion1_0 ? MiTokenBLEWatcher.MiTokenBLEUUID.ATTRIBUTE_UUID.HASH_OLD.STATUS : MiTokenBLEWatcher.MiTokenBLEUUID.ATTRIBUTE_UUID.HASH.STATUS);

                            device.SyncScanServicesInRange(uuid_hash_service, 0xFFFF, BLE.Core.CONN_TIMEOUT_INFINITE);
                            byte[] fullHash = null;
                            //we need to have the hash by now
                            gotHash.WaitOne();
                            switch (versionID)
                            {
                                case DeviceVersion.DevVersion1_0:
                                    fullHash = binhash;
                                    break;
                                case DeviceVersion.DevVersion1_1:
                                    fullHash = new byte[16 + 20];
                                    for (int pos = 0; pos < 16; pos += 6)
                                    {
                                        Array.Copy(devInfo.macAddress, 0, fullHash, pos, 6);
                                    }
                                    Array.Copy(binhash, 0, fullHash, 16, 20);
                                    break;
                            }

                            device.SyncSetAttribute(uuid_hash_buffer, fullHash, BLE.Core.CONN_TIMEOUT_INFINITE);
                            
                            device.SyncReadAttribute(uuid_hash_buffer, true, out readHash, BLE.Core.CONN_TIMEOUT_INFINITE);

                            device.Disconnect();
                            ble.Pipe_ReleaseExclusiveAccess();
                        }

                        string b64outhash = Convert.ToBase64String(readHash);
                        if (useAPI)
                        {
                            b64outhash = b64outhash.Replace('/', '_').Replace('+', '-');

                            string verifyResponse = MiToken.API.AnonVerifyBLEHashResponse(serverName, devInfo.macString(), UID, b64outhash);
                            Dictionary<string, string> veriJson = new Dictionary<string, string>();
                            try
                            {
                                veriJson = MiToken.API.ParseAPIJSON(verifyResponse);
                            }
                            catch (FormatException)
                            {
                                //most likely a failed call
                                //TODO handle this
                                return;
                            }

                            if (veriJson.ContainsKey("result") && (veriJson["result"] == "success"))
                            {
                                if (veriJson.ContainsKey("username"))
                                {
                                    verifiedUsername = veriJson["username"];
                                    SwitchToState(StateMachine.CheckingForUserPassDB);
                                }
                            }

                        }
                        else
                        {
                            //TODO : Handle this
                            return;
                        }


                        


                    }
                }
            }
        }

        void getDeviceVersion(BLE.V2_2.Device device, ref DeviceVersion versionID)
        {
            throw new NotImplementedException();
        }

        internal void State_RequestingLock()
        {
        }
        internal void State_SendingUserPassDB()
        {
        }
        internal void State_StoringUserPassDB()
        {
        }
        internal void State_WaitingForCPResponse()
        {
        }
        internal void State_WaitingForWakeup()
        {
            _namedPipeDataEvent.WaitOne();
        }
        internal void State_WatchingToken()
        {

        }


    };

    class WatcherCode
    {
        bool runMainLoop;

        BLE.V2_2.MiTokenBLE MiTokenBLE;
        Dictionary<string, string> passStore = new Dictionary<string, string>();

        static bool regKeyLogging = false;

        static public void eventLogMessage(string message, bool requireRegKey = false)
        {
#if EVENT_LOG

            if ((requireRegKey) && (!regKeyLogging))
            {
                return;
            }
            const string source = "Mi-Token BLE Watcher";
            const string log = "Application";

            if (!EventLog.SourceExists(source))
            {
                EventLog.CreateEventSource(source, log);
            }
            EventLog.WriteEntry(source, message);
            return;
#else
            return;
#endif
        }

#if DEBUG_MODE
#else

        public void logException(Exception ex)
        {
            eventLogMessage(string.Format("Exception raised : \r\n{0}\r\nStack Trace :\r\n{1}", ex.Message, ex.StackTrace));
        }

        const byte DATA_FLAG_LOGIN_ON = 0x01;
        const byte DATA_FLAG_LOGIN_DONE = 0x02;
        const byte DATA_FLAG_CHECK_FOR_MAC = 0x03;

        const byte DATA_FLAG_REQUEST_LOCK = 0x04;

        const byte DATA_FLAG_REQUEST_QUIT = 0x05;

        const byte DATA_FLAG_PASSWORD = 0x06;
        const byte DATA_FLAG_REQ_PASSWORD = 0x07;
        const byte DATA_FLAG_PASSWORD_VAL = 0x08;

        bool connected = false;

        byte[] addressToWatch = null;

        BLE.V2_1.Filter filter = null;

        AutoResetEvent watcherPowerup;

        string COMPort;

        public bool initFine = false;

        int minReqRSSI = 0;

        public WatcherCode()
        {
            watcherPowerup = new AutoResetEvent(false);
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\BLE", false))
            {
                if (key != null)
                {
                    COMPort = (string)key.GetValue("COMPort", null);
                    minReqRSSI = (int)((byte[])key.GetValue("LogoffRSSI", 0))[0];

                    MiTokenBLE = new BLE.V2_2.MiTokenBLE();

                    eventLogMessage("Getting Verbose Log Setting.");
                    object o = key.GetValue("servicelog", null);

                    if (o != null)
                    {
                        if (((byte[])o)[0] == 1)
                        {
                            regKeyLogging = true;
                            eventLogMessage("Verbose Logging Enabled.");
                        }
                        else
                        {
                            eventLogMessage("Verbose Logging Disabled.");
                        }
                    }
                    else
                    {
                        eventLogMessage("Verbose Logging Unspecified - Defaulting to Disabled.");
                    }
                }
            }

            if (COMPort == null)
            {
                eventLogMessage("Error - COM port not set in registry.");
                initFine = false;
            }
            else
            {
                initFine = true;
            }
        }

        public bool connectToCOM()
        {
            //if (DLLWrapper.V2.InitializeEx("COM11", "\\\\.\\pipe\\Mi-TokenBLE", true) == 0)
            if(MiTokenBLE.Initialize(COMPort, "\\\\.\\pipe\\Mi-TokenBLE", true) == BLE.Core.API_RET.BLE_API_SUCCESS)
            {
                eventLogMessage("Initialized DLL");
                connected = true;

                BLE.V2_1.MiTokenBLE_GotMessageCallback += V2_1_MiTokenBLE_GotMessageCallback;
                MiTokenBLE.Pipe_SetMessageCallback();

                /*
                DLLWrapper.V2.NP.NP_MessageCallback += new DLLWrapper.V2.NP.NPMessageRecieved(NP_NP_MessageCallback);
                DLLWrapper.V2.NP.SetNPSCallback();
                 * */
                return true;
            }
            return false;
        }

        public bool hasAccessToCOMPort(bool waitForAccess)
        {
            eventLogMessage("Checking for COM Access");
            if (MiTokenBLE.ConnectedToCOM() == true)
            {
                eventLogMessage("Has Access to COM Port");
                return true;
            }

            if (waitForAccess)
            {
                if (MiTokenBLE.COMOwnerKnown())
                {
                    eventLogMessage("COM Owner Known, Waiting");
                    MiTokenBLE.WaitForCOMToBeFree();
                    MiTokenBLE.ReinitCOM();
                    return true;
                }
                else
                {
                    eventLogMessage("COM Owner Unknown");
                }
            }
            return false;
        }

        public void waitForWatcherPowerup()
        {
            eventLogMessage("Waiting for Powerup");
            watcherPowerup.WaitOne();
        }

        volatile bool closeTime = false;

        public void closeDown()
        {
            closeTime = true;
            watcherPowerup.Set();

            eventLogMessage("Requesting BLELockout to terminate");
            MiTokenBLE.Pipe_SendMessage(new byte[] { DATA_FLAG_REQUEST_QUIT }, -1);
            eventLogMessage("BLELockout termination request sent");

        }

        public void mainWatching()
        {
            runMainLoop = true;
            try
            {
                if(addressToWatch == null)
                {
                    eventLogMessage("Address to watch is null");
                    return;
                }

                eventLogMessage("In Main Watching");
                int pollSpeed = 300;
                int DefaultCoolDown = 10;
                int curCoolDown = DefaultCoolDown;
                int pollID = 0;


                while ((curCoolDown > 0) && (runMainLoop))
                {
                    eventLogMessage(string.Format("Cooldown = {0}", curCoolDown), true);
                    pollID++;
                    MiTokenBLE.SetPollID(pollID);
                    Thread.Sleep(pollSpeed);
                    int devFound = MiTokenBLE.GetDeviceFoundCount();
                    curCoolDown--;
                    for (int devID = 0; devID < devFound; ++devID)
                    {
                        BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                        MiTokenBLE.GetDeviceInfo(devID, ref devInfo);
                        bool equal = true;
                        for (int id = 0; id < 6; ++id)
                        {
                            if (devInfo.macAddress[id] != addressToWatch[id])
                            {
                                equal = false;
                                break;
                            }
                        }

                        if (equal)
                        {

                            if (devInfo.lastPollSeen >= pollID)
                            {
                                if (devInfo.RSSI > minReqRSSI)
                                {
                                    eventLogMessage(string.Format("Device found PID {0} vs {1}", devInfo.lastPollSeen, pollID), true);
                                    curCoolDown = DefaultCoolDown;
                                }
                                else
                                {
                                    eventLogMessage(string.Format("Device is too far away PID {0}", devInfo.lastPollSeen), true);
                                }
                            }
                            else
                            {
                                string stalemessage = string.Format("Device is stale {0} vs {1}", devInfo.lastPollSeen, pollID);
                                eventLogMessage(stalemessage, true);
                            }
                            break;
                        }
                    }
                }

                if (runMainLoop)
                {
                    lockMachine();
                }

                addressToWatch = null;
            }

            catch(Exception ex)
            {
                eventLogMessage("Exception raised : " + ex.Message + "\r\n" + ex.StackTrace);
            }
        }
        public void lockMachine()
        {
            eventLogMessage("Locking machine");
            MiTokenBLE.Pipe_SendMessage(new byte[] { DATA_FLAG_REQUEST_LOCK }, -1);
            eventLogMessage("Send Message over NamedPipe");
        }

        /*
        void NP_NP_MessageCallback(byte[] data)
        {
            V2_1_MiTokenBLE_GotMessageCallback(null, data);
        }*/


        void V2_1_MiTokenBLE_GotMessageCallback(BLE.V2_1.MiTokenBLE instance, byte[] data)
        {
            eventLogMessage("Got NP Message");
            switch (data[0])
            {
                case DATA_FLAG_LOGIN_ON:
                    runMainLoop = false;
                    break;
                case DATA_FLAG_LOGIN_DONE:
                    break;
                case DATA_FLAG_CHECK_FOR_MAC:
                    if (data.Length == 7)
                    {
                        //data should be [flag] [mac_address]
                        addressToWatch = new byte[6];
                        Array.Copy(data, 1, addressToWatch, 0, 6);


                        //filter = new DLLWrapper.V2.Filter();


                        watcherPowerup.Set();
                    }
                    break;
                case DATA_FLAG_PASSWORD:
                    try
                    {
                        eventLogMessage(string.Format("Got Password store request of size {0}", data.Length), true);
                        if (data.Length < 2)
                        {
                            //there is an error
                            return;
                        }
                        int usernameLen = data[1];

                        if (data.Length < (2 + usernameLen))
                        {
                            return;
                        }

                        byte[] usernameBytes = new byte[usernameLen];

                        Array.Copy(data, 2, usernameBytes, 0, usernameLen);

                        int offset = 2 + usernameLen;
                        int passwordLen = data[offset];

                        if (data.Length < (offset + passwordLen))
                        {
                            return;
                        }

                        byte[] passwordBytes = new byte[passwordLen];
                        Array.Copy(data, offset + 1, passwordBytes, 0, passwordLen);

                        string susername = Encoding.ASCII.GetString(usernameBytes);
                        string spassword = Encoding.ASCII.GetString(passwordBytes);

                        if (passStore.ContainsKey(susername))
                        {
                            passStore[susername] = spassword;
                        }
                        else
                        {
                            passStore.Add(susername, spassword);
                        }

                        eventLogMessage(string.Format("Storing password {1} for user {0}", susername, spassword), true);
                    }
                    catch (Exception ex)
                    {
                        logException(ex);
                    }

                    break;

                case DATA_FLAG_REQ_PASSWORD:
                    try
                    {
                        eventLogMessage(string.Format("Got Password request for user, of size {0}", data.Length), true);

                        int usernameLen = data[1];
                        byte[] username = new byte[usernameLen];

                        Array.Copy(data, 2, username, 0, usernameLen);


                        string susername = Encoding.ASCII.GetString(username);
                        eventLogMessage(string.Format("Checking PassStore for username {0}", susername), true);

                        if(passStore.ContainsKey(susername))
                        {
                            string spass = passStore[susername];
                            eventLogMessage(string.Format("Pass of {0} found for user {1}", spass, susername), true);

                            byte[] bpass = Encoding.ASCII.GetBytes(spass);
                            int retMessageLen = 1 + //header
                                                1 + //user len
                                                username.Length + //user data
                                                1 + //pass len
                                                spass.Length;

                            byte[] message = new byte[retMessageLen];
                            int writePos = 0;
                            message[writePos] = DATA_FLAG_PASSWORD_VAL;
                            writePos++;
                            message[writePos] = data[1];
                            writePos++;
                            Array.Copy(username, 0, message, writePos, username.Length);
                            writePos += username.Length;
                            message[writePos] = (byte)bpass.Length;
                            writePos++;
                            Array.Copy(bpass, 0, message, writePos, bpass.Length);


                            eventLogMessage("Sending Password Value message", true);
                            instance.Pipe_SendMessage(message, -1);
                        }
                    }
                    catch (Exception ex)
                    {
                        logException(ex);
                    }
                    break;

            }
            
        }

        public void startPoller()
        {
            //eventLogMessage("Starting Poller");
            if(MiTokenBLE.ConnectedToCOM())
            {
                /*
                eventLogMessage("Connected to COM");
                DLLWrapper.V2.NewPollResults += new DLLWrapper.V2.NewPollResultsDelegate(V2_NewPollResults);
                DLLWrapper.V2.AddPollThreadInstance();
                 * */
            }
        }

        void V2_NewPollResults()
        {

        }

#endif
        
    }
}
