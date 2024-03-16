#define EVENT_LOG

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using BLE_API;
using Microsoft.Win32;
using System.Diagnostics;


using System.Security.Cryptography;

namespace BLEWatcherStateMachine
{
    public class StateMachine
    {

        public void eventLogMessage(string message, string source = "Mi-Token BLE Watcher", bool requireRegKey = false)
        {
#if EVENT_LOG

            if ((requireRegKey))
            {
                return;
            }
            
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

        public enum State
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

            PreEmptiveLogin,
        }

        public abstract class IState
        {
            public IState(StateMachine myMachine)
            {
                this.myMachine = myMachine;
            }
            protected StateMachine myMachine;
            private IState()
            { }
            public abstract void DoProcess();
            public abstract State MyState();

            internal void _debugMessage(string strSender, string message)
            {
                myMachine._debugMessage(strSender, message);
            }

            internal void _debugMessage(string message)
            {
                myMachine._debugMessage(this.ToString(), message);
            }

            internal BLE.V2_2.MiTokenBLE ble
            {
                get
                {
                    return myMachine.myData.ble;
                }
                set
                {
                    myMachine.myData.ble = value;
                }
            }

            internal BLE.V2_2.Device device
            {
                get
                {
                    return myMachine.myData.device;
                }
                set
                {
                    myMachine.myData.device = value;
                }
            }

            public abstract State[] validTransferStates();
        }


        IState _myState;
        public IState currentState
        {
            get
            {
                return _myState;
            }
        }

        EventOverrides _eventOverrides;
        public EventOverrides eventOverrides
        {
            get
            {
                return _eventOverrides;
            }
        }


        Dictionary<State, IState> _stateDictionary;
        
        Dictionary<State, List<State>> validStateTransitions;

        public PasswordHandler passwordHandler;

        public class StateData
        {
            public byte[] bestDeviceMac;
            public sbyte bestDeviceRSSI;

            public AutoResetEvent _HasCPReturnData;
            public byte[] CPReturnData;
            public ManualResetEvent ForceStateUpdate;
            public bool ResetToWaitForWakeupState;

            public BLE_API.BLE.V2_2.MiTokenBLE ble;
            public BLE.V2_2.Device device;

            public string COMPort;
            public SByte logoffRSSI;
            public SByte logonRSSI;
            public bool verboseLogging;
            public string verifiedUsername;
            public bool verifiedUserMustChangePassword;
            public bool isUserMulti;

            public AutoResetEvent _hasDeviceEvent;
            public AutoResetEvent _namedPipeDataEvent;

            public class PreEmptive
            {
                public AutoResetEvent _hasPreEmptiveDevice;
                public bool _allowPreEmptiveLogins;

                protected class MapData
                {
                    public DateTime expiry;
                    public string BoundAccount;

                    public MapData(string accountID)
                    {
                        expiry = DateTime.Now;
                        BoundAccount = accountID;

                        RefreshExpiry();
                    }
                    public void RefreshExpiry()
                    {
                        expiry = DateTime.Now + new TimeSpan(0, 1, 0);
                    }
                    public bool Valid
                    {
                        get
                        {
                            return (expiry > DateTime.Now);
                        }
                    }
                }

                protected Dictionary<string, MapData> _validTokensMap;



                public PreEmptive()
                {
                    _allowPreEmptiveLogins = false;
                    _hasPreEmptiveDevice = new AutoResetEvent(false);
                    _validTokensMap = new Dictionary<string, MapData>();
                }

                public bool TokenHasBeenValidated(string tokenMac)
                {
                    if(_validTokensMap.ContainsKey(tokenMac))
                    {
                        if(_validTokensMap[tokenMac].Valid)
                        {
                            return true;
                        }
                        else
                        {
                            _validTokensMap.Remove(tokenMac);
                            return false;
                        }
                    }
                    return false;
                }

                public void TokenValidated(string tokenMac, string accountID)
                {
                    if(_validTokensMap.ContainsKey(tokenMac))
                    {
                        _validTokensMap[tokenMac].RefreshExpiry();
                    }
                    else
                    {
                        _validTokensMap.Add(tokenMac, new MapData(accountID));
                    }
                }
                public string TokensUser(string tokenMac)
                {
                    if(_validTokensMap.ContainsKey(tokenMac))
                    {
                        return _validTokensMap[tokenMac].BoundAccount;
                    }
                    return null;
                }
            };

