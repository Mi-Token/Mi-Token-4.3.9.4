using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.IO;
using System.Threading;
using System.Security.Cryptography;

namespace MiTokenOfflineTokenSync
{
    class CacheOTPClient
    {
        private readonly string _customerID, _MiTokenIP;
        private readonly int _pollInterval;
        private readonly string _userName;
        private readonly bool _loadRSAFromFile = false;
        private readonly Action<string, bool> _log;

        private void Log(string info)
        {
            _log(info, false);
        }


        public CacheOTPClient(Action<string,bool> log, string customerID, int pollInterval, string MiTokenIP, string userName, bool loadRSAFromFile = false)
        {
            _customerID = customerID;
            _pollInterval = pollInterval;
            _MiTokenIP = MiTokenIP;
            _userName = userName;
            Common._MiTokenIP = MiTokenIP;
            Common._CustomerID = customerID;
            _loadRSAFromFile = loadRSAFromFile;
            _log = log;
        }

        public byte[] Start(object obj)
        {
            if (!(obj is CancellationToken))
                throw new ArgumentException("no cancellation token");

            return fetchCache((CancellationToken)obj);
        }

        private string _reqID;
        private byte[] _nonce;
        private byte[] _IV;
        private byte[] _KEY;
        private const int KEY_LENGTH = 32; //AES256 key
        private const int IV_LENGTH = 16; //AES256 IV
        private const int NONCE_LENGTH = KEY_LENGTH + IV_LENGTH; //the nonce will be the AES256 Key appended with AES256 IV

        internal RSAParameters decryptKey;
        internal RSAParameters encryptKey;

        private void generateNonce()
        {
            Log("Generating Nonce");
            _nonce = new byte[4 + NONCE_LENGTH];
            _nonce[0] = (byte)'T';
            _nonce[1] = (byte)'E';
            _nonce[2] = (byte)'S';
            _nonce[3] = (byte)'U';

            RNGCryptoServiceProvider rngCsp = new RNGCryptoServiceProvider();
            byte[] nonce = new byte[NONCE_LENGTH];
            rngCsp.GetBytes(nonce);
            _KEY = new byte[KEY_LENGTH];
            _IV = new byte[IV_LENGTH];
            Array.Copy(nonce, 0, _nonce, 4, NONCE_LENGTH);
            Array.Copy(nonce, 0, _KEY, 0, KEY_LENGTH);
            Array.Copy(nonce, KEY_LENGTH, _IV, 0, IV_LENGTH);
        }

        private bool validateNonce(byte[] input)
        {
            Log("Validating Nonce");
            if (input.Length == _nonce.Length)
            {
                for (int i = 4 + KEY_LENGTH; i < input.Length; ++i) //changing only the IV to be part of the NONCE, The KEY Value send back from the server will be XOR'd will our KEY value to create the real key value.
                    if (input[i] != _nonce[i])
                        return false;
                return true;
            }
            return false;
        }

        private void loadRSAParams()
        {
            if (_loadRSAFromFile)
            {
                Log("Loading RSA Parameters");
                FileStream fstream = new FileStream("Client_Pub.key", FileMode.Open);
                encryptKey = Common.DemoLoadParams(fstream, false);
                fstream.Close();
            
            
                fstream = new FileStream("Client_Priv.key", FileMode.Open);
                decryptKey = Common.DemoLoadParams(fstream, true);
                fstream.Close();
            }
        }

        private bool stageClientRequest(CancellationToken token)
        {
            loadRSAParams();
            Log("ClientRequest");
            generateNonce();
            string challenge = Common.encryptData(_nonce, encryptKey);
            string reqStr = string.Format("http://{0}/Default.aspx?type=gcs&action=client-new&userName={1}&customerId={2}&challenge={3}", _MiTokenIP, _userName, _customerID, challenge);
            //Log(Environment.NewLine + reqStr + Environment.NewLine);

            string resp = Common.processStage(reqStr);
            if (resp[0] == 's')
            {
                Log("ClientRequest: Request Sent");
                _reqID = resp.Substring(1);
                Log("ID : " + _reqID);
                return true;
            }
            else
            {
                Log("ClientRequest: Request Failed");
                //something went wrong
                return false;
            }
        }

