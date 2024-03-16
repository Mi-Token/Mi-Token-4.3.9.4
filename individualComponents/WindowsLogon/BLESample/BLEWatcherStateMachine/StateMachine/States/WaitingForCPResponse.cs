using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLEWatcherStateMachine.States
{
    class WaitingForCPResponse : StateMachine.IState
    {
        public WaitingForCPResponse(StateMachine myMachine)
            : base(myMachine)
        { }

        public override StateMachine.State MyState()
        {
            return StateMachine.State.WaitingForCPResponse;
        }
        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.WaitingForWakeup, StateMachine.State.WatchingToken };
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }


        struct CPData
        {
            public string username;
            public string password;
            public bool returnToWakeup;

            public void ProcessData(byte[] givenData)
            {
                byte userlen, passlen;
                userlen = givenData[1];
                passlen = givenData[2 + userlen];
                username = Encoding.Unicode.GetString(givenData, 2, userlen);
                password = Encoding.Unicode.GetString(givenData, 3 + userlen, passlen);

                //Trim extra binary zeros off the end
                username = username.TrimEnd('\0');
                password = password.TrimEnd('\0'); 
            }

        };

        public override void DoProcess()
        {
            myMachine.myData.ble.Pipe_SendMessage(new byte[] { (byte)StateMachine.DATA_FLAG.SET_WATCHER_STAGE, (byte)StateMachine.WATCHER_STAGES.STAGE_WaitingForUsername }, -1);
            _debugMessage(this.ToString(), "Start DoProcess");
            if (myMachine.WaitForEvent(myMachine.myData._HasCPReturnData) == StateMachine.WaitEventsResult.Reset)
            {
                if (myMachine.myData.verifiedUserMustChangePassword)
                {
                    _debugMessage("Password for username " + myMachine.myData.verifiedUsername + " must be changed. Deleting password from Database");
                    myMachine.passwordHandler.resetPasswordForUsername(myMachine.myData.verifiedUsername);
                    myMachine.myData.verifiedUserMustChangePassword = false;
                }
                return;
            }
            CPData mData = new CPData();
            mData.ProcessData(myMachine.myData.CPReturnData);
            myMachine.myData.CPReturnData = null;
            bool validUser = false;
            if (mData.returnToWakeup)
            {
                myMachine.SwitchToState(StateMachine.State.WaitingForWakeup);
                return;
            }
            if (mData.password != null && (mData.username == myMachine.myData.verifiedUsername))
            {
                myMachine.passwordHandler.setPasswordForUsername(mData.username, mData.password);
                mData.username = "";
                mData.password = "";
                validUser = true;
            }

            if (!validUser)
            {
                _debugMessage("Recieved a password, but the username was wrong");
                myMachine.SwitchToState(StateMachine.State.WaitingForWakeup); //change to a waiting for wakeup state, the user logged in with a username that doesn't belong to this token
            }
            else
            {
                myMachine.SwitchToState(StateMachine.State.WatchingToken);
            }
        }

    }
}