            public PreEmptive _PreEmptive;

            public ManualResetEventSlim _connectionUpdateRspEvent;
            public SemaphoreSlim _connectionUpdateRspSemaphore;
            public ManualResetEventSlim _procedureCompletedEvent;

            public AutoResetEvent _CPOnlineEvent;
            public List<byte[]> _ignoredMacAddresses;            
            public class Filter
            {
                class metaDataFilter
                {
                    public byte flag;
                    public byte[] data;

                    public bool validData(byte[] checkData)
                    {
                        if ((checkData == null) || (checkData.Length == 0))
                        {
                            return false;
                        }

                        int offset = 0;
                        bool found = false;
                        while ((offset < checkData.Length) && (!found))
                        {
                            byte len, flag;
                            len = (byte)(checkData[offset] - 1);
                            flag = checkData[offset + 1];
                            offset += 2;
                            if (flag == this.flag)
                            {
                                found = true;
                                if (len == data.Length)
                                {
                                    for (int i = 0; i < len; ++i)
                                    {
                                        if (data[i] != checkData[offset + i])
                                        {
                                            return false; //they are different
                                        }
                                    }
                                }
                                else
                                {
                                    //they are different lengths
                                    return false;
                                }
                            }
                            offset += len;
                        }

                        //everything passed
                        return found;
                    }
                };
                metaDataFilter[] metaFilters;
                class addressFilter
                {
                    public byte[] address;

                    public bool validAddress(byte[] checkAddress)
                    {
                        if (checkAddress.Length == 6)
                        {
                            for (int i = 0; i < 6; ++i)
                            {
                                if (checkAddress[i] != address[i])
                                {
                                    return false;
                                }
                            }

                            return true;
                        }

                        return false;
                    }
                };

                addressFilter[] addressFilters;

                public Filter(byte[] filterBytes, StateMachine linkedMachine)
                {
                    /*
                        FilterBytes Format (Version 00)
                     *  Version [4 bytes] (FBS{xx})
                     *  Filter Count [4 bytes]
                     *  Address Count [4 bytes]
                     *  Foreach(filter)
                     *      flag [1 byte]
                     *      length [1 byte]
                     *      data [length]
                     *  Foreach(Address)
                     *      address [6 bytes]
                     */

                    if (filterBytes == null)
                    {
                        metaFilters = new metaDataFilter[0];
                        addressFilters = new addressFilter[0];
                        return;
                    }

                    int version = BitConverter.ToInt32(filterBytes, 0);
                    if (version == 0x00534246) //FBS[x00]
                    {
                        int filterCount = BitConverter.ToInt32(filterBytes, 4);
                        int addressCount = BitConverter.ToInt32(filterBytes, 8);
                        int offset = 12;
                        metaFilters = new metaDataFilter[filterCount];
                        addressFilters = new addressFilter[addressCount];
                        for (int i = 0; i < filterCount; ++i)
                        {
                            metaFilters[i] = new metaDataFilter();
                            metaFilters[i].flag = filterBytes[offset];
                            offset++;
                            metaFilters[i].data = new byte[filterBytes[offset]];
                            offset++;
                            Array.Copy(filterBytes, offset, metaFilters[i].data, 0, metaFilters[i].data.Length);
                            offset += metaFilters[i].data.Length;
                        }
                        for (int i = 0; i < addressCount; ++i)
                        {
                            addressFilters[i] = new addressFilter();
                            addressFilters[i].address = new byte[6];
                            Array.Copy(filterBytes, offset, addressFilters[i].address, 0, 6);
                            offset += 6;
                        }
                    }
                    else
                    {
                        //unsupported filter version
                        linkedMachine._debugMessage(this.ToString(), "Unsupported Filter Version ID " + version);
                        addressFilters = new addressFilter[0];
                        metaFilters = new metaDataFilter[0];
                    }
                }

