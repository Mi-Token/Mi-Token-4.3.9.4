using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Net;
using System.IO;
using CachedOTPCommon;

using System.Security.Cryptography;
using Microsoft.Win32;

namespace CachedOTPServer
{
    class CacheOTPServerThread
    {

        

        public delegate void DebugInfoHandle(object sender, string info);
        public event DebugInfoHandle DebugInfo;
        
        private void m_DebugInfo(string info)
        {
            if (DebugInfo != null)
            {
                DebugInfo(this, info);
            }
        }

        
        string _customerID;
        int _pollInterval;

        Thread _thread;
        bool _running;
        bool _threadRunning;
        string _MiTokenIP;

        /// <summary>
        /// Creates a new Thread to do polling of the MiToken Server at pollInterval time
        /// </summary>
        /// <param name="customerID">The CustomerID to use for requests</param>
        /// <param name="pollInterval">How often to poll the MiToken Server (in seconds)</param>
        /// <param name="MiTokenIP">The MiToken Server IP</param>
        public CacheOTPServerThread(string customerID, int pollInterval, string MiTokenIP)
        {
            _customerID = customerID;
            _pollInterval = pollInterval;
            _MiTokenIP = MiTokenIP;
            _thread = new Thread(new ThreadStart(threadFunction));
            _running = true;
            Common._MiTokenIP = MiTokenIP;
            Common._CustomerID = customerID;
            _threadRunning = false;
        }

