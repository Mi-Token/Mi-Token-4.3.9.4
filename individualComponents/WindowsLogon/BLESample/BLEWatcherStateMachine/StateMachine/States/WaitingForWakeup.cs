using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLEWatcherStateMachine.States
{
    class WaitingForWakeup : StateMachine.IState
    {
        public WaitingForWakeup(StateMachine myMachine)
            : base(myMachine)
        {
        }

        public override StateMachine.State MyState()
        {
            return StateMachine.State.WaitingForWakeup;
        }

        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.LookingForToken, StateMachine.State.WaitingForWakeup, StateMachine.State.PreEmptiveLogin };
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }

        public override void DoProcess()
        {
            _debugMessage(this.ToString(), "Start DoProcess");
            myMachine.WaitForEvent(myMachine.myData._CPOnlineEvent);
            if (myMachine.myData._PreEmptive._allowPreEmptiveLogins)
            {
                myMachine.SwitchToState(StateMachine.State.PreEmptiveLogin);
            }
            else
            {
                myMachine.SwitchToState(StateMachine.State.LookingForToken);
            }
            _debugMessage(this.ToString(), "End DoProcess");
        }

    }
}