                public bool devicePassesFilter(byte[] deviceMac, byte[] deviceMetaData)
                {
                    bool addressPass = false;
                    if (addressFilters.Length == 0)
                    {
                        addressPass = true;
                    }
                    else
                    {
                        for (int i = 0; ((i < addressFilters.Length) && (!addressPass)); ++i)
                        {
                            addressPass = addressFilters[i].validAddress(deviceMac);
                        }
                    }

                    if (!addressPass)
                    {
                        return false;
                    }

                    bool metaPass = true;
                    if (metaFilters.Length > 0)
                    {
                        for (int i = 0; ((i < metaFilters.Length) && (metaPass)); ++i)
                        {
                            metaPass = metaFilters[i].validData(deviceMetaData);
                        }
                    }

                    if (!metaPass)
                    {
                        return false;
                    }

                    //everything has passed
                    return true;
                }
            };

            public Filter _deviceFilter;
            public bool BeepOnConnect, BeepOnDisconnect;

            protected int curPollID;
            public int PollID
            {
                get
                {
                    curPollID++;
                    return curPollID;
                }
            }

            public class PersonalBond
            {
                public string Username;

                public PersonalBond(byte[] device, byte[] key, int challengeID, string username)
                {
                    this.device = device;
                    this.hmac_key = key;
                    this.challengeID = challengeID;
                    this.Username = username;
                }

                byte[] device;

                public bool CorrectDevice(byte[] macAddress)
                {
                    if (macAddress == null || device == null)
                    {
                        return false;
                    }
                    if (macAddress.Length != device.Length)
                    {
                        return false;
                    }
                    for (int i = 0; i < macAddress.Length; ++i)
                    {
                        if (macAddress[i] != device[i])
                        {
                            return false;
                        }
                    }

                    return true;
                }

                public string NextChallenge
                {
                    get
                    {
                        string challenge = "";
                        using (HMACSHA1 hmac = new HMACSHA1())
                        {
                            hmac.Key = hmac_key;
                            challengeID++;
                            byte[] challengeBytes = BitConverter.GetBytes(challengeID);
                            hmac.ComputeHash(challengeBytes);
                            byte[] hash = hmac.Hash;
                            challenge = Convert.ToBase64String(hash);

                            Thread T = new Thread(new ThreadStart(() =>
                                {
                                    if (Thread.CurrentThread.Name == null)
                                    {
                                        Thread.CurrentThread.Name = "HMAC Response Generator";
                                    }
                                    using (HMACSHA1 hmac2 = new HMACSHA1())
                                    {
                                        hmac2.Key = hmac_key;
                                        hmac2.ComputeHash(hash);
                                        expectedResponse = hmac2.Hash;

                                        //Update Registry Key
                                        using (RegistryKey key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\BLE", true))
                                        {
                                            if (key != null)
                                            {
                                                key.SetValue("BondDeviceCID", challengeID);
                                            }
                                        }
                                    }
                                }));
                            T.Start();
                        }

                        return challenge;
                    }
                }

                public bool Validate(byte[] response)
                {
                    if ((response == null) || (expectedResponse == null))
                    {
                        return false;
                    }
                    if ((response.Length != expectedResponse.Length))
                    {
                        return false;
                    }
                    for (int i = 0; i < response.Length; ++i)
                    {
                        if (response[i] != expectedResponse[i])
                        {
                            return false;
                        }
                    }

                    return true;
                }

                protected byte[] expectedResponse;

                protected byte[] hmac_key;
                protected int challengeID;
            };

            public PersonalBond _personalBond;

            public volatile bool running;

            public StateData()
            {
                running = false;
                bestDeviceMac = null;
                bestDeviceRSSI = SByte.MinValue;

                curPollID = 0;
                _HasCPReturnData = new AutoResetEvent(false);
                CPReturnData = null;
                ForceStateUpdate = new ManualResetEvent(false);
                ResetToWaitForWakeupState = false;
                ble = null;
                device = null;
                COMPort = "";
                logoffRSSI = 0;
                logonRSSI = 0;
                verboseLogging = false;
                verifiedUsername = "";
                verifiedUserMustChangePassword = false;

                isUserMulti = false;
                _hasDeviceEvent = new AutoResetEvent(false);
                _namedPipeDataEvent = new AutoResetEvent(false);
                _connectionUpdateRspEvent = new ManualResetEventSlim(false);
                _connectionUpdateRspSemaphore = new SemaphoreSlim(2);
                _procedureCompletedEvent = new ManualResetEventSlim(false);
                _CPOnlineEvent = new AutoResetEvent(false);
                _ignoredMacAddresses = new List<byte[]>();
                _deviceFilter = new Filter(null, null);
                BeepOnConnect = BeepOnDisconnect = true;
                _personalBond = null;

                _PreEmptive = new PreEmptive();

            }
        };