        public void Start()
        {
            _threadRunning = true;
            _thread.Start();
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

        

        Dictionary<string, byte[]> nonceLUT = new Dictionary<string, byte[]>();
        Dictionary<string, RSAParameters> encryptKeys = new Dictionary<string,RSAParameters>();
        Dictionary<string, RSAParameters> decryptKeys = new Dictionary<string,RSAParameters>();

        void addRSAKeys(string userID, string reqID)
        {
            m_DebugInfo("Loading RSA Keys for user " + userID);
            FileStream fstream = new FileStream("C:\\Debug\\Keys\\" + userID + ".key", FileMode.Open);
            encryptKeys[reqID] = Common.DemoLoadParams(fstream, false);
            fstream.Close();

            m_DebugInfo("Loading Private RSA Keys from Registry");

            RegistryKey rkey = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\Logon");
            //check that the required values exist
            RSAParameters privKey;
            privKey.Modulus = (byte[])(rkey.GetValue("PublicModulus", null));
            privKey.Exponent = (byte[])(rkey.GetValue("PublicExponent", null));
            privKey.D = (byte[])(rkey.GetValue("PrivateD", null));
            privKey.DP = (byte[])(rkey.GetValue("PrivateDP", null));
            privKey.DQ = (byte[])(rkey.GetValue("PrivateDQ", null));
            privKey.InverseQ = (byte[])(rkey.GetValue("PrivateInverseQ", null));
            privKey.P = (byte[])(rkey.GetValue("PrivateP", null));
            privKey.Q = (byte[])(rkey.GetValue("PrivateQ", null));

            if ((privKey.Modulus == null) || (privKey.Exponent == null) || (privKey.D == null) || (privKey.DP == null)
                || (privKey.DQ == null) || (privKey.InverseQ == null) || (privKey.P == null) || (privKey.Q == null))
            {
                //error
                m_DebugInfo("Error : One or more keys not set");
                throw new Exception("Error : Missing Required RSA Key");
            }
            decryptKeys[reqID] = privKey;

            /*
            m_DebugInfo("Adding RSA Keys for " + reqID);
            m_DebugInfo("Loading RSA Parameters");
            FileStream fstream = new FileStream("Server_Pub.key", FileMode.Open);
            encryptKeys[reqID] = Common.DemoLoadParams(fstream, false);
            fstream.Close();
            fstream = new FileStream("Server_Priv.key", FileMode.Open);
            decryptKeys[reqID] = Common.DemoLoadParams(fstream, true);
            fstream.Close();
             * */
        }

        void generateCustomerResponse(string user, string data, string reqID)
        {
            addRSAKeys(user, reqID);
            m_DebugInfo("Generating Response for " + user);
            byte[] bdata;
            if (!Common.tryDecryptData(data, decryptKeys[reqID], out bdata))
            {
                m_DebugInfo("Failed to RSA Decrypt - Rejecting");
                Common.updateStage(reqID, GCSStage.ClientRequest, GCSStage.CustomerReject, "");
                return;
            }

            if ((bdata[0] == 'T') && (bdata[1] == 'E') && (bdata[2] == 'S') && (bdata[3] == 'T'))
            {
                m_DebugInfo("Valid Nonce");
                bdata[0] = (byte)'S';
                bdata[1] = (byte)'E';
                bdata[2] = (byte)'N';
                bdata[3] = (byte)'D';

                nonceLUT[reqID] = bdata;

                string sreply = Common.encryptData(bdata, encryptKeys[reqID]);
                string resp = Common.updateStage( reqID, GCSStage.ClientRequest, GCSStage.CustomerChallenge, sreply);
                if (resp != "sUPDATED")
                {
                    //something went wrong
                    m_DebugInfo("Error:");
                    m_DebugInfo(resp);                    
                }
            }
            else
            {
                m_DebugInfo("Invalid Nonce");
                //invalid data
                string resp = Common.updateStage(reqID, GCSStage.ClientRequest, GCSStage.CustomerReject, "");
                if (resp != "sUPDATED")
                {
                    m_DebugInfo("Error");
                    m_DebugInfo(resp);
                    //something went wrong
                }
            }

        }

        bool validateClientResponse(string reqID, string data)
        {
            m_DebugInfo("Validating Client Response");
            if(nonceLUT.ContainsKey(reqID))
            {
                byte[] nonce = nonceLUT[reqID];

                byte[] bdata = Common.decryptData(data, decryptKeys[reqID]);

                if((bdata[0] == 'D') && (bdata[1] == 'O') && (bdata[2] == 'N') && (bdata[3] == 'E'))
                {
                    if(bdata.Length == nonce.Length)
                    {
                        for(int i = 4 ; i < nonce.Length ; ++i)
                        {
                            if(nonce[i] != bdata[i])
                                return false;
                        }
                        return true;
                    }
                }
            }
            return  false;
        }

        const int KEY_LENGTH = 32;
        const int IV_LENGTH = 16;
        const int NONCE_LENGTH = KEY_LENGTH + IV_LENGTH;

        void generateCustomerData(string reqID, string userName)
        {
            m_DebugInfo("Generating Client Data");
            byte[] nonce = nonceLUT[reqID];
            nonceLUT.Remove(reqID);
            
            byte[] AESKey = new byte[KEY_LENGTH];
            byte[] AESIV = new byte[IV_LENGTH];

            //byte[] data = new byte[] { (byte)'X', (byte)'Y', (byte)'Z', (byte)'W' };

            FileStream fstream = new FileStream("DemoData.dat", FileMode.Open);
            byte[] data = new byte[(int)fstream.Length];
            fstream.Read(data, 0, data.Length);
            fstream.Close();

            string sdata = Common.encryptAES(data, AESKey, AESIV);

            string resp = Common.updateStage(reqID, GCSStage.ClientResponse, GCSStage.CustomerData, sdata);
            if(resp[0] == 's')
            {
                m_DebugInfo("Done");
                //well we are done
                return;
            }
            else
            {
                m_DebugInfo("Error:");
                m_DebugInfo(resp);
                //something went wrong :(
            }
        }

        void processStageClientRequest()
        {
            m_DebugInfo("Checking for ClientRequest");
            string reqStr = string.Format("http://{0}/Default.aspx", _MiTokenIP); //?type=gcs&action=customer-poll&customerId={1}&stage={2}", _MiTokenIP, _customerID, GCSStage.ClientRequest.ToString());
            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("type", "gcs");
            args.Add("action", "customer-poll");
            args.Add("customerId", _customerID);
            args.Add("stage", GCSStage.ClientRequest.ToString());
            string resp = Common.processStage(reqStr, args);

            if (resp[0] == 's')
            {
                resp = resp.Substring(1);
                if (resp == "NOTHING")
                {
                    m_DebugInfo("Nothing Found");
                    return;
                }
                string[] strs = resp.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries);
                foreach (string s in strs)
                {
                    string[] parts = s.Split('|');
                    if (parts.Length == 3)
                    {
                        string reqID = parts[0];
                        string user = parts[1];
                        string data = parts[2];
                        generateCustomerResponse(user, data, reqID);
                    }
                    else
                    {
                        m_DebugInfo("Error with parts = " + parts.Length.ToString());
                        //something is wrong :(
                    }
                }
            }
        }

