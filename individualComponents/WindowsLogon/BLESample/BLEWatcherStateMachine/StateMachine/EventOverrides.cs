using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BLE_API;


namespace BLEWatcherStateMachine
{
    public class EventOverrides
    {
        internal StateMachine myMachine;

        public EventOverrides(StateMachine myMachine)
        {
            this.myMachine = myMachine;
            
        }

        public void registerBLEDevice()
        {
            if (myMachine.myData.ble != null)
            {
                overrideDictionary.Add(myMachine.myData.ble, this);
            }
        }

        static internal Dictionary<BLE.V2_2.MiTokenBLE, EventOverrides> overrideDictionary = new Dictionary<BLE.V2_2.MiTokenBLE, EventOverrides>();

        public static BLE.CommandChain.CommandChainCall ccc_evt_gap_scan_response = (IntPtr caller, IntPtr data) =>
        {
            BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
            if (bleObject == null)
            {
                return;
            }
            if (overrideDictionary.ContainsKey(bleObject))
            {
                overrideDictionary[bleObject].loc_ccc_evt_gap_scan_response(caller, data);
            }
        };

        public static BLE.CommandChain.CommandChainCall ccc_evt_connection_status = (IntPtr caller, IntPtr data) =>
            {
                BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
                if (bleObject == null)
                {
                    return;
                }
                if (overrideDictionary.ContainsKey(bleObject))
                {
                    overrideDictionary[bleObject].loc_ccc_evt_connection_status(caller, data);
                }
            };


        public static BLE.CommandChain.CommandChainCall ccc_rsp_connection_update = (IntPtr caller, IntPtr data) =>
            {
                BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
                if (bleObject == null)
                {
                    return;
                }
                if (overrideDictionary.ContainsKey(bleObject))
                {
                    overrideDictionary[bleObject].loc_ccc_rsp_connection_update(caller, data);
                }
            };

        public static BLE.CommandChain.CommandChainCall ccc_evt_procedure_completed = (IntPtr caller, IntPtr data) =>
            {
                BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
                if (bleObject == null)
                {
                    return;
                }
                if (overrideDictionary.ContainsKey(bleObject))
                {
                    overrideDictionary[bleObject].loc_ccc_evt_procedure_completed(caller, data);
                }
            };        

        public void loc_ccc_evt_connection_status(IntPtr caller, IntPtr data)
        {
            BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.STATUS msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.STATUS(data);
            //myMachine._debugMessage(this.ToString(), msg.__debugString);
            if (msg.flags == 1 && myMachine.myData.device == null)
            {
                //we have a connection and we shouldn't
                myMachine._debugMessage(this.ToString(), "Found a connection when we shouldn't have one. Forcing an Unsafe Disconnection");
                myMachine.myData.ble.ForceUnsafeDisconnection(msg.connection);
            }
        }

        public void loc_ccc_evt_gap_scan_response(IntPtr caller, IntPtr data)
        {
            
            BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE(data);
            //myMachine._debugMessage(this.ToString(), msg.__debugString);
            if (myMachine.currentState == null)
            {
                return;
            }

            if ((myMachine.currentState.MyState() == StateMachine.State.LookingForToken) ||
                (myMachine.currentState.MyState() == StateMachine.State.PreEmptiveLogin))
            {
                bool same = false;
                foreach (byte[] addr in myMachine.myData._ignoredMacAddresses)
                {
                    bool innerSame = true;
                    for (int i = 0; i < 6 && innerSame; ++i)
                    {
                        if (addr[i] != msg.sender[i])
                        {
                            innerSame = false;
                        }
                    }

                    if (innerSame)
                    {
                        same = true;
                        break;
                    }
                }

                if (same)
                {
                    //device is in the banned mac address list
                    return;
                }

                if (myMachine.useEventViewerDebug)
                {
                    myMachine.eventLogMessage(string.Format("Found device [{0}] with RSSI of {1} vs {2} [{3}]. FilterPass = {4}", BitConverter.ToString(msg.sender), msg.rssi, myMachine.myData.logonRSSI, msg.rssi > myMachine.myData.logonRSSI ? "TRUE" : "FALSE", myMachine.myData._deviceFilter.devicePassesFilter(msg.sender, msg.data) ? "TRUE" : "FALSE"), "Mi-Token BLE Verbose Token");
                }
                if (BLE.Core.SignalQualityFromRSSI(msg.rssi) >= 50) //pre-emptive login level is ALWAYS 50%
                {
                    if (!myMachine.myData._PreEmptive.TokenHasBeenValidated(BLE.Core.MacStringFromAddressBytes(msg.sender)))
                    {
                        //The Token is close enough and is not currently considered valid.
                        if (myMachine.myData._deviceFilter.devicePassesFilter(msg.sender, msg.data))
                        {
                            myMachine.myData.bestDeviceRSSI = msg.rssi;
                            myMachine.myData.bestDeviceMac = msg.sender;
                            myMachine.myData._PreEmptive._hasPreEmptiveDevice.Set();
                        }
                    }
                }
                if (msg.rssi > myMachine.myData.logonRSSI)
                {
                    if (myMachine.myData._deviceFilter.devicePassesFilter(msg.sender, msg.data))
                    {
                        //if (msg.rssi > myMachine.myData.bestDeviceRSSI)
                        {
                            //myMachine._debugMessage(this.ToString(), string.Format("Got Resp from a device within range, (Range {0} vs {1})", msg.rssi, myMachine.myData.logonRSSI));
                            myMachine.myData.bestDeviceRSSI = msg.rssi;
                            myMachine.myData.bestDeviceMac = msg.sender;
                            myMachine.myData._hasDeviceEvent.Set();
                            myMachine.myData._PreEmptive._hasPreEmptiveDevice.Set();
                        }
                    }
                }
            }

            return;
        }

        public void loc_ccc_rsp_connection_update(IntPtr caller, IntPtr data)
        {
            myMachine.myData._connectionUpdateRspSemaphore.Wait();
            var msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.UPDATE(data);

            if (msg.result == 0 && myMachine.myData.device != null)
            {
                myMachine.myData._connectionUpdateRspEvent.Set();
            }
            myMachine.myData._connectionUpdateRspSemaphore.Release();
        }

        public void loc_ccc_evt_procedure_completed(IntPtr caller, IntPtr data)
        {
            var msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.PROCEDURE_COMPLETED(data);
            
            if (myMachine.myData.device != null)
            {
                myMachine.myData._procedureCompletedEvent.Set();
            }
        }

    }
}