        public StateData myData; 
        
        public void SwitchToState(State newState)
        {
            if (!_stateDictionary.ContainsKey(newState))
            {
                throw new NotImplementedException();
            }

            State curState = _myState.MyState();
            if (!validStateTransitions.ContainsKey(curState))
            {
                throw new NotImplementedException();
            }

            if (!validStateTransitions[curState].Contains(newState))
            {
                _debugMessage(this.ToString(), "Error : Unhandled SwitchToState." + newState + " from State " + curState);
                eventLogMessage("Error : Unhandled SwitchToState." + newState + " from State " + curState);
                return;
            }
            else
            {
                _myState = _stateDictionary[newState];
            }
        }

        internal bool readConfig()
        {
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\BLE", false))
            {
                if (key != null)
                {
                    if (((byte[])key.GetValue("EventDebug", new byte[] { 0 }))[0] == 1)
                    {
                        useEventViewerDebug = true;
                        eventLogMessage("Running version 2.2.17.1");

                        BLE_API.BLE.Core.Debug.DebugMessageCallback += Debug_DebugMessageCallback;
                        string error;
                        BLE_API.BLE.Core.Debug.RegisterForDebugMessages(myData.ble, out error);
                        if (error != "")
                        {
                            _debugMessage(this.ToString(), "Init Debug Callback returned with : " + error);
                        }
                    }

                    myData.COMPort = (string)key.GetValue("COMPort", null);
                    myData.logoffRSSI = (SByte)((byte[])key.GetValue("LogoffRSSI", new byte[] { 0 }))[0];
                    myData.logonRSSI = (SByte)((byte[])key.GetValue("LogonRSSI", new byte[] { 0 }))[0];
                    object o = key.GetValue("servicelog", null);
                    if (o != null)
                    {
                        if (((byte[])o)[0] == 1)
                        {
                            myData.verboseLogging = true;
                        }
                    }

                    if (myData.COMPort == null)
                    {
                        eventLogMessage("Error - COM Port not set in registry!");
                        return false;
                    }
                    myData._deviceFilter = new StateData.Filter((byte[])key.GetValue("FilterStream", null), this);

                    byte[] deviceMac = (byte[])key.GetValue("BondDeviceMac", null);
                    byte[] hmac_key = (byte[])key.GetValue("BondDeviceH", null);
                    int challengeID = (int)key.GetValue("BondDeviceCID", 0);

                    string user = (string)key.GetValue("BondDeviceUser", null);
                    if (deviceMac == null || hmac_key == null || user == null)
                    {
                        myData._personalBond = null;
                    }
                    else
                    {
                        myData._personalBond = new StateData.PersonalBond(deviceMac, hmac_key, challengeID, user);
                    }

                    myData.isUserMulti = BitConverter.ToBoolean((byte[])key.GetValue("IsUserMulti", new byte[] { 0 }), 0);
                }
            }
            return true;
        }

        void Debug_DebugMessageCallback(BLE.V2_2.MiTokenBLE sender, string message)
        {
            eventLogMessage(string.Format("DEBUG : " + DateTime.Now.ToString("[HH:MM:ss.fff] ") + message), "Mi-Token BLE Verbose");
        }

        internal void InitializeState(IState state)
        {
            State fstate = state.MyState();
            _stateDictionary.Add(fstate, state);
            State[] validTransfers = state.validTransferStates();
            if (!validStateTransitions.ContainsKey(fstate))
            {
                validStateTransitions.Add(fstate, new List<State>());
            }
            foreach (State s in validTransfers)
            {
                validStateTransitions[fstate].Add(s);
            }
        }

