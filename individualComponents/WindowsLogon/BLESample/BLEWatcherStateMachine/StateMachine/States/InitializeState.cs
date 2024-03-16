using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

using BLE_API;


namespace BLEWatcherStateMachine.States
{
    public class StateInitializing : BLEWatcherStateMachine.StateMachine.IState
    {
        public StateInitializing(StateMachine myMachine)
            : base(myMachine)
        {

        }
        public override StateMachine.State MyState()
        {
            return StateMachine.State.Initializing;
        }


        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.WaitingForWakeup };
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }

        public override void DoProcess()
        {
            _debugMessage(this.ToString(), "Start DoProcess");
            if (ble.ConnectedToCOM() == false)
            {
                if (ble.COMOwnerKnown())
                {
                    _debugMessage("Waiting for COM To be free");
                    ble.WaitForCOMToBeFree();
                    _debugMessage("Trying to reinit COM");
                    ble.ReinitCOM();
                }
                else
                {
                    Thread.Sleep(1000);
                    _debugMessage("Trying to reinit COM");
                    BLE_API.BLE.Core.API_RET ret = ble.ReinitCOM(); //we don't know who has it, but maybe it is free now.
                    _debugMessage("Returned with " + ret.ToString());
                    
                }
            }
            else
            {
                myMachine.SwitchToState(StateMachine.State.WaitingForWakeup);
            }
        }

        



        

    }
}
