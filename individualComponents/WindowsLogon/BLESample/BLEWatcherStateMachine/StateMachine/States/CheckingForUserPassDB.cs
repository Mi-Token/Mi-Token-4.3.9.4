using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLEWatcherStateMachine.States
{
    class CheckingForUserPassDB : StateMachine.IState
    {
        public CheckingForUserPassDB(StateMachine myMachine)
            : base(myMachine)
        { }
        public override StateMachine.State MyState()
        {
            return StateMachine.State.CheckingForUserPassDB;
        }

        static StateMachine.State[] validStates = new StateMachine.State[] { StateMachine.State.WaitingForWakeup, StateMachine.State.WaitingForCPResponse };
        public override StateMachine.State[] validTransferStates()
        {
            return validStates;
        }

        public override void DoProcess()
        {
            _debugMessage(this.ToString(), "Start DoProcess");
            string username = myMachine.myData.verifiedUsername;
            string password;
            bool hasPass = myMachine.passwordHandler.getPasswordForUsername(username, out password);
            if (hasPass)
            {
                myMachine.SwitchToState(StateMachine.State.WaitingForCPResponse);
                sendPasswordKnownBlock(password);                
            }
            else
            {
                myMachine.SwitchToState(StateMachine.State.WaitingForCPResponse);
                //send the Password not known block
                sendPasswordUnknownBlock();                
            }
        }

        protected void sendPasswordUnknownBlock()
        {
            ByteBuilder bb = new ByteBuilder();
            byte[] userBytes = Encoding.Unicode.GetBytes(myMachine.myData.verifiedUsername);
            byte[] nullBytes = new byte[] { 0, 0 };
            bb.Add((byte)StateMachine.DATA_FLAG.SET_USERNAME);
            bb.Add((byte)(userBytes.Length + 2));
            bb.Add(userBytes);
            bb.Add(nullBytes);
            myMachine.myData.ble.Pipe_SendMessage(bb.ToArray, -1);

            //throw new NotImplementedException();
        }
        protected void sendPasswordKnownBlock(string password)
        {
            ByteBuilder bb = new ByteBuilder();
            byte[] userBytes = Encoding.Unicode.GetBytes(myMachine.myData.verifiedUsername);
            byte[] passBytes = Encoding.Unicode.GetBytes(password);
            byte[] nullBytes = new byte[] { 0, 0 };
            bb.Add((byte)StateMachine.DATA_FLAG.SET_USERANDPASS);
            bb.Add((byte)(userBytes.Length + 2));
            bb.Add(userBytes);
            bb.Add(nullBytes);
            bb.Add((byte)(passBytes.Length + 2));
            bb.Add(passBytes);
            bb.Add(nullBytes);
            myMachine.myData.ble.Pipe_SendMessage(bb.ToArray, -1);

            //throw new NotImplementedException();
        }
    }
}
