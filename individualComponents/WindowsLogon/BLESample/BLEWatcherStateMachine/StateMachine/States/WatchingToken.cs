using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BLE_API;


namespace BLEWatcherStateMachine.States
{
    class WatchingToken : StateMachine.IState
    {
        public WatchingToken(StateMachine myMachine)
            : base(myMachine)
        { }
        public override StateMachine.State MyState()
        {
            return StateMachine.State.WatchingToken;
        }

        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.WaitingForWakeup };
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }

        const int tokenTimeout = 1000;
        public override void DoProcess()
        {
            _debugMessage(this.ToString(), "Start DoProcess");

            System.Threading.ManualResetEvent are = new System.Threading.ManualResetEvent(false);
            int failCount = 0;
            while (true)
            {
                myMachine.myData.ble.SetPollID(myMachine.myData.PollID);
                StateMachine.WaitEventsResult result = StateMachine.WaitEventsResult.Unknown;
                for (int i = 0; i < 5; ++i)
                {
                    myMachine.myData.device.PollRSSI();
                    result = myMachine.WaitForEvent(are, 100);
                }

                switch (result)
                {
                    case StateMachine.WaitEventsResult.Unknown:
                        return;
                    case StateMachine.WaitEventsResult.Reset:
                        return;
                    case StateMachine.WaitEventsResult.Event: //how did the event get triggered :/
                        return;
                    case StateMachine.WaitEventsResult.Timeout:
                        //don't return, this is our expected path
                        break;
                }

                byte rssi;
                myMachine.myData.device.GetRssi(out rssi);
                myMachine.myData.bestDeviceRSSI = (SByte)rssi;
                if ((SByte)rssi < myMachine.myData.logoffRSSI || (rssi == 0))
                {
                    failCount++;
                }
                else
                {
                    failCount = 0;
                }

                if(failCount >= 2)
                {
                    _debugMessage(this.ToString(), "Sending timeout message");

                    //logoff time - send the disconnect request first
                    ByteBuilder bb = new ByteBuilder();
                    bb.Add((byte)StateMachine.DATA_FLAG.REQUEST_LOCK);
                    bb.Add(BitConverter.GetBytes(myMachine.myData.isUserMulti));
                    myMachine.myData.ble.Pipe_SendMessage(bb.ToArray, -1);
                    
                    //now send beeps
                    if (myMachine.myData.BeepOnDisconnect)
                    {
                        if (device.SyncScanServicesInRange(BLE.Core.UUIDs.SECURE_BOND.ServiceUUID, 0xFFFF, tokenTimeout) != BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                        {
                            // Attempt to send Ping V2 command
                            MiTokenBLEWatcher.PingBurstCfg pingBurstCfg = new MiTokenBLEWatcher.PingBurstCfg();
                            pingBurstCfg.DurationMs = 100;
                            pingBurstCfg.IntervalMs = 150;
                            pingBurstCfg.AddPing(4096); // High tone
                            pingBurstCfg.AddPing(4096); // High tone

                            ByteBuilder bbPing = new ByteBuilder();
                            bbPing.Add(0x05 /*PING V2 Command*/);
                            bbPing.Add(pingBurstCfg.Serialize());
                            var pingV2Result = device.SyncSetAttribute(BLE.Core.UUIDs.SECURE_BOND.CONFIG, bbPing.ToArray, tokenTimeout);

                            // If Ping V2 command fails then use Ping V1; assuming arguments are valid
                            if (device.GetLastError() != 0 || pingV2Result != BLE.Core.CONN_RET.BLE_CONN_SUCCESS)
                            {
                                // Ping device
                                device.SyncSetAttribute(BLE.Core.UUIDs.SECURE_BOND.CONFIG, new byte[] { 0x04 /*PING Command*/ }, tokenTimeout);
                                System.Threading.Thread.Sleep(100);
                                device.SyncSetAttribute(BLE.Core.UUIDs.SECURE_BOND.CONFIG, new byte[] { 0x04 /*PING Command*/ }, tokenTimeout);
                            }
                        }                        
                    }                    
                    
                    //and lastly disconnect from the device
                    myMachine.myData.device.Disconnect();
                    myMachine.myData.ble.Pipe_ReleaseExclusiveAccess();
                    myMachine.SwitchToState(StateMachine.State.WaitingForWakeup);
                    return;
                }
            }
        }
    }
}
