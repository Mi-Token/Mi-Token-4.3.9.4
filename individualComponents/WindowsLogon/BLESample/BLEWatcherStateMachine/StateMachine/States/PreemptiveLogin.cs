using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLEWatcherStateMachine.States
{
    class PreemptiveLogin : BLEWatcherStateMachine.States.LookingForToken
    {
        public PreemptiveLogin(StateMachine myMachine)
            : base(myMachine)
        { }

        public override StateMachine.State MyState()
        {
            return StateMachine.State.PreEmptiveLogin;
        }

        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.WaitingForWakeup, StateMachine.State.AuthenticatingToken, StateMachine.State.LookingForToken };
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }

        public override void DoProcess()
        {
            _debugMessage("Start DoProcess");
            if (BLE_API.BLE.Core.SignalQualityFromRSSI(myMachine.myData.logonRSSI) < 50)
            {
                //LogonRSSI is <50%. Pre-emptive waits would take longer than normal logins, so revert to LookingForToken.
                _debugMessage("Logon Signal Quality is <50% (" + BLE_API.BLE.Core.SignalQualityFromRSSI(myMachine.myData.logonRSSI) + "). Reverting to Looking For Token state");
                myMachine.SwitchToState(StateMachine.State.LookingForToken);
            }

            SendStateMessage();
            myMachine.myData.ble.SetPollID(myMachine.myData.PollID);
            System.Threading.Thread.Sleep(1000);
            StateMachine.WaitEventsResult waitResult = myMachine.WaitForEvent(myMachine.myData._PreEmptive._hasPreEmptiveDevice, 100);
            _debugMessage(this.ToString(), "WR[1] = " + waitResult.ToString());

            while (waitResult != StateMachine.WaitEventsResult.Event)
            {
                _debugMessage("Polling...");
                myMachine.myData.ble.SetPollID(myMachine.myData.PollID);
                System.Threading.Thread.Sleep(1000);
                waitResult = myMachine.WaitForEvent(myMachine.myData._PreEmptive._hasPreEmptiveDevice, 100);
            }

            BLE_API.BLE.Core.Structures.DeviceInfo devInfo = GetBestDevice();
            if (devInfo == null)
            {
                return;
            }

            myMachine.myData.bestDeviceRSSI = devInfo.sRSSI;
            myMachine.myData.bestDeviceMac = devInfo.macAddress;

            myMachine.SwitchToState(StateMachine.State.AuthenticatingToken);
        }
    }
}