        void processStageClientResponse()
        {
            m_DebugInfo("Checking for Client Response");
            string reqStr = string.Format("http://{0}/Default.aspx", _MiTokenIP); //?type=gcs&action=customer-poll&customerId={1}&stage={2}", _MiTokenIP, _customerID, GCSStage.ClientResponse.ToString());
            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("type", "gcs");
            args.Add("action", "customer-poll");
            args.Add("customerId", _customerID);
            args.Add("stage", GCSStage.ClientResponse.ToString());

            string resp = Common.processStage(reqStr, args);

            if (resp[0] == 's')
            {
                resp = resp.Substring(1);
                if (resp == "NOTHING")
                {
                    m_DebugInfo("Nothing Found");
                    return;
                }
                string[] strs = resp.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries);
                foreach (string s in strs)
                {
                    string[] parts = s.Split('|');
                    if (parts.Length == 3)
                    {
                        string reqID = parts[0];
                        string user = parts[1];
                        string data = parts[2];
                        if (validateClientResponse(reqID, data))
                        {
                            m_DebugInfo("Valid Response");
                            generateCustomerData(reqID, user);
                        }
                        else
                        {
                            //invalid response, reject it
                            m_DebugInfo("Invalid Response - Rejecting");
                            Common.updateStage(reqID, GCSStage.ClientResponse, GCSStage.CustomerReject, "");
                        }

                    }
                    else
                    {
                        m_DebugInfo("Error with parts length = " + parts.Length.ToString());
                        //something is wrong :(
                    }
                }
            }
        }

        void processStageClientReject()
        {

            m_DebugInfo("Checking for ClientReject");
            string reqStr = string.Format("http://{0}/Default.aspx", _MiTokenIP); //?type=gcs&action=customer-poll&customerId={1}&stage={2}", _MiTokenIP, _customerID, GCSStage.ClientResponse.ToString());
            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("type", "gcs");
            args.Add("action", "customer-poll");
            args.Add("customerId", _customerID);
            args.Add("stage", GCSStage.ClientReject.ToString());

            string resp = Common.processStage(reqStr, args);

            if (resp[0] == 's')
            {
                resp = resp.Substring(1);
                if (resp == "NOTHING")
                {
                    m_DebugInfo("Nothing Found");
                    return;
                }
                string[] strs = resp.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries);
                foreach (string s in strs)
                {
                    string[] parts = s.Split('|');
                    if (parts.Length == 1)
                    {
                        string reqID = parts[0];
                        m_DebugInfo("Closing " + reqID);
                        Common.updateStage(reqID, GCSStage.ClientReject, GCSStage.Closing, "");
                    }
                    else
                    {
                        m_DebugInfo("Error with parts length = " + parts.Length.ToString());
                        //something is wrong :(
                    }
                }
            }
            
        }

        void threadFunction()
        {
            try
            {
                while (_running)
                {
                    //first poll for items we need to process
                    //we will process stages in the same order as the same order as the state machine follows
                    processStageClientRequest();
                    processStageClientResponse();
                    processStageClientReject();
                    Thread.Sleep(_pollInterval);
                }

            }
            finally
            {
                _threadRunning = false;
            }
        }

    }
}
