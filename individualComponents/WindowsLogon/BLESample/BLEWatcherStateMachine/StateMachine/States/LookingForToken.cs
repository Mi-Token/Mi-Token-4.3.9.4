using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using BLE_API;

namespace BLEWatcherStateMachine.States
{
    class LookingForToken : StateMachine.IState
    {
        public LookingForToken(StateMachine myMachine)
            : base(myMachine)
        { }
        public override StateMachine.State MyState()
        {
            return StateMachine.State.LookingForToken;
        }

        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.WaitingForWakeup, StateMachine.State.AuthenticatingToken };
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }

        protected bool isBannedMac(byte[] mac)
        {
            bool same = false;
            for (int i = 0; i < myMachine.myData._ignoredMacAddresses.Count; ++i)
            {
                bool id = true;
                for (int j = 0; j < 6 && id; ++j)
                {
                    if (mac[j] == myMachine.myData._ignoredMacAddresses[i][j])
                    {
                        id = false;
                    }
                }

                if (!id)
                {
                    same = true;
                    break;
                }
            }

            return same;
        }

        protected void SendStateMessage()
        {
            myMachine.myData.ble.Pipe_SendMessage(new byte[] { (byte)StateMachine.DATA_FLAG.SET_WATCHER_STAGE, (byte)StateMachine.WATCHER_STAGES.STAGE_LookingForToken }, -1);
        }

        protected BLE.Core.Structures.DeviceInfo GetBestDevice()
        {
            int tokenCount = ble.GetDeviceFoundCount();
            BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
            char bestRSSI = char.MinValue;
            int bestLocation = -1;

            for (int i = 0; i < tokenCount; ++i)
            {
                ble.GetDeviceInfo(i, ref devInfo);
                if ((devInfo.RSSI > bestRSSI) && (devInfo.RSSI > myMachine.myData.logonRSSI) && !isBannedMac(devInfo.macAddress))
                {
                    bestLocation = i;
                    bestRSSI = devInfo.RSSI;
                }
            }
            if (bestLocation == -1)
            {
                //we are done
                return null;
            }

            ble.GetDeviceInfo(bestLocation, ref devInfo);
            return devInfo;
        }

        public override void DoProcess()
        {
            _debugMessage(this.ToString(), "Start DoProcess");
            SendStateMessage();
            myMachine.myData.ble.SetPollID(myMachine.myData.PollID);
            System.Threading.Thread.Sleep(1000);
            StateMachine.WaitEventsResult waitResult = myMachine.WaitForEvent(myMachine.myData._hasDeviceEvent, 100);
            _debugMessage(this.ToString(), "WR[1] = " + waitResult.ToString());
            switch(waitResult)
            {
                case StateMachine.WaitEventsResult.Event:
                    {
                        BLE.Core.Structures.DeviceInfo devInfo = GetBestDevice();
                        if (devInfo == null)
                        {
                            //we are done
                            return;
                        }
                        myMachine.myData.bestDeviceRSSI = devInfo.sRSSI;
                        myMachine.myData.bestDeviceMac = devInfo.macAddress;

                        myMachine.SwitchToState(StateMachine.State.AuthenticatingToken);
                    }
                    break;
                case StateMachine.WaitEventsResult.Reset:
                    break;
                case StateMachine.WaitEventsResult.Timeout:
                    {
                        //We have found no tokens, so change to use first token found mode
                        waitResult = myMachine.WaitForEvent(myMachine.myData._hasDeviceEvent);
                        _debugMessage(this.ToString(), "WR[2] = " + waitResult.ToString());
                        myMachine.SwitchToState(StateMachine.State.AuthenticatingToken);
                    }
                    break;;
                case StateMachine.WaitEventsResult.Unknown:
                    break;
            }
            
        }


    }
}
