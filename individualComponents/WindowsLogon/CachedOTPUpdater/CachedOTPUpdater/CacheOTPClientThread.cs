using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Threading;
using System.Net;
using System.IO;

using CachedOTPCommon;
using System.Security.Cryptography;


namespace CachedOTPUpdater
{
    class CacheOTPClientThread
    {
        GCSStage _curStage;
        Thread _thread;
        bool _running;
        bool _threadRunning;

        string _customerID, _MiTokenIP;
        int _pollInterval;
        string _userName;

        public delegate void ProcessingCompletedHandle(object sender, string result);
        public event ProcessingCompletedHandle ProcessingCompleted;

        public delegate void DebugInfoHandle(object sender, string info);
        public event DebugInfoHandle DebugInfo;

        private void m_ProcessingCompleted(string result)
        {
            if (_threadRunning)
            {
                if (ProcessingCompleted != null)
                {
                    ProcessingCompleted(this, result);
                }
            }
        }

        private void m_DebugInfo(string info)
        {
            if (_threadRunning)
            {
                if (DebugInfo != null)
                {
                    DebugInfo(this, info);
                }
            }
        }

        bool _loadRSAFromFile = false;

        public CacheOTPClientThread(string customerID, int pollInterval, string MiTokenIP, string userName, bool loadRSAFromFile = false)
        {
            _customerID = customerID;
            _pollInterval = pollInterval;
            _MiTokenIP = MiTokenIP;
            _userName = userName;
            Common._MiTokenIP = MiTokenIP;
            Common._CustomerID = customerID;
            _loadRSAFromFile = false;

            _thread = new Thread(new ThreadStart(threadFunction));
            _running = true;
        }

        //Will start an ASYNC request - this one uses callbacks to update on the status (ProgessCompleted has the final result)
        public void Start()
        {
            _threadRunning = true;
            _thread.Start();
        }

        //Will start a SYNC request - this one will not use any callbacks, and returns the final result
        public string StartSync()
        {
            return innerThreadFunction();
        }

        /// <summary>
        /// Informs the thread to stop when it is next safe to do so
        /// </summary>
        public void Stop()
        {
            _running = false;
        }

        /// <summary>
        /// Check if the thread has signaled that it is safe to shutdown the application
        /// </summary>
        /// <returns>True if the thread has finished</returns>
        public bool SafeToShutdown()
        {
            return !_threadRunning;
        }

        /// <summary>
        /// Forces the thread to shutdown.
        /// Note: Skips certain parts of the shutdown sequence to allow for a much faster shutdown
        /// </summary>
        public void Abort()
        {
            _thread.Abort();
        }

        string _reqID;
        byte[] _nonce;
        const int NONCE_LENGTH = 28;

        Random rnd = new Random();
        public RSAParameters decryptKey;
        public RSAParameters encryptKey;

        void generateNonce()
        {
            m_DebugInfo("Generating Nonce");
            _nonce = new byte[4 + NONCE_LENGTH];
            _nonce[0] = (byte)'T';
            _nonce[1] = (byte)'E';
            _nonce[2] = (byte)'S';
            _nonce[3] = (byte)'T';

            for (int i = 4; i < _nonce.Length; ++i)
            {
                _nonce[i] = (byte)rnd.Next(256);
            }
        }

        bool validateNonce(byte[] input)
        {
            m_DebugInfo("Validating Nonce");
            if (input.Length == _nonce.Length)
            {
                for (int i = 4; i < input.Length; ++i)
                    if (input[i] != _nonce[i])
                        return false;
                return true;
            }
            return false;
        }

        void loadRSAParams()
        {
            if (_loadRSAFromFile)
            {
                m_DebugInfo("Loading RSA Parameters");
                FileStream fstream = new FileStream("Client_Pub.key", FileMode.Open);
                encryptKey = Common.DemoLoadParams(fstream, false);
                fstream.Close();
                fstream = new FileStream("Client_Priv.key", FileMode.Open);
                decryptKey = Common.DemoLoadParams(fstream, true);
                fstream.Close();
            }
        }