        public delegate void debugMessageDelegate(string strSender, string message);
        public event debugMessageDelegate debugMessage;

        public bool useEventViewerDebug = false;

        public void _debugMessage(string strSender, string message)
        {
            if (debugMessage != null)
            {
                debugMessage.Invoke(strSender, message);
            }
            if (useEventViewerDebug)
            {
                eventLogMessage(string.Format("BLE Watcher Message, From {0} : {1}", strSender, message));
            }
        }

        public StateMachine()
        {
            myData = new StateData();
        }

        private string macToString(byte[] mac)
        {
            return string.Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
#if ALLOW_FORM_CONTROLS
        public void populateNode(out System.Windows.Forms.TreeNode stateNode)
        {
            stateNode = new System.Windows.Forms.TreeNode();
            stateNode.Text = "State";
            System.Windows.Forms.TreeNode bestDeviceNode = new System.Windows.Forms.TreeNode();
            stateNode.Nodes.Add(string.Format("State {0}", _myState.MyState().ToString()));
            bestDeviceNode.Text = "Best Device";
            if (myData.bestDeviceMac == null)
            {
                bestDeviceNode.Nodes.Add("NULL");
            }
            else
            {
                bestDeviceNode.Nodes.Add(string.Format("MAC : {0}", macToString(myData.bestDeviceMac)));
                bestDeviceNode.Nodes.Add(string.Format("RSSI : {0}", myData.bestDeviceRSSI));
            }
            stateNode.Nodes.Add(bestDeviceNode);
            stateNode.Nodes.Add(string.Format("BLE : {0}", myData.ble == null ? "NULL" : "SET"));
            stateNode.Nodes.Add(string.Format("DEV : {0}", myData.device == null ? "NULL" : "SET"));
            stateNode.Nodes.Add(string.Format("COM : {0}", myData.COMPort));
            stateNode.Nodes.Add(string.Format("LOGOFF : {0}", myData.logoffRSSI));
            stateNode.Nodes.Add(string.Format("LOGIN : {0}", myData.logonRSSI));
            stateNode.Nodes.Add(string.Format("VERBOSE : {0}", myData.verboseLogging));
            stateNode.Nodes.Add(string.Format("USERNAME : {0}", myData.verifiedUsername));
            System.Windows.Forms.TreeNode bannedMacs = new System.Windows.Forms.TreeNode();
            bannedMacs.Text = "BANNED MACs";
            foreach (byte[] mac in myData._ignoredMacAddresses)
            {
                bannedMacs.Nodes.Add(macToString(mac));
            }
            stateNode.Nodes.Add(bannedMacs);
        }
#endif

        public void Initialize()
        {
            _debugMessage(this.ToString(), "Initializing");

            _eventOverrides = new EventOverrides(this);

            _stateDictionary = new Dictionary<State, IState>();
            validStateTransitions = new Dictionary<State, List<State>>();

            
            InitializeState(new States.AuthenticateToken(this));
            InitializeState(new States.CheckingForUserPassDB(this));
            InitializeState(new States.LookingForToken(this));
            InitializeState(new States.StateInitializing(this));
            InitializeState(new States.WaitingForCPResponse(this));
            InitializeState(new States.WaitingForWakeup(this));
            InitializeState(new States.WatchingToken(this));
            InitializeState(new States.PreemptiveLogin(this));

            //State Table

            myData.ble = new BLE_API.BLE.V2_2.MiTokenBLE();
            _eventOverrides.registerBLEDevice();

            _debugMessage(this.ToString(), "Setting up CommandChain");
            BLE.CommandChain CC = new BLE.CommandChain();
            CC.evt.gap.scan_response = EventOverrides.ccc_evt_gap_scan_response;
            CC.evt.connection.status = EventOverrides.ccc_evt_connection_status;
            CC.rsp.connection.update = EventOverrides.ccc_rsp_connection_update;
            CC.evt.attclient.procedure_completed = EventOverrides.ccc_evt_procedure_completed;
            myData.ble.AppendCommandChain(CC);

            if (readConfig())
            {
                if (myData.ble.Initialize(myData.COMPort, "\\\\.\\pipe\\Mi-TokenBLEV2", true) == BLE.Core.API_RET.BLE_API_SUCCESS)
                {
                    _debugMessage(this.ToString(), "Initialized DLL");
                    eventLogMessage("Initialized DLL");
                    BLE.V2_1.MiTokenBLE_GotMessageCallback += V2_1_MiTokenBLE_GotMessageCallback;
                    myData.ble.Pipe_SetMessageCallback();
                }
            }

            _myState = _stateDictionary[State.Initializing];

            

            passwordHandler = new PasswordHandler();

            //dummy hit the API Servers on a new thread
            Thread dummyAPI = new Thread(new ThreadStart(() =>
                {
                    if (Thread.CurrentThread.Name == null)
                    {
                        Thread.CurrentThread.Name = "Dummy API call Thread";
                    }

                    canHitAPIServer();
                }));
            dummyAPI.Start();            
        }


        public bool canHitAPIServer()
        {
            bool serverHit = false;
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\BLE"))
            {
                if (key != null)
                {
                    String[] vals = key.GetValueNames();                    
                    foreach (string val in vals)
                    {
                        if (key.GetValueKind(val) == RegistryValueKind.MultiString)
                        {
                            string[] serverNames = (string[])key.GetValue(val);
                            foreach (string server in serverNames)
                            {
                                DateTime start = DateTime.Now;
                                string resp = "";
                                try
                                {
                                    resp = MiToken.API.AnonDummyRequest("https://" + server);
                                }
                                catch (System.Net.WebException webEx)
                                {
                                    if (webEx.Status == System.Net.WebExceptionStatus.ProtocolError)
                                    {
                                        if (((System.Net.HttpWebResponse)webEx.Response).StatusCode == System.Net.HttpStatusCode.NotFound)
                                        {
                                        }
                                        else if (((System.Net.HttpWebResponse)webEx.Response).StatusCode == System.Net.HttpStatusCode.BadRequest)
                                        {
                                            //perfect, the server was hit, but told us it was bad (this is the expected result)
                                            serverHit = true;
                                        }
                                        else
                                        {
                                            resp = "Error Code " + ((System.Net.HttpWebResponse)webEx.Response).StatusCode + "\t" + resp;
                                        }

                                    }
                                    resp = "DummyAPI call : Exception : " + webEx.Message;
                                }
                                TimeSpan timeTaken = DateTime.Now - start;
                                _debugMessage(this.ToString(), String.Format("API call finished. Took {0} seconds", timeTaken.TotalSeconds));
                                _debugMessage(this.ToString(), String.Format("JSON String : {0}", resp));
                            }
                        }
                    }
                }
            }
            return serverHit;
        }


        Thread machineThread = null;
        public void StartMachine()
        {
            myData.running = true;
            if (machineThread != null)
            {
                return; //machine is already started
            }
            else
            {
                machineThread = new Thread(new ThreadStart(() =>
                    {
                        if (Thread.CurrentThread.Name == null)
                        {
                            Thread.CurrentThread.Name = "Machine Main Thread";
                        }

                        _debugMessage(this.ToString(), "Thread Started");
                        while (myData.running)
                        {
                            DoProcess();
                        }
                        if (myData.device != null)
                        {
                            myData.device.Disconnect();
                            myData.ble.Pipe_ReleaseExclusiveAccess();
                            myData.device = null;
                        }
                        if (myData.ble != null)
                        {
                            myData.ble.DeleteInstance();
                            myData.ble = null;
                        }
                        _debugMessage(this.ToString(), "Thread Stopped");
                    }));
                machineThread.Start();
            }
        }

        public void StopMachine()
        {
            myData.running = false;
            myData.ForceStateUpdate.Set();
            machineThread = null;
        }

        public void DoProcess()
        {
            _debugMessage(this.ToString(), "Do Process");
            _myState.DoProcess();

            if (myData.ResetToWaitForWakeupState)
            {
                if (myData.device != null)
                {
                    myData.device.Disconnect();
                    myData.ble.Pipe_ReleaseExclusiveAccess();
                    myData.device = null;
                }
                myData.ResetToWaitForWakeupState = false;
                myData._HasCPReturnData.Reset();
                myData.ForceStateUpdate.Reset();
                SwitchToState(State.WaitingForWakeup);
            }
        }

        public enum WaitEventsResult
        {
            Reset = 0,
            Timeout = 1,
            Event = 2,
            Unknown = -1,
        }

        /// <summary>
        /// Waits for a specified event or the myData.ForceStateUpdate Events
        /// </summary>
        /// <param name="eventToWaitFor">The event that we should wait for</param>
        /// <returns>True if the specified event was triggered, false if the myData.ForceStateUpdate event was triggered</returns>
        public WaitEventsResult WaitForEvent(ManualResetEvent eventToWaitFor, int timeout = -1)
        {
            ManualResetEvent[] handles = new ManualResetEvent[] {myData.ForceStateUpdate, eventToWaitFor};
            int retIndex = WaitHandle.WaitAny(handles, timeout);

            switch (retIndex)
            {
                case 0:
                    return WaitEventsResult.Reset;
                case 1:
                    return WaitEventsResult.Event;
                case WaitHandle.WaitTimeout:
                    return WaitEventsResult.Timeout;
            }

            return WaitEventsResult.Unknown;
        }

        public WaitEventsResult WaitForEvent(AutoResetEvent eventToWaitFor, int timeout = -1)
        {
            WaitHandle[] handles = new WaitHandle[] { (WaitHandle)myData.ForceStateUpdate, (WaitHandle)eventToWaitFor };
            int retIndex = WaitHandle.WaitAny(handles, timeout);

            switch (retIndex)
            {
                case 0:
                    return WaitEventsResult.Reset;
                case 1:
                    return WaitEventsResult.Event;
                case System.Threading.WaitHandle.WaitTimeout:
                    return WaitEventsResult.Timeout;
            }

            return WaitEventsResult.Unknown;
        }

        public enum DATA_FLAG : byte
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
            SET_USERNAME = 0x10,
            SET_USERANDPASS = 0x11,
            SET_WATCHER_STAGE = 0x12,

            CP_PASS_CHANGE = 0x20,

        };

