#define USE_RSA

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Threading;
using System.Net;
using System.IO;

using System.Security.Cryptography;

namespace CachedOTPCommon
{
    public static class Utils
    {
        public static byte[] URLdecode(string inp)
        {
            inp = inp.Replace('-', '+');
            inp = inp.Replace('_', '/');
            while (inp.Length % 4 != 0)
            {
                inp += "=";
            }
            return Convert.FromBase64String(inp);
        }

        public static string URLencode(byte[] inp)
        {
            string pout = Convert.ToBase64String(inp);
            pout = pout.Replace('+', '-').Replace('/', '_');
            pout = pout.Replace("=", "");
            return pout;
        }

        public static string URLencode(byte[] inp, int start, int length)
        {
            string pout = Convert.ToBase64String(inp, start, length);
            pout = pout.Replace('+', '-').Replace('/', '_');
            pout = pout.Replace("=", "");
            return pout;
        }
    }

    enum GCSStage
    {
        ClientRequest, //Client starts with a request for updated tokens
        CustomerChallenge, //Customer makes a challenge
        ClientResponse, //Client responds to the challenge
        CustomerData, //Customer sends data for the client
        ClientReject, //Client has cancelled the request
        CustomerReject, //Customer has cancelled the request
        Closing //(Not sure)
    }

    class Common
    {

        public static string _MiTokenIP;

        public static string _CustomerID;

        struct scratch
        {
            public byte[] data;
            public int length;
        }

        private static byte[] readStream(Stream stream)
        {
            List<scratch> scratchBuffer = new List<scratch>();
            while (true)
            {
                byte[] b = new byte[1024];
                int readSize = stream.Read(b, 0, 1024);
                if (readSize == 0)
                    break;
                scratch ss = new scratch();
                ss.length = readSize;
                ss.data = b;
                scratchBuffer.Add(ss);
            }
            int totalSize = 0;
            foreach (scratch s in scratchBuffer)
                totalSize += s.length;

            byte[] ret = new byte[totalSize];
            int pos = 0;
            foreach (scratch s in scratchBuffer)
            {
                Array.Copy(s.data, 0, ret, pos, s.length);
                pos += s.length;
            }

            return ret;
        }

        public static string processStage(string reqStr, Dictionary<string, string> args)
        {
            //Use POST as we will be sending lots of data
            using (WebClient client = new WebClient())
            {
                System.Collections.Specialized.NameValueCollection Dic = new System.Collections.Specialized.NameValueCollection();
                Dictionary<string, string>.KeyCollection argKeys = args.Keys;
                foreach (String argkey in argKeys)
                {
                    Dic.Add(argkey, args[argkey]);
                }
                byte[] resp = client.UploadValues(reqStr, Dic);
                return System.Text.Encoding.ASCII.GetString(resp);
            }


            /*
            WebRequest req = WebRequest.Create(reqStr);

            req.Method = "GET";

            WebResponse resp = req.GetResponse();
            Stream stream = resp.GetResponseStream();
            byte[] bdata = readStream(stream);
            return System.Text.Encoding.ASCII.GetString(bdata);
             * */
        }

        public static string updateStage(string reqID, GCSStage currentStage, GCSStage nextStage, string data)
        {
            string reqStr = string.Format("http://{0}/Default.aspx", _MiTokenIP); //?type=gcs&action=update-state&customerId={1}&requestId={2}&currentState={3}&nextState={4}&data={5}", _MiTokenIP, _CustomerID, reqID, currentStage.ToString(), nextStage.ToString(), data);
            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("type", "gcs");
            args.Add("action", "update-state");
            args.Add("customerId", _CustomerID);
            args.Add("requestId", reqID);
            args.Add("currentState", currentStage.ToString());
            args.Add("nextState", nextStage.ToString());
            args.Add("data", data);
            return processStage(reqStr, args);
        }

        public static string encryptAES(byte[] data, byte[] Key, byte[] IV)
        {
            byte[] sizedData = new byte[data.Length + 4];
            int size = data.Length;
            for (int i = 0; i < 4; ++i)
            {
                sizedData[i] = (byte)(size & 0xFF);
                size >>= 8;
            }
            Array.Copy(data, 0, sizedData, 4, data.Length);

            byte[] encryptedData = null;
            using (AesCryptoServiceProvider myAES = new AesCryptoServiceProvider())
            {
                myAES.Key = Key;
                myAES.IV = IV;
                ICryptoTransform encryptor = myAES.CreateEncryptor(myAES.Key, myAES.IV);
                using (MemoryStream msEncrypt = new MemoryStream())
                {
                    using (CryptoStream csEncrypt = new CryptoStream(msEncrypt, encryptor, CryptoStreamMode.Write))
                    {
                        using (BinaryWriter brEncrypt = new BinaryWriter(csEncrypt))
                        {
                            brEncrypt.Write(sizedData, 0, sizedData.Length);
                        }
                        encryptedData = msEncrypt.ToArray();
                    }
                }
            }
            return Utils.URLencode(encryptedData);
        }

        public static string encryptData(byte[] data, RSAParameters RSAKeyInfo)
        {
            byte[] encryptedData;
#if USE_RSA
            using (RSACryptoServiceProvider RSA = new RSACryptoServiceProvider())
            {
                RSA.ImportParameters(RSAKeyInfo);

                encryptedData = RSA.Encrypt(data, true);
            }
#else
            encryptedData = data;
#endif
            return Utils.URLencode(encryptedData);
        }

        public static bool tryDecryptData(string data, RSAParameters RSAKeyInfo, out byte[] output)
        {
            try
            {
                output = decryptData(data, RSAKeyInfo);
                return true;
            }
            catch(CryptographicException e)
            {
                output = null;
                return false;
            }
        }

        public static byte[] decryptData(string data, RSAParameters RSAKeyInfo)
        {
            byte[] bdata = Utils.URLdecode(data);
            byte[] decryptedData;
#if USE_RSA
            using (RSACryptoServiceProvider RSA = new RSACryptoServiceProvider())
            {
                RSA.ImportParameters(RSAKeyInfo);

                decryptedData = RSA.Decrypt(bdata, true);

            }
#else
            decryptedData = bdata;
#endif
            return decryptedData;
        }


        static int readInt(FileStream stream)
        {
            byte[] data = new byte[4];
            stream.Read(data, 0, 4);
            int len = 0;
            for (int i = 0; i < 4; ++i)
            {
                len += (data[i] << (8 * i));
            }

            return len;
        }


        static byte[] readByteArr(FileStream stream)
        {
            int length = readInt(stream);
            byte[] ret = new byte[length];
            stream.Read(ret, 0, length);

            return ret;
        }

        public static byte[] flipDwordOrder(byte[] source)
        {
            byte[] output = new byte[source.Length];
            for (int i = 0; i < source.Length; i += 4)
            {
                int sourceIndex = source.Length - i - 4;
                for (int j = 0; j < 4; ++j)
                    output[i + j] = source[sourceIndex + j];
            }
            return output;
        }
        public static RSAParameters DemoLoadParams(FileStream stream, bool priv)
        {
            RSAParameters p = new RSAParameters();
            p.Exponent = readByteArr(stream);
            p.Modulus = readByteArr(stream);
            if (priv)
            {
                p.D = readByteArr(stream);
                p.DP = readByteArr(stream);
                p.DQ = readByteArr(stream);
                p.InverseQ = readByteArr(stream);
                p.P = readByteArr(stream);
                p.Q = readByteArr(stream);

                
            }

            return p;
        }
    }

}
