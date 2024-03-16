using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BLE_API;
using MiToken;
using System.Threading;
using Microsoft.Win32;



namespace BLEWatcherStateMachine.States
{
    class AuthenticateToken : StateMachine.IState
    {
        public AuthenticateToken(StateMachine myMachine)
            : base(myMachine)
        { }

        public override StateMachine.State MyState()
        {
            return StateMachine.State.AuthenticatingToken;
        }

        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.WaitingForWakeup, StateMachine.State.CheckingForUserPassDB , StateMachine.State.LookingForToken, StateMachine.State.PreEmptiveLogin};
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }

        enum DeviceVersion
        {
            DevVersion_Unknown,
            DevVersion1_0,
            DevVersion1_1,
            DevVersion1_2,
        };

        enum verifyResult
        {
            Success,
            Failure,
            OtherError,
            PersonalBondError,
            Timeout,
            API_Error,
        };

        const int tokenTimeout = 1000;

        private void tokenValidated(bool tokenIsInRange, string tokenMacString, string tokenBoundUsername)
        {
            if (tokenIsInRange)
            {
                myMachine.SwitchToState(StateMachine.State.CheckingForUserPassDB);
                myMachine.myData.verifiedUsername = tokenBoundUsername;
                return;
            }
            else
            {
                myMachine.myData._PreEmptive.TokenValidated(tokenMacString,tokenBoundUsername);
                myMachine.SwitchToState(StateMachine.State.PreEmptiveLogin);
                device.Disconnect();
                return;
            }
        }

        private verifyResult verifyToken(StateMachine.StateData.PersonalBond myBond, out bool tokenIsInRange)
        {
            _debugMessage("In Verify Token");
            myMachine.myData.ble.Pipe_SendMessage(new byte[] { (byte)StateMachine.DATA_FLAG.SET_WATCHER_STAGE, (byte)StateMachine.WATCHER_STAGES.STAGE_AuthenticatingToken }, -1);

            tokenIsInRange = false;
            int devCount = ble.GetDeviceFoundCount();
            BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
            bool gotAddress = false;
            for (int i = 0; i < devCount; ++i)
            {
                ble.GetDeviceInfo(i, ref devInfo);
                bool sameAddress = true;
                for (int j = 0; (j < 6) && (sameAddress); ++j)
                {
                    if (devInfo.macAddress[j] != myMachine.myData.bestDeviceMac[j])
                    {
                        sameAddress = false;
                    }
                }
                if (sameAddress)
                {
                    _debugMessage("Token Found");
                    gotAddress = true;
                    break;
                }
            }

            if (gotAddress)
            {
                if (myBond != null && !myBond.CorrectDevice(devInfo.macAddress))
                {
                    myBond = null;
                }

                ManualResetEvent gotHash = new ManualResetEvent(false);
                string serverName = "";
                string hash = "", UID = "";
                byte[] binHash = null;
                bool useAPI = false;
                bool noSuchToken = false;

                string MacString = devInfo.MacString;

                Thread hashThread = new Thread(new ThreadStart(() =>
                {
                    _debugMessage("Starting HASH Gettting Thread");
                    if (myBond != null)
                    {
                        _debugMessage("Using Personal Bond");
                        hash = myBond.NextChallenge;
                        binHash = Convert.FromBase64String(hash);
                        useAPI = true;
                        UID = "";
                    }
                    else
                    {
                        _debugMessage("Using Server Bond");
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
                                            serverName = "https://" + server;
                                            DateTime start = DateTime.Now;
                                            string resp = "";
                                            try
                                            {
                                                resp = MiToken.API.AnonGenerateBLEHash(serverName, devInfo.macString());
                                            }
                                            catch (System.Net.WebException)
                                            {
                                                continue;
                                            }
                                            TimeSpan timeTaken = DateTime.Now - start;
                                            _debugMessage(String.Format("API call finished. Took {0} seconds", timeTaken.TotalSeconds));
                                            _debugMessage(String.Format("JSON String : {0}", resp));
                                            try
                                            {
                                                Dictionary<string, string> json = MiToken.API.ParseAPIJSON(resp);
                                                if (json.ContainsKey("result") && json["result"] == "success")
                                                {
                                                    if (json.ContainsKey("hash") && json.ContainsKey("UID"))
                                                    {
                                                        hash = json["hash"];
                                                        hash = hash.Replace("-", "+").Replace("_", "/");
                                                        UID = json["UID"];
                                                        _debugMessage(this.ToString(), string.Format("Got hash of {0} and UID of {1}", hash, UID));

                                                        binHash = Convert.FromBase64String(hash);
                                                        StringBuilder hexHash = new StringBuilder();
                                                        foreach (byte b in binHash)
                                                        {
                                                            hexHash.Append(string.Format("{0:X2}", b));
                                                        }
                                                        _debugMessage(String.Format("Bin Hash Is : {0}", hexHash.ToString()));
                                                        useAPI = true;
                                                        break;
                                                    }
                                                }

                                            }
                                            catch (FormatException)
                                            {
                                                //invalid JSON string
                                                continue;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    gotHash.Set();
                }
                ));
                hashThread.Start();

                _debugMessage("Getting Pipe Exclusive Access");
                bool hashVersion1_1 = false;
                BLE.Core.API_NP_RET requestExclusive = ble.Pipe_RequestExclusiveAccess();
                if (requestExclusive != BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                {
                    //we could already have exclusive access, let's try releasing and getting it back
                    if (ble.Pipe_ReleaseExclusiveAccess() == BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                    {
                        //we did have access, re-request it
                        if (ble.Pipe_RequestExclusiveAccess() == BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                        {
                            requestExclusive = BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS;
                        }
                    }

                    //we don't have exclusive, and the release/re-request block did not give it to us. Assume someone else must have it
                    if (requestExclusive != BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                    {
                        return verifyResult.OtherError;
                    }
                }
                //bool waitForExclusive = true; //no need, we are the server
                ble.Pipe_SyncWaitForExclusive();
                if (ble.Pipe_HasExclusiveAccess() == BLE.Core.API_NP_RET.BLE_API_EXCLUSIVE_REJECTED)
                {
                    return verifyResult.OtherError;
                }

                if (device != null)
                {
                    device.Disconnect();
                    device = null;
                }

                _debugMessage("Connecting To Device");
                device = ble.connectToDevice(devInfo);
                if (device.SyncWaitForConnection(7000) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                {
                    //failed to connect to the device within 7 second(s)
                    if (device.TimeoutRequest() == BLE.Core.CONN_RET.BLE_CONN_ERR_CONNECTION_TIMED_OUT)
                    {
                        //we have successfully requested the timeout
                        device.Disconnect();
                        return verifyResult.Timeout;
                    }
                }

                _debugMessage("Connected");

                // Start asynchronous call to wait for residual events to complete
                myMachine.myData._connectionUpdateRspEvent.Reset();
                {
                    bool asyncThreadResult = false;
                    ManualResetEventSlim asyncThreadEvent = new ManualResetEventSlim(false);
                    Thread asyncThread = new Thread(new ThreadStart(() =>
                      {
                          asyncThreadResult = myMachine.myData._procedureCompletedEvent.Wait(2000);
                          asyncThreadEvent.Set();
                      }));
                    asyncThread.Start();
                    asyncThreadEvent.Wait();
                }

                _debugMessage(String.Format("Setting Connection Update Parameters"));
                // Start asynchronous call to wait for connection update response semaphore                
                {
                    bool asyncThreadResult = false;
                    ManualResetEventSlim asyncThreadEvent = new ManualResetEventSlim(false);
                    Thread asyncThread = new Thread(new ThreadStart(() =>
                    {
                        asyncThreadResult = myMachine.myData._connectionUpdateRspSemaphore.Wait(1000);
                        asyncThreadEvent.Set();
                    }));
                    asyncThread.Start();
                    asyncThreadEvent.Wait();
                    if (!asyncThreadResult)
                    {
                        _debugMessage("Failed to receive connection response semaphore");
                        return verifyResult.Timeout;
                    }
                }

                // Manually reset event before making conneciton update call
                myMachine.myData._connectionUpdateRspEvent.Reset();

                // Send connection update call
                device.UpdateConnectionParameters(30, 30, 50, 0);

                // Start asynchronous call to wait for connection update response event
                {
                    bool asyncThreadResult = false;
                    ManualResetEventSlim asyncThreadEvent = new ManualResetEventSlim(false);
                    Thread asyncThread = new Thread(new ThreadStart(() =>
                    {
                        asyncThreadResult = myMachine.myData._connectionUpdateRspEvent.Wait(1000);
                        asyncThreadEvent.Set();
                    }));
                    asyncThread.Start();
                    asyncThreadEvent.Wait();
                    if (!asyncThreadResult)
                    {
                        _debugMessage("Failed to receive connection response event");
                        return verifyResult.Timeout;
                    }
                }

                // Release connection update response semaphore before continuing
                myMachine.myData._connectionUpdateRspSemaphore.Release();

                _debugMessage(String.Format("Scanning for service"));
                DeviceVersion versionID = DeviceVersion.DevVersion_Unknown;
                if (!getDeviceVersion(device, ref versionID))
                {
                    return verifyResult.Timeout;
                }
                byte[] readHash;

                if (versionID == DeviceVersion.DevVersion_Unknown)
                {
                    return verifyResult.Failure;
                }
                else if (versionID == DeviceVersion.DevVersion1_1)
                {
                    hashVersion1_1 = true;
                }
                else if (versionID == DeviceVersion.DevVersion1_2)
                {
                    hashVersion1_1 = true;
                }

                UInt16 uuid_hash_service = (hashVersion1_1 ? BLE.Core.UUIDs.HASH.ServiceUUID : BLE.Core.UUIDs.HASH_OLD.ServiceUUID);
                UInt16 uuid_hash_buffer = (hashVersion1_1 ? BLE.Core.UUIDs.HASH.BUFFER : BLE.Core.UUIDs.HASH_OLD.BUFFER);
                UInt16 uuid_hash_status = (hashVersion1_1 ? BLE.Core.UUIDs.HASH.STATUS : BLE.Core.UUIDs.HASH_OLD.STATUS);

                if (device.SyncScanServicesInRange(uuid_hash_service, 0xFFFF, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                {
                    return verifyResult.Timeout;
                }

                byte[] fullHash = null;
                //we need to have the hash by now
                if (myMachine.WaitForEvent(gotHash) == StateMachine.WaitEventsResult.Reset)
                {
                    return verifyResult.OtherError;
                }

                sbyte devRSSI;
                devRSSI = myMachine.myData.bestDeviceRSSI;
                if (devRSSI >= myMachine.myData.logonRSSI)
                {
                    tokenIsInRange = true;
                }

                if (tokenIsInRange)
                {
                    //Beep the token
                    if (myMachine.myData.BeepOnConnect)
                    {
                        if (device.SyncScanServicesInRange(BLE.Core.UUIDs.SECURE_BOND.ServiceUUID, 0xFFFF, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                        {
                            return verifyResult.Timeout;
                        }
                        if (device.SyncSetAttribute(BLE.Core.UUIDs.SECURE_BOND.CONFIG, new byte[] { 0x04 /*PING Command*/ }, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                        {
                            return verifyResult.Timeout;
                        }
                    }
                }
                gotHash.WaitOne();
                if (noSuchToken)
                {
                    //there is no such token in the API, return a failure so this token is added to the ignore list
                    return verifyResult.Failure;
                }
                if (!useAPI)
                {
                    //we haven't got a valid challenge to hash, so just fail
                    return verifyResult.API_Error;
                }
                switch (versionID)
                {
                    case DeviceVersion.DevVersion1_0:
                        fullHash = binHash;
                        if (myBond != null)
                        {
                            //cannot use myBond with a DevVersion1_0 device
                            return verifyResult.PersonalBondError;
                        }
                        break;
                    case DeviceVersion.DevVersion1_1:
                        fullHash = new byte[16 + 20];
                        for (int pos = 0; pos < 16; pos += 6)
                        {
                            Array.Copy(devInfo.macAddress, 0, fullHash, pos, 6);
                        }
                        if (myBond != null)
                        {
                            fullHash[15]++; //add one (and wrap around) to the last digit of the pseudo UID
                        }
                        Array.Copy(binHash, 0, fullHash, 16, 20);
                        break;
                }

                _debugMessage("Setting HASH");
                if (device.SyncSetAttribute(uuid_hash_buffer, fullHash, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                {
                    byte lastError = device.GetLastError();
                    _debugMessage("Setting hash Failed, Internal Error Code = " + lastError.ToString());
                    return verifyResult.Timeout;
                }
                _debugMessage("Getting HASH");
                if (device.SyncReadAttribute(uuid_hash_buffer, true, out readHash, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                {
                    return verifyResult.Timeout;
                }

                _debugMessage("Getting RSSI");
                

                _debugMessage("Validating HASH");
                string b64outhash = Convert.ToBase64String(readHash);
                if (useAPI)
                {
                    if (myBond != null)
                    {
                        _debugMessage("Validating Local Bond");
                        if (myBond.Validate(readHash))
                        {
                            tokenValidated(tokenIsInRange, MacString, myBond.Username);
                            return verifyResult.Success;
                        }
                        else
                        {
                            return verifyResult.PersonalBondError;
                        }
                    }
                    else
                    {
                        _debugMessage("Validating Server Bond");
                        b64outhash = b64outhash.Replace("/", "_").Replace("+", "-");
                        _debugMessage(this.ToString(), string.Format("Returning response of {0} with UID of {1}", b64outhash, UID));
                        DateTime start = DateTime.Now;
                        string verifyResponse = API.AnonVerifyBLEHashResponse(serverName, devInfo.macString(), UID, b64outhash);
                        TimeSpan timeTaken = DateTime.Now - start;
                        _debugMessage(String.Format("API Request took {0} seconds", timeTaken.TotalSeconds));
                        _debugMessage(String.Format("JSON String : {0}", verifyResponse));
                        Dictionary<string, string> veriJson = new Dictionary<string, string>();
                        try
                        {
                            veriJson = MiToken.API.ParseAPIJSON(verifyResponse);
                        }
                        catch (FormatException)
                        {
                            //bad JSON string
                            return verifyResult.OtherError;
                        }
                        if (veriJson.ContainsKey("result") && (veriJson["result"] == "success"))
                        {
                            if (veriJson.ContainsKey("username"))
                            {
                                _debugMessage("Successful Verification");
                                tokenValidated(tokenIsInRange, MacString, veriJson["username"]);
                                return verifyResult.Success;
                            }
                        }
                        else
                        {
                            return verifyResult.Failure;
                        }
                    }
                }
            }
            return verifyResult.OtherError;
        }

        private BLE.Core.Structures.DeviceInfo GetDeviceInfo(byte[] address)
        {
            int devCount = ble.GetDeviceFoundCount();
            BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
            bool gotAddress = false;
            for (int i = 0; i < devCount; ++i)
            {
                ble.GetDeviceInfo(i, ref devInfo);
                bool sameAddress = true;
                for (int j = 0; (j < 6) && (sameAddress); ++j)
                {
                    if (devInfo.macAddress[j] != address[j])
                    {
                        sameAddress = false;
                    }
                }
                if (sameAddress)
                {
                    _debugMessage("Token Found");
                    gotAddress = true;
                    break;
                }
            }

            if (gotAddress)
            {
                return devInfo;
            }
            return null;
        }

        private void connectAndBeep(byte[] address)
        {
            BLE.Core.API_NP_RET requestExclusive = ble.Pipe_RequestExclusiveAccess();
            if (requestExclusive != BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
            {
                if (ble.Pipe_ReleaseExclusiveAccess() == BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                {
                    if (ble.Pipe_RequestExclusiveAccess() == BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                    {
                        requestExclusive = BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS;
                    }
                }

                if (requestExclusive != BLE.Core.API_NP_RET.BLE_API_NP_SUCCESS)
                {
                    return;
                }
            }

            ble.Pipe_SyncWaitForExclusive();
            if (ble.Pipe_HasExclusiveAccess() == BLE.Core.API_NP_RET.BLE_API_EXCLUSIVE_REJECTED)
            {
                return;
            }
            if (device != null)
            {
                device.Disconnect();
                device = null;
            }

            _debugMessage("Connecting To Device (for beeping)");
            BLE.Core.Structures.DeviceInfo devInfo = GetDeviceInfo(address);
            device = ble.connectToDevice(devInfo);
            if (device.SyncWaitForConnection(7000) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
            {
                //failed to connect to the device within 7 second(s)
                if (device.TimeoutRequest() == BLE.Core.CONN_RET.BLE_CONN_ERR_CONNECTION_TIMED_OUT)
                {
                    //we have successfully requested the timeout
                    device.Disconnect();
                    return;
                }
            }

            _debugMessage("Connected");
            myMachine.myData._connectionUpdateRspEvent.Reset();
            {
                bool asyncThreadResult = false;
                ManualResetEventSlim asyncThreadEvent = new ManualResetEventSlim(false);
                Thread asyncThread = new Thread(new ThreadStart(() =>
                {
                    asyncThreadResult = myMachine.myData._procedureCompletedEvent.Wait(2000);
                    asyncThreadEvent.Set();
                }));
                asyncThread.Start();
                asyncThreadEvent.Wait();
            }
            _debugMessage(String.Format("Setting Connection Update Parameters"));
            // Start asynchronous call to wait for connection update response semaphore                
            {
                bool asyncThreadResult = false;
                ManualResetEventSlim asyncThreadEvent = new ManualResetEventSlim(false);
                Thread asyncThread = new Thread(new ThreadStart(() =>
                {
                    asyncThreadResult = myMachine.myData._connectionUpdateRspSemaphore.Wait(1000);
                    asyncThreadEvent.Set();
                }));
                asyncThread.Start();
                asyncThreadEvent.Wait();
                if (!asyncThreadResult)
                {
                    _debugMessage("Failed to receive connection response semaphore");
                    return;
                }
            }

            // Manually reset event before making conneciton update call
            myMachine.myData._connectionUpdateRspEvent.Reset();

            // Send connection update call
            device.UpdateConnectionParameters(30, 30, 50, 0);

            // Start asynchronous call to wait for connection update response event
            {
                bool asyncThreadResult = false;
                ManualResetEventSlim asyncThreadEvent = new ManualResetEventSlim(false);
                Thread asyncThread = new Thread(new ThreadStart(() =>
                {
                    asyncThreadResult = myMachine.myData._connectionUpdateRspEvent.Wait(1000);
                    asyncThreadEvent.Set();
                }));
                asyncThread.Start();
                asyncThreadEvent.Wait();
                if (!asyncThreadResult)
                {
                    _debugMessage("Failed to receive connection response event");
                    return ;
                }
            }

            // Release connection update response semaphore before continuing
            myMachine.myData._connectionUpdateRspSemaphore.Release();
            _debugMessage(String.Format("Scanning for service"));

            if (myMachine.myData.BeepOnConnect)
            {
                if (device.SyncScanServicesInRange(BLE.Core.UUIDs.SECURE_BOND.ServiceUUID, 0xFFFF, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                {
                    return;
                }
                if (device.SyncSetAttribute(BLE.Core.UUIDs.SECURE_BOND.CONFIG, new byte[] { 0x04 /*PING Command*/ }, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                {
                    return;
                }
            }

            return;
        }

        public override void DoProcess()
        {
            _debugMessage(this.ToString(), "Start DoProcess");

            string MacString = BLE.Core.MacStringFromAddressBytes(myMachine.myData.bestDeviceMac);
            if (myMachine.myData._PreEmptive.TokenHasBeenValidated(MacString))
            {
                _debugMessage("Token was already validated at an earlier point in time, and is still considered valid. Skipping Authentication check");
                //This device has already been validated. Therefore the reason we are here is that it now has been put in range.
                connectAndBeep(myMachine.myData.bestDeviceMac);
                myMachine.SwitchToState(StateMachine.State.CheckingForUserPassDB);
                myMachine.myData.verifiedUsername = myMachine.myData._PreEmptive.TokensUser(MacString);
                return;
                                
            }

            bool tokenIsInRange;
            verifyResult result = verifyToken(myMachine.myData._personalBond, out tokenIsInRange);
            _debugMessage(this.ToString(), "Verify Token Result=" + result);
            if (result != verifyResult.Success)
            {
                if (result == verifyResult.Failure)
                {
                    _debugMessage("A Token Has Been Ignored!");
                    myMachine.myData._ignoredMacAddresses.Add(myMachine.myData.bestDeviceMac);
                }

                if (result == verifyResult.PersonalBondError)
                {
                    _debugMessage("The Token's Personal Bond Failed, removing PersonalBonding information");
                    myMachine.myData._personalBond = null;
                }
                if (result == verifyResult.API_Error)
                {
                    _debugMessage("Returned API Error : Validating that we can reach an API Server");
                    if (!myMachine.canHitAPIServer())
                    {
                        myMachine.myData.ble.Pipe_SendMessage(new byte[] { (byte)StateMachine.DATA_FLAG.SET_WATCHER_STAGE, (byte)StateMachine.WATCHER_STAGES.STAGE_API_Issue }, -1);
                        Thread.Sleep(30000); //Sleep for 30 seconds before we try again
                    }
                }
                
                myMachine.SwitchToState(StateMachine.State.LookingForToken);
                if (device != null)
                {
                    device.Disconnect();
                }
                ble.Pipe_ReleaseExclusiveAccess();
            }

        }

        bool getDeviceVersion(BLE.V2_2.Device device, ref DeviceVersion versionID)
        {
            if (device.SyncScanServicesInRange(BLE.Core.UUIDs.DEVICE_INFO.ServiceUUID, 0xFFFF, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
            {
                return false;
            }
            byte[] firmwareData;
            if (device.SyncReadAttribute(BLE.Core.UUIDs.DEVICE_INFO.FIRMWARE_VERSION, true, out firmwareData, tokenTimeout) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
            {
                return false;
            }
            if (firmwareData.Length == 0)
            {
                return false;
            }
            string firmwareString = Encoding.ASCII.GetString(firmwareData);
            string versionSubstring = firmwareString;
            if (versionSubstring.Contains('_'))
            {
                versionSubstring = versionSubstring.Substring(0, versionSubstring.IndexOf('_'));
            }

            if (versionSubstring.ToLower().Equals("prox-logon"))
            {
                versionID = DeviceVersion.DevVersion1_0;
            }
            else if (versionSubstring.ToLower().Equals("blu"))
            {
                versionID = DeviceVersion.DevVersion1_1;
            }
            else
            {
                versionID = DeviceVersion.DevVersion_Unknown;
            }

            return true;
        }


    }
}