        private bool stageCustomerChallenge(CancellationToken token)
        {
            string resp = null;

            while (!token.IsCancellationRequested)
            {
                Log("CustomerChallenge");
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
                    Log("CustomerChallenge Error:");
                    Log(resp);
                }
                Log("CustomerChallenge: Nothing Found");
                Thread.Sleep(_pollInterval);
            }

            if (!string.IsNullOrWhiteSpace(resp))
                Log("CustomerChallenge: response received");

            if (resp == "sREJECT" || token.IsCancellationRequested)
            {
                stageReject(token.IsCancellationRequested);
                return false;
            }

            //we will be getting the reply to our challenge
            resp = resp.Substring(1);
            byte[] challenge = Common.decryptData(resp, decryptKey);
            if (validateNonce(challenge))
            {
                if ((challenge[0] == 'S') && (challenge[1] == 'E') && (challenge[2] == 'N') && (challenge[3] == 'D'))
                {
                    Log("CustomerChallenge: Valid Nonce");
                    //Create the real key
                    for (int i = 0; i < KEY_LENGTH; ++i)
                    {
                        challenge[i + 4] ^= _nonce[i + 4]; //XOR the Server's KEY with the clients KEY, we send this back to validate to the server that we read there data
                        _nonce[i + 4] = challenge[i + 4]; //We need to update _nonce, because that is where KEY will be read from when decrypting
                        _KEY[i] = _nonce[i + 4]; //KEY is read from _KEY
                    }
                    challenge[0] = (byte)'D';
                    challenge[1] = (byte)'O';
                    challenge[2] = (byte)'N';
                    challenge[3] = (byte)'E';
                    Log("CustomerChallenge: Sending ClientResponse");
                    string data = Common.encryptData(challenge, encryptKey);
                    string resp2 = Common.updateStage(_reqID, GCSStage.CustomerChallenge, GCSStage.ClientResponse, data);

                    if (resp2[0] == 's')
                    {
                        Log("CustomerChallenge: Request Sent");
                        return true;
                    }

                    Log("CustomerChallenge: Request Failed");
                    //something went wrong
                    return false;
                }
            }

            Log("CustomerChallenge: Invalid Nonce");
            //invalid nonce
            return false;
        }

        private bool stageCustomerData(CancellationToken token, ref byte[] result)
        {
            string resp = null;

            while (!token.IsCancellationRequested)
            {
                Log("CusomterData");
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
                    Log("CusomterData Error:");
                    Log(resp);
                    //something went wrong
                }
                Log("CusomterData: Nothing found");
                Thread.Sleep(_pollInterval);
            }

            if (!string.IsNullOrWhiteSpace(resp))
                Log("CustomerData: response received");

            if (resp == "sREJECT" || token.IsCancellationRequested)
            {
                stageReject(token.IsCancellationRequested);
                return false;
            }

            resp = resp.Substring(1);

            /*
            StringBuilder sb = new StringBuilder();
            sb.Append("AES KEY\r\n");
            for (int i = 0; i < KEY_LENGTH; ++i)
            {
                sb.Append(string.Format("{0:X2} ", _KEY[i]));
            }
            sb.Append("\r\nIV KEY\r\n");
            for (int i = 0; i < IV_LENGTH; ++i)
            {
                sb.Append(string.Format("{0:X2} ", _IV[i]));
            }
            sb.Append("\r\n");
            Log(sb.ToString());
            */

            byte[] tokenInfo = Common.decryptAES(resp, _KEY, _IV);
            result = tokenInfo;

            Log("CusomterData: Closing Request");
            Common.updateStage(_reqID, GCSStage.CustomerData, GCSStage.Closing, "");
            return true;
        }

        void stageReject(bool bCancelled)
        {
            Log(bCancelled? "StageReject: cancelled" : "StageReject: received Reject");
            Common.updateStage(_reqID, GCSStage.CustomerReject, GCSStage.Closing, "");
        }

        byte[] fetchCache(CancellationToken token)
        {
            if (!System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
            {
                Log("FetchCache - network not available");
                return null;
            }

            if (!token.IsCancellationRequested && stageClientRequest(token))
                if (!token.IsCancellationRequested && stageCustomerChallenge(token))
                    if (!token.IsCancellationRequested)
                    {
                        byte[] result = null;

                        if (stageCustomerData(token, ref result))
                          return result;
                    }

            return null;
        }
    }
}
