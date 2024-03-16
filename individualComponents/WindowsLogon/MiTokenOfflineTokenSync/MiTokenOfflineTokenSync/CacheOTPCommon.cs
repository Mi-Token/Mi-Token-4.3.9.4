#define USE_RSA

using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Security.Cryptography;

namespace MiTokenOfflineTokenSync
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

        /// <summary>
        /// Execute a Func and wait for it to complete while imposing a timeout on wait.
        /// </summary>
        /// <param name="timeout">Timeout to wait for the action to complete.</param>
        /// <param name="fun">Func to execute. It should cast its state object to CancellationToken and periodically check: if (token.IsCancellationRequested) {...}</param>
        /// <param name="lst">Error list</param>
        /// <returns>null if timeout, byte[] if the Func completed or null if faulted</returns>
        public static byte[] WaitWithTimeout(int timeout, Func<object, byte[]> fun, ConcurrentDictionary<Exception,int> dic)
        {
            if (dic == null || fun == null)
                throw new ArgumentNullException("WaitWithTimeout");

            var ts = new CancellationTokenSource();
            var task = Task.Factory.StartNew(fun, ts.Token, ts.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);

            task.ContinueWith(t =>
                              {
                                 var ex = t.Exception.Flatten();

                                 foreach (var e in ex.InnerExceptions)
                                    dic.TryAdd(e, 1);
                              },
                              CancellationToken.None,
                              TaskContinuationOptions.OnlyOnFaulted,
                              TaskScheduler.Default);

            bool ret = task.Wait(timeout);

            if (ret == false)
            {
                // timeout
                ts.Cancel(false);
                return null;
            }
            else
            {
                return task.Result;
            }
        }

        /// <summary>
        /// Execute a fire-and-forget action with a startup delay and without waiting for the delay or for the action to complete.
        /// </summary>
        /// <param name="delay">Startup delay</param>
        /// <param name="action">Action to execute - there is no return value</param>
        /// <param name="lst">error list</param>
        public static void ExecuteWithDelay(int delay, Action action, Action<string,bool> log)
        {
            if (log == null || action == null)
                throw new ArgumentNullException("ExecuteWithDelay");

            var tcs = new TaskCompletionSource<object>();
            var timer = new System.Timers.Timer(Convert.ToDouble(delay));
            timer.AutoReset = false;
            timer.Elapsed += (x, y) => tcs.SetResult(null);
            var t = tcs.Task.ContinueWith(_ => { timer.Dispose(); action(); },
                                          CancellationToken.None,
                                          TaskContinuationOptions.PreferFairness,
                                          TaskScheduler.Default);

            t.ContinueWith(previousTask =>
                           {
                              var ex = previousTask.Exception.Flatten();

                              foreach (var e in ex.InnerExceptions)
                              {
                                  var exc = e.InnerException == null? e: e.InnerException;
                                  log(string.Format("Exception {0} while executing action: {1}", exc.GetType().ToString(), exc.Message), false);
                              }
                           },
                           CancellationToken.None,
                           TaskContinuationOptions.OnlyOnFaulted,
                           TaskScheduler.Default);

            timer.Enabled = true;
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

        public static string processStage(string reqStr)
        {
            WebRequest req = WebRequest.Create(reqStr);

            req.Method = "GET";

            WebResponse resp = req.GetResponse();
            Stream stream = resp.GetResponseStream();
            byte[] bdata = readStream(stream);
            return System.Text.Encoding.ASCII.GetString(bdata);
        }

        public static string updateStage(string reqID, GCSStage currentStage, GCSStage nextStage, string data)
        {
            string reqStr = string.Format("http://{0}/Default.aspx?type=gcs&action=update-state&customerId={1}&requestId={2}&currentState={3}&nextState={4}&data={5}", _MiTokenIP, _CustomerID, reqID, currentStage.ToString(), nextStage.ToString(), data);
            return processStage(reqStr);
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
            catch(CryptographicException)
            {
                output = null;
                return false;
            }
        }

        public static byte[] decryptAES(string data, byte[] Key, byte[] IV)
        {
            byte[] bdata = Utils.URLdecode(data);
            byte[] decryptedData = new byte[bdata.Length];
            using (AesCryptoServiceProvider myAES = new AesCryptoServiceProvider())
            {
                myAES.Key = Key;
                myAES.IV = IV;

                ICryptoTransform decryptor = myAES.CreateDecryptor(myAES.Key, myAES.IV);
                using(MemoryStream msDecrypt = new MemoryStream(bdata))
                {
                    using (CryptoStream csDecrypt = new CryptoStream(msDecrypt, decryptor, CryptoStreamMode.Read))
                    {
                        using (BinaryReader brDecrypt = new BinaryReader(csDecrypt))
                        {
                            brDecrypt.Read(decryptedData, 0, decryptedData.Length);
                        }
                    }
                }
            }

            int size = 0;
            for (int i = 0; i < 4; ++i)
            {
                size += (decryptedData[i] << (8 * i));
            }
            if (size > decryptedData.Length - 4)
                throw new Exception("Invalid Size parameter of " + size);

            byte[] retData = new byte[size];
            Array.Copy(decryptedData, 4, retData, 0, size);
            

            return retData;

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

                p.DP = null;
                p.DQ = null;
                p.InverseQ = null;
            }

            return p;
        }
    }

}