        bool stageClientRequest()
        {
            loadRSAParams();
            m_DebugInfo("ClientRequest");
            generateNonce();
            string challenge = Common.encryptData(_nonce, encryptKey);
            string reqStr = string.Format("http://{0}/Default.aspx?type=gcs&action=client-new&userName={1}&customerId={2}&challenge={3}", _MiTokenIP, _userName, _customerID, challenge);

            string resp = Common.processStage(reqStr);
            if (resp[0] == 's')
            {
                m_DebugInfo("Request Sent");
                _reqID = resp.Substring(1);
                m_DebugInfo("ID : " + _reqID);
                return true;
            }
            else
            {
                m_DebugInfo("Request Failed");
                //something went wrong
                return false;
            }
        }

        bool stageCustomerChallenge()
        {
            string resp;
            while (true)
            {
                m_DebugInfo("Checking for Customer Challenge");
                string reqStr = string.Format("http://{0}/Default.aspx?type=gcs&action=client-poll&userName={1}&requestId={2}", _MiTokenIP, _userName, _reqID);
                resp = Common.processStage(reqStr);
                if (resp[0] == 's')
                {
                    //resp will just be the letter 's' iff the request isn't at a repliable stage
                    if (resp.Length > 1)
                        break;
                }
                else
                {
                    //something went wrong
                    m_DebugInfo("Error:");
                    m_DebugInfo(resp);
                }
                m_DebugInfo("Nothing Found");
                Thread.Sleep(_pollInterval);
            }

            m_DebugInfo("Something Found");
            if (resp == "sREJECT")
            {
                stageReject();
                return false;
            }

            //we will be getting the reply to our challenge
            resp = resp.Substring(1);
            byte[] challenge = Common.decryptData(resp, decryptKey);
            if (validateNonce(challenge))
            {
                if ((challenge[0] == 'S') && (challenge[1] == 'E') && (challenge[2] == 'N') && (challenge[3] == 'D'))
                {
                    m_DebugInfo("Valid Nonce");
                    challenge[0] = (byte)'D';
                    challenge[1] = (byte)'O';
                    challenge[2] = (byte)'N';
                    challenge[3] = (byte)'E';
                    m_DebugInfo("Sending ClientResponse");
                    string data = Common.encryptData(challenge, encryptKey);
                    string resp2 = Common.updateStage(_reqID, GCSStage.CustomerChallenge, GCSStage.ClientResponse, data);

                    if (resp2[0] == 's')
                    {
                        m_DebugInfo("Request Sent");
                        return true;
                    }

                    m_DebugInfo("Request Failed");
                    //something went wrong
                    return false;
                }
            }

            m_DebugInfo("Invalid Nonce");
            //invalid nonce
            return false;
        }


        string res;

        bool stageCustomerData(ref string result)
        {
            string resp;
            while (true)
            {
                m_DebugInfo("Checking for CusomterData");
                string reqStr = string.Format("http://{0}/Default.aspx?type=gcs&action=client-poll&userName={1}&requestId={2}", _MiTokenIP, _userName, _reqID);
                resp = Common.processStage(reqStr);
                if (resp[0] == 's')
                {
                    //resp will just be the letter 's' iff the request isn't at a repliable stage
                    if (resp.Length > 1)
                        break;
                }
                else
                {
                    m_DebugInfo("Error:");
                    m_DebugInfo(resp);
                    //something went wrong
                }
                m_DebugInfo("Nothing found");
                Thread.Sleep(_pollInterval);
            }

            m_DebugInfo("Something Found");
            if (resp == "sREJECT")
            {
                stageReject();
                return false;
            }
            
            resp = resp.Substring(1);
            byte[] tokenInfo = Common.decryptData(resp, decryptKey);
            result = Encoding.ASCII.GetString(tokenInfo);

            m_DebugInfo("Closing Request");
            Common.updateStage(_reqID, GCSStage.CustomerData, GCSStage.Closing, "");
            return true;
        }

        void stageReject()
        {
            m_DebugInfo("Received Reject");
            Common.updateStage(_reqID, GCSStage.CustomerReject, GCSStage.Closing, "");
        }

        string innerThreadFunction()
        {
            bool worked = false;
            string result = "";
            
            if(stageClientRequest())
                if(stageCustomerChallenge())
                    if (stageCustomerData(ref result))
                        worked = true;

            if (!worked)
                result = "FAILED";


            return result;
        }

        
        void threadFunction()
        {
            try
            {
                m_DebugInfo("Thread Started");
                string result = innerThreadFunction();
                m_ProcessingCompleted(result);
            }
            finally
            {
                _threadRunning = false;
            }
        }

    }
}