        public enum WATCHER_STAGES : byte
        {
            STAGE_LookingForToken = 0x01,
            STAGE_AuthenticatingToken = 0x02,
            STAGE_WaitingForUsername = 0x03,
            STAGE_API_Issue = 0x04,
        };

        public void V2_1_MiTokenBLE_GotMessageCallback(BLE_API.BLE.V2_1.MiTokenBLE instance, byte[] data)
        {
            DATA_FLAG flagByte = (DATA_FLAG)data[0];
            switch (flagByte)
            {
                case DATA_FLAG.CP_ONLINE:
                    if (_myState.MyState() != State.WaitingForWakeup)
                    {
                        myData.ResetToWaitForWakeupState = true;
                        myData.ForceStateUpdate.Set();
                    }
                    myData._CPOnlineEvent.Set();

                    break;

                case DATA_FLAG.STORE_PASSWORD:
                    myData.CPReturnData = data;
                    myData._HasCPReturnData.Set();
                    break;

                case DATA_FLAG.LOGIN_DONE:
                    State cstate = _myState.MyState();
                    if ((cstate != State.WaitingForCPResponse) && (cstate != State.WatchingToken))
                    {
                        _debugMessage(this.ToString(), "Resetting to WaitState due to current state being " + cstate.ToString());
                        myData.ResetToWaitForWakeupState = true;
                        myData.ForceStateUpdate.Set();
                    }
                    break;

                case DATA_FLAG.CP_PASS_CHANGE:
                    {
                        myData.verifiedUserMustChangePassword = true;   
                        myData.ResetToWaitForWakeupState = true;
                        myData.ForceStateUpdate.Set();
                    }
                    break;

                case DATA_FLAG.CHECK_FOR_MAC:
                case DATA_FLAG.LOGIN_ON:
                case DATA_FLAG.PASSWORD_VAL:
                case DATA_FLAG.REQUEST_LOCK:
                case DATA_FLAG.REQUEST_PASSWORD:
                case DATA_FLAG.REQUEST_QUIT:
                
                    break;

                    throw new NotImplementedException();
            }
        }
        
    }
}
