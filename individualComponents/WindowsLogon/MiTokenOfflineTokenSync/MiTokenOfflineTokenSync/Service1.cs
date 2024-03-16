using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Net;
using System.IO;
using System.Security.Principal;
using System.Security.AccessControl;
using System.Security.Cryptography;
using Microsoft.Win32;

namespace MiTokenOfflineTokenSync
{
    public partial class Service1 : ServiceBase
    {
        #region Data

        //flag for OTP Caching
        private readonly int OTPEX_FLAG_OTP_CACHING = 0x02;

        private byte[] m_bUsername;
        private RSA_Data keyData;
        private int ptrSize;
        private int m_totpCount;
        private int m_hotpCount;

        /*
         *  cacheHeader History
         *      0x0243544D (M T 0 0x02) - First POC Version
         *      0x0343544D (M T 0 0x03) - First version with all RSA Keys (Mod/Exp/D/DP/DQ/invQ/P/Q)
         *      0x0443544D (M T 0 0x04) - Added The Server's RSA Public Keys to the file format
         *      0x0543544D (M T C 0x05)
         *      0x0643544D (M T C 0x06) - Added TokenID prefix to HOTPs
         */

        private static readonly Int32 s_cacheHeaderMTCV5 = 0x0643544D;  // M T C 0x06
        private static readonly Int32 s_cacheHeaderMTCV4 = 0x0543544D;  // M T C 0x05
        private static readonly Int32[] s_validHeaders = { s_cacheHeaderMTCV4, s_cacheHeaderMTCV5 };

        //public static string APIServer = "127.0.0.1";
        public static readonly string s_Filepath = Path.Combine(Path.GetPathRoot(Environment.SystemDirectory), "Program Files\\Mi-Token\\Mi-Token Desktop Login\\cache.mtc");
        private static readonly string s_dbgFilepath = Path.Combine(Path.GetPathRoot(Environment.SystemDirectory), "Program Files\\Mi-Token\\Mi-Token Cache Updater - Client\\Log.txt");
        private static readonly int s_timeoutFetch = 15 * 60 * 1000;  // 15 min cache fetching timeout
        private static readonly string s_regKey = "Software\\Mi-Token\\Logon\\Config\\";
        private static readonly string s_eventName = "MT58917405";

        private Thread m_worker;
        private static readonly EventWaitHandle s_waitHandle;
        private static long s_stop;
        private static bool s_dummy;
        private static ManualResetEvent s_evt;
        private static EventWaitHandle s_ewh;
        private static RegisteredWaitHandle s_registerWaitHandle;
        private static Random s_rnd;

        private int m_encVersion = 1;
        private byte[] m_masterKey = new byte[0x20];
        private CacheSettings m_settings;

        #endregion

        #region Utility

        static Service1()
        {
            s_waitHandle = new AutoResetEvent(false);
            s_evt = new ManualResetEvent(false);
            s_rnd = new Random(Guid.NewGuid().GetHashCode());
        }

        public Service1()
        {
            s_dummy = true;
            InitializeComponent();

            m_totpCount = 1000;
            m_hotpCount = 100;
        }

        public static void Log(string message, EventLogEntryType type = EventLogEntryType.Information, string source = "Mi-Token Client Cache Updater")
        {
            const string log = "Application";
            if (!EventLog.SourceExists(source))
            {
                EventLog.CreateEventSource(source, log);
            }
            EventLog.WriteEntry(source, message, type);
        }

        public static void Trace(string message, bool bSupressTimestamp = false)
        {
#if _CONSOLE
            Console.WriteLine(message);
#else
            using (RegistryKey hkey = Registry.LocalMachine.OpenSubKey(s_regKey))
            {
                if (hkey != null)
                {
                    int svalue = (int)hkey.GetValue("ClientCacheDebug", 0);
                    if (svalue != 0)
                    {
                        var logmsg = bSupressTimestamp? message: string.Format("[{0}]   {1}", DateTime.Now.ToString("dd'/'MM'/'yyyy HH:mm:ss.ffff"), message);

                        System.IO.FileStream appender = new System.IO.FileStream(s_dbgFilepath, System.IO.FileMode.Append);
                        if (appender != null)
                        {
                            byte[] data = Encoding.ASCII.GetBytes(logmsg);
                            appender.Write(data, 0, data.Length);
                            appender.Close();
                        }
                    }
                }
            }
#endif
        }

        public static void writeDebugLogLine(string message, bool bSupressTimestamp = false)
        {
            Trace(message + Environment.NewLine, bSupressTimestamp);
        }

        public static void writeDebugLogInt(int value)
        {
            Trace(value.ToString());
        }

        protected override void OnStart(string[] args)
        {
            Log("Starting");
            writeDebugLogLine("OnStart - reading settings");

            if (!GetCacheSettings(out m_settings))
            {
                Log("Failed to start due to incorrect settings", EventLogEntryType.Error);
                writeDebugLogLine("OnStart - failed to read settings");
                this.ExitCode = 13;         // ERROR_INVALID_DATA 
                return;
            }

            if (m_settings.enabled == false)
            {
                Log("The service is not enabled", EventLogEntryType.Warning);
                writeDebugLogLine("OnStart - not enabled");
                this.ExitCode = 87;         // ERROR_INVALID_PARAMETER
                return;
            }
 
            Interlocked.Exchange(ref s_stop, 0L);
            s_waitHandle.Reset();

            m_worker = new Thread(() => workProcess());
            m_worker.Start();

            s_registerWaitHandle = ThreadPool.RegisterWaitForSingleObject(s_evt, (x, y) => CheckTip(x, y), s_waitHandle, 10000, false);
        }

        protected override void OnStop()
        {
            Log("Stopping");
            writeDebugLogLine("OnStop");

            if (m_worker != null && m_worker.ThreadState == System.Threading.ThreadState.Running)
            {
                Interlocked.Exchange(ref s_stop, 1L);
                s_waitHandle.Set();

                var ret = Utils.WaitWithTimeout(5000,
                                                obj =>
                                                {
                                                    var ct = (CancellationToken)obj;
                                                    while (!ct.IsCancellationRequested)
                                                    {
                                                        Thread.Sleep(100);
                                                        if (m_worker.ThreadState == System.Threading.ThreadState.Stopped)
                                                            return new byte[] { 1 };
                                                    }
                                                    return null;
                                                },
                                                new ConcurrentDictionary<Exception, int>());

                if (ret == null)
                {
                    Log("The service failed to shutdown", EventLogEntryType.Warning);
                    writeDebugLogLine("OnStop - failed to stop, terminating");
                    m_worker.Abort();
                    this.ExitCode = 351;         // ERROR_FAIL_SHUTDOWN
                }
            }

            if (s_registerWaitHandle != null)
              s_registerWaitHandle.Unregister(null);
        }

        #endregion

        #region Structures
        public enum CacheMode
        {
            CacheMode_Storage = 0,
            CacheMode_RAM = 1
        };
        public struct KeyData
        {
            public byte[] keyData;
            public int length;
        };
        public struct RSA_Data
        {
            public KeyData exponent, modulus, D, DP, DQ, inverseQ, P, Q, serverExponent, serverModulus;
            public int totalLength()
            {
                return exponent.keyData.Length + modulus.keyData.Length + D.keyData.Length +
                    DP.keyData.Length + DQ.keyData.Length + inverseQ.keyData.Length +
                    P.keyData.Length + Q.keyData.Length + serverExponent.keyData.Length + serverModulus.keyData.Length;
            }
        };
        public struct GenericOTPInfo
        {
            public Int32 tokenSize;
            public Int32 count;
            public Int32 lastUsed;
            public byte[] values;
        };
        public struct timeOTPInfo
        {
            public GenericOTPInfo genInfo;
            public Int32 timeSteps;
        };
        public struct eventOTPInfo
        {
            public GenericOTPInfo genInfo;
            public Int32 IDLength;
            public Byte[] ID;
        };
        public class TokenCache
        {
            public TokenCache(int version)
            {
                versionID = version;
                m_canStoredInV4file = true;
            }

            public readonly int versionID;
            public CacheMode currentMode;
            public bool m_canStoredInV4file;    // can the cache represented by this class be stored in v4 cache file

            public static readonly int s_headerOTPC = 0x4350544F; // OTPC [v4] in little endian.
            public static readonly int s_headerOTPD = 0x4450544F; // OTPD [v5] in little endian.

            public struct _header
            {
                public Int32 totalSize;
                public Int32 userLen;
                public byte[] username;
                RSA_Data keyInfo;
            };
            public _header header;

            public struct _timeOTPHeader
            {
                public Int32 timeTokens;
                public Int32 _padding;
                public Int64 timeStart;
                public List<timeOTPInfo> timeInfos;
            };
            public _timeOTPHeader timeOTPHeader;

            public struct _eventOTPHeader
            {
                public Int32 eventTokens;
                public List<eventOTPInfo> eventInfos;
            };
            public _eventOTPHeader eventOTPHeader;


        };
        #endregion
        #region Conversions
        public Int64 convertStreamToInt64(byte[] dataStream, ref int position)
        {
            Int64 value = 0;
            for (int i = 0; i < 8; ++i)
            {
                byte v = dataStream[position + i];
                // Casting to Int64 is required to prevent
                // the 64-bit 'value' from getting mangled
                value += (Int64)v << (8 * i);
            }
            position += 8;
            return value;
        }
        public Int32 convertStreamToInt32(byte[] dataStream, ref int position)
        {
            Int32 value = 0;
            for (int i = 0; i < 4; ++i)
            {
                byte v = dataStream[position + i];
                value += (v << (8 * i));
            }
            position += 4;
            return value;
        }
        public UInt32 convertStreamToPtr(byte[] dataStream, int ptrSize, ref int position)
        {
            if (ptrSize == 4)
                return (UInt32)convertStreamToInt32(dataStream, ref position);
            else if (ptrSize == 8)
                return (UInt32)((UInt64)convertStreamToInt64(dataStream, ref position));
            else
                throw new ArgumentException("Argument ptrSize was a not a valid value", "ptrSize");
        }
        public byte[] convertStreamToByteArr(byte[] dataStream, int ptrSize, ref int position)
        {
            int length;
            Int32 offset;
            byte[] data;
            writeDebugLogLine("\t\tPosition..." + position);
            Trace("\t\tOffset...");
            offset = (Int32)convertStreamToPtr(dataStream, ptrSize, ref position);
            Trace(offset + "\r\n");
            Trace("\t\tLength...");
            length = convertStreamToInt32(dataStream, ref position);
            writeDebugLogInt(length);
            if (ptrSize == 8)
            {
                position += 4;
            }
            Trace("\t\tData...");
            data = new byte[length];
            Array.Copy(dataStream, offset, data, 0, length);
            writeDebugLogLine("Done", true);
            return data;
        }

        public byte[] convertStreamToByteArrInline(byte[] dataStream, ref int position, int length)
        {
            byte[] data = new byte[length];
            Array.Copy(dataStream, position, data, 0, length);
            position += length;
            return data;
        }

        public void appendData(byte[] data, ref int position, byte[] value)
        {
            Array.Copy(value, 0, data, position, value.Length);
            position += value.Length;
        }
        public void appendData(byte[] data, ref UInt32 position, byte[] value)
        {
            Array.Copy(value, 0, data, position, value.Length);
            position += (UInt32)value.Length;
        }
        public void appendInt32(byte[] data, ref int position, Int32 value)
        {
            for (int i = 0; i < 4; ++i)
            {
                byte v = (byte)((value & (0xFF << (8 * i))) >> (8 * i));
                data[position + i] = v;
            }
            position += 4;
            return;
        }
        public void appendInt64(byte[] data, ref int position, Int64 value)
        {
            for (int i = 0; i < 8; ++i)
            {
                // Casting to Int64 is required to prevent the generated 
                // sequence of bytes from being mangled
                byte v = (byte)((value & ((Int64)0xFF << (8 * i))) >> (8 * i));
                data[position + i] = v;
            }
            position += 8;
            return;
        }
        public void appendPtr(byte[] data, ref int position, UInt32 value)
        {
            if (ptrSize == 4)
                appendInt32(data, ref position, (Int32)value);
            else if (ptrSize == 8)
                appendInt64(data, ref position, (Int64)(UInt64)value);
            else
                throw new ArgumentException("PtrSize is not 4 or 8", "PtrSize");
        }
        public void appendBytePtr(byte[] data, ref int position, ref uint scratch, byte[] source)
        {
            appendPtr(data, ref position, scratch);
            appendInt32(data, ref position, source.Length);
            if (ptrSize == 8)
            {
                appendInt32(data, ref position, 0);
            }
            appendData(data, ref scratch, source);
        }

        public timeOTPInfo convertStreamToTimeOTPHeader(byte[] dataStream, ref int position)
        {
            timeOTPInfo value;
            value.timeSteps = convertStreamToInt32(dataStream, ref position);
            value.genInfo.tokenSize = convertStreamToInt32(dataStream, ref position);
            value.genInfo.count = m_totpCount;
            value.genInfo.lastUsed = -1;
            int length = m_totpCount * value.genInfo.tokenSize;
            value.genInfo.values = new byte[length];
            Array.Copy(dataStream, position, value.genInfo.values, 0, length);
            position += length;

            return value;
        }

        /*
         Don't need this function cause we do have different token headers (OTPC and OTPD) and this information is duplicated by 
         the first byte of the cache header (5 or 4)
         
        public static bool isStreamV5(byte[] dataStream, int positionToTestAt)
        {
            writeDebugLogLine("+isStreamV5");


             * Because the OTPC header does not change for a V5 stream, we will instead do some deep inspection to see which version we are running
             * At positionToTestAt we should either be at:
             *  OTPCodes for V4
             *  IDLength for V5
             *  
             * Thus we will test if the next four bytes are all in the 0x30-0x39 range ('0' to '9' in ASCII)
             * V4 should satisfy this test
             * V5 should fail as at least 3 of the next 4 bytes should be 0x00 (because we expect either an Int32 of 0 or 12)
             *  
             * 

            bool isV5 = false;
            for (int i = 0; (i < 4) && (!isV5); ++i)
            {
                if ((dataStream[positionToTestAt + i] < (byte)'0') || (dataStream[positionToTestAt + i] > (byte)'9'))
                {
                    isV5 = true; //we have a byte that isn't between '0' and '9'
                    writeDebugLog(string.Format("Stream Version 5, Due to byte {0:X2}\r\n", dataStream[positionToTestAt + i]));
                }
            }

            writeDebugLogLine(string.Format("-isStreamV5 - {0}, isV5.ToString()"));
            return isV5;
        }
        */

        public eventOTPInfo convertStreamToEventOTPHeader(byte[] dataStream, ref int position, int version, out bool bPublicIdenityFound)
        {
            writeDebugLogLine("+convertStreamToEventOTPHeader");
            bPublicIdenityFound = false;

            eventOTPInfo value;
            value.genInfo.count = m_hotpCount;
            value.genInfo.lastUsed = -1;
            value.genInfo.tokenSize = convertStreamToInt32(dataStream, ref position);
            if (version == 5)
            {
                writeDebugLogLine("convertStreamToEventOTPHeader - v5");

                value.IDLength = convertStreamToInt32(dataStream, ref position);
                value.ID = new byte[12];
                for (int i = 0; i < 12; ++i)
                {
                    value.ID[i] = 0;
                }

                if (value.IDLength > 0)
                {
                    Array.Copy(dataStream, position, value.ID, 0, 12);
                    position += 12;
                    bPublicIdenityFound = true;
                }
            }
            else if (version == 4)
            {
                writeDebugLogLine("convertStreamToEventOTPHeader - v4");

                value.IDLength = 0;
                value.ID = new byte[12];
                for (int i = 0; i < 12; ++i)
                {
                    value.ID[i] = 0;
                }
            }
            else
            {
                throw new ArgumentOutOfRangeException("invalid version");
            }

            int length = m_hotpCount * value.genInfo.tokenSize;
            value.genInfo.values = new byte[length];
            Array.Copy(dataStream, position, value.genInfo.values, 0, length);
            position += length;

            writeDebugLogLine("-convertStreamToEventOTPHeader");
            return value;
        }
        #endregion

        #region Main Stuff
        public bool verifyServerData(byte[] data, ref int position, out int versionID)
        {
            writeDebugLogLine("+verifyServerData");

            versionID = 0;
            if (data.Length < 8)
            {
                writeDebugLogLine("-verifyServerData - length too short");
                return false;
            }
            versionID = convertStreamToInt32(data, ref position);
            writeDebugLogLine(string.Format("Version of Stream is {0}", versionID));
            if ((versionID != 0x00000004) && (versionID != 0x00000005)) //V4 and V5
            {
                writeDebugLogLine("-verifyServerData - invalid header");
                return false;
            }
            int length = convertStreamToInt32(data, ref position);
            if (length != data.Length)
            {
                writeDebugLogLine("-verifyServerData - length incorrect");
                return false;
            }

            writeDebugLogLine("-verifyServerData");
            return true;
        }

        /// <summary>
        /// Converts the data stream to a TokenCache structure
        /// Note, the returned structure still needs to be converted to a new byte stream for saving in the log file
        /// </summary>
        /// <returns> TokenCache </returns>
        public TokenCache processCacheStream(byte[] dataStream)
        {
            writeDebugLogLine("+processCacheStream");
            /*
                Token Codes Format:
             *  (V3)
                     [version][length][time 8][Server RSA Exp Len][Server RSA Mod Len][Server RSA Exp Key][Server RSA Mod Key][[totp c][totp Steps][hotp c][hotpSteps] { [totp step] [totp len] {totp token}} { [hotp len] {hotp token} }
            */
            int versionID;
            int position = 0;

            if (!verifyServerData(dataStream, ref position, out versionID))  // reads version and length
                throw new ArgumentException("DataStream verification failed", "dataStream");

            writeDebugLogLine(string.Format("processCacheStream - version {0}", versionID));

            TokenCache tcache = new TokenCache(versionID);

            int flags = convertStreamToInt32(dataStream, ref position);
            if ((flags & OTPEX_FLAG_OTP_CACHING) == 0)
                throw new ArgumentException("Cached Bypass flag was not set", "dataStream");

            int cacheOffset = convertStreamToInt32(dataStream, ref position);
            if (cacheOffset > dataStream.Length)
                throw new ArgumentException("Cache Location is beyond the end of the data stream", "dataStream");

            //move to the cache offset
            position = cacheOffset;

            //now verify the cache header
            int cacheHeaderBytes = convertStreamToInt32(dataStream, ref position);
            writeDebugLogLine(string.Format("processCacheStream - header: {0:X4}", cacheHeaderBytes));

            if (cacheHeaderBytes != TokenCache.s_headerOTPC && cacheHeaderBytes != TokenCache.s_headerOTPD)
                throw new ArgumentException("Cache Header is invalid");

            if (cacheHeaderBytes == TokenCache.s_headerOTPC && tcache.versionID != 4 ||
                cacheHeaderBytes == TokenCache.s_headerOTPD && tcache.versionID != 5)
                throw new ArgumentException("Cache header is inconsistent with token header");

            /*
             Commented out because we don't check cache file here, we rather check two headers inside the received cache and inconsistency (checked above) means API Service code is broken.
             
            //check if we are using a v5 cache block with a v4 MTC file
            if ((versionID == 0x00000004) && (cacheHeaderBytes == 0x4450544F))
            {
                throw new ArgumentException("Version Mismatch between Mi-Token Cache File and Cache Updater Block.\r\nMi-Token Cache File is Version : 5\r\nCache Updater is Version : 4.\r\nPlease update the Cache Updater Service Piece to fix this problem.");
            }
            */

            int cacheSize = convertStreamToInt32(dataStream, ref position);
            if (cacheSize + position - 8 > dataStream.Length) //-8 due to : -4 for the header read, and -4 for the length read
                throw new ArgumentException("Cache section would run past end of dataStream", "dataStream");

            tcache.timeOTPHeader.timeStart = convertStreamToInt64(dataStream, ref position);

            int RSAServerExpLen, RSAServerModLen;
            RSAServerExpLen = convertStreamToInt32(dataStream, ref position);
            RSAServerModLen = convertStreamToInt32(dataStream, ref position);
            byte[] ServerMod, ServerExp;
            ServerExp = convertStreamToByteArrInline(dataStream, ref position, RSAServerExpLen);
            ServerMod = convertStreamToByteArrInline(dataStream, ref position, RSAServerModLen);

            keyData.serverExponent.keyData = ServerExp;
            keyData.serverModulus.keyData = ServerMod;

            tcache.timeOTPHeader.timeTokens = convertStreamToInt32(dataStream, ref position);
            m_totpCount = convertStreamToInt32(dataStream, ref position);

            tcache.eventOTPHeader.eventTokens = convertStreamToInt32(dataStream, ref position);
            m_hotpCount = convertStreamToInt32(dataStream, ref position);

            tcache.timeOTPHeader.timeInfos = new List<timeOTPInfo>();
            tcache.eventOTPHeader.eventInfos = new List<eventOTPInfo>();
            for (int i = 0; i < tcache.timeOTPHeader.timeTokens; ++i)
            {
                tcache.timeOTPHeader.timeInfos.Add(convertStreamToTimeOTPHeader(dataStream, ref position));
            }

            for (int i = 0; i < tcache.eventOTPHeader.eventTokens; ++i)
            {
                bool bPublicIdentityFound;
                tcache.eventOTPHeader.eventInfos.Add(convertStreamToEventOTPHeader(dataStream, ref position, versionID, out bPublicIdentityFound));

                if (bPublicIdentityFound)
                    tcache.m_canStoredInV4file = false;
            }

            tcache.currentMode = CacheMode.CacheMode_RAM;
            tcache.header.userLen = 0;
            tcache.header.username = new byte[0];
            tcache.header.totalSize = -1;

            writeDebugLogLine("-processCacheStream");
            return tcache;
        }

        private void ReadMasterKey()
        {
            bool readKey = false;

            using (RegistryKey key = Registry.LocalMachine.OpenSubKey(s_regKey, true))
            {
                if (key != null)
                {
                    byte[] keydata = (byte[])key.GetValue("MKey");
                    if (keydata != null)
                    {
                        if (keydata.Length == 0x20)
                        {
                            Array.Copy(keydata, m_masterKey, 0x20);
                            readKey = true;
                        }
                    }
                    if (!readKey)
                    {
                        var rng = new RNGCryptoServiceProvider();
                        rng.GetBytes(m_masterKey);
                        key.SetValue("MKey", m_masterKey);
                    }
                }
                else
                {
                    writeDebugLogLine("Missing Registry key: " + s_regKey);
                }
            }
        }
        private void EncryptRoutine(ref byte[] data, int offset, int tokenSize, int tokenID, ref int keyIndex, ref int countIndex)
        {
            if (m_encVersion > 0)
            {
                byte lastLetterVal = 0;
                for (int tokenSpot = 0; tokenSpot < tokenSize; ++tokenSpot)
                {
                    byte mask = lastLetterVal;
                    lastLetterVal = data[offset + tokenSpot];
                    mask ^= m_masterKey[keyIndex];
                    keyIndex++;
                    keyIndex &= 0x1F;
                    mask ^= (byte)(tokenID * tokenSize);
                    mask ^= (byte)tokenSpot;
                    //writeDebugLogLine(string.Format("Encryption {0} with {1} = {2}", data[offset + tokenSpot], mask, data[offset + tokenSpot] ^ mask));
                    data[offset + tokenSpot] ^= mask;
                }
            }
        }

        private void EncryptTokenList(GenericOTPInfo tokenList)
        {
            //writeDebugLogLine(string.Format("Encrypting Token List... {0} codes", tokenList.count));
            int keyIndex = 0, countIndex = 0;
            int tokenSize = tokenList.tokenSize;
            for (int tokenCode = 0; tokenCode < tokenList.count; ++tokenCode)
            {
                int offset = (tokenCode * tokenSize);
                EncryptRoutine(ref tokenList.values, offset, tokenSize, tokenCode, ref keyIndex, ref countIndex);
            }
        }


        public byte[] generateCacheFile(TokenCache cache, int versionID)
        {
            writeDebugLogLine(string.Format("+generateCacheFile - version {0}", versionID));

            ReadMasterKey();
            byte[] data;
            int length = 0;
            UInt32 scratchLocation = 0;
            int writePos = 0;
            int timeTokenStructSize = 0;
            int eventTokenStructSize = 0;
            #region TokenCache Structure size
            //calculate the basic length of the TOKEN_CACHE structure (ignoring internal structures)
            length = 16 //cacheHeader / pointerSize / curMode / totalSize
                + (4 + ptrSize) //header [userlen + username]
                + (16 + ptrSize)//timeOTPHeader [timeTokens / _padding + timeStart + timeInfos]
                + (4 + ptrSize) //eventOTPHeader [eventTokens + eventInfos]
                ;

            //now add the length of the keyInfo struct

            //keyData [length + pos of (exponent, mod, D, DP, DQ, invQ, P, Q, serverExp, serverMod)]
            //(10 * 4 * (ptrSize == 8 ? 1 : 0)) => Add 4 bytes per keydata item if the ptrSize is 8 (due to padding)
            length += 40 + (10 * ptrSize) + (10 * 4 * (ptrSize == 8 ? 1 : 0));

            scratchLocation = (UInt32)length;
            /*
            //now add the length of the timeOTPInfo (not including GenericOTPInfo)
            length += 4; //timeOTPInfo [timeStep]

            //EventOTPInfo only has the internal GenericOTPInfo so ignore it for the time being

            //Now we have 2 GenericOTPInfo structs (one in timeOTPInfo, one in eventOTPInfo)
            length += 2 * (12 + ptrSize); //GenericOTPInfo [tokenSize/count/lastUsed / values]
            */
            #endregion

            #region Pointer Element Size
            //So that is the basic length calculation done, now we need to add the size of what we point to

            //username pointers size is held in the bUsername.length
            length += m_bUsername.Length;

            //get the total length of all the keys
            length += keyData.totalLength();

            //now the timeTokens
            timeTokenStructSize = 4 //timeSteps
                + 12 + ptrSize; // GenericOTPInfo [tokenSize / count / lastUsed / ptr(values) ]
            length += timeTokenStructSize * (cache.timeOTPHeader.timeInfos.Count);

            //now the eventTokens
            if (versionID >= 5)
            {
                eventTokenStructSize = 12 + ptrSize + 4 + 12; //GenericOTPInfo [tokenSize / count / lastUsed / ptr(values) ] + IDLength + ID
            }
            else if (versionID == 4)
            {
                eventTokenStructSize = 12 + ptrSize; //GenericOTPInfo [tokenSize / count / lastUsed / ptr(values) ] 
            }
            length += eventTokenStructSize * (cache.eventOTPHeader.eventInfos.Count);
            #endregion

            #region OTP Element Size

            //add the length of each time based token value
            foreach (timeOTPInfo toi in cache.timeOTPHeader.timeInfos)
                length += (toi.genInfo.values.Length);

            //add the length of each event based token value
            foreach (eventOTPInfo eoi in cache.eventOTPHeader.eventInfos)
                length += (eoi.genInfo.values.Length);

            #endregion

            data = new byte[length];

            //pack the data
            /*
                length is the total length of the data array, 
                scratchLocation is currently the location after the tokenCache struction
                writePos is currently 0 and will be used when writing to the tokenCache structure
             */

            //write tokenCache
            if (versionID == 5)
            {
                appendInt32(data, ref writePos, s_cacheHeaderMTCV5); //cacheHeader (M T C 0x06)
            }
            else if (versionID == 4)
            {
                appendInt32(data, ref writePos, s_cacheHeaderMTCV4); //cacheHeader (M T C 0x05)
            }
            else
            {
                throw new ArgumentOutOfRangeException("unsupported version");
            }

            appendInt32(data, ref writePos, ptrSize); //ptrSize
            appendInt32(data, ref writePos, (Int32)CacheMode.CacheMode_Storage); //storageMode
            appendInt32(data, ref writePos, length); //totalSize

            appendInt32(data, ref writePos, m_bUsername.Length); //userlen
            appendPtr(data, ref writePos, scratchLocation); //username (ptr)
            writeDebugLogLine("Adding Username : " + Encoding.Unicode.GetString(m_bUsername) + " @" + scratchLocation);
            appendData(data, ref scratchLocation, m_bUsername); //username (value)

            for (int upos = 0; upos < m_bUsername.Length; ++upos)
            {
                Trace(string.Format("{0:X2} ", m_bUsername[upos]), true);
            }

            writeDebugLogLine(string.Empty, true);
            //write key data
            writeDebugLogLine("Exponent @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.exponent.keyData);
            writeDebugLogLine("Modulus @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.modulus.keyData);
            writeDebugLogLine("D @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.D.keyData);
            writeDebugLogLine("DP @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.DP.keyData);
            writeDebugLogLine("DQ @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.DQ.keyData);
            writeDebugLogLine("Q^-1 @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.inverseQ.keyData);
            writeDebugLogLine("P @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.P.keyData);
            writeDebugLogLine("Q @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.Q.keyData);
            writeDebugLogLine("Server Exponent @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.serverExponent.keyData);
            writeDebugLogLine("Server Modulus @ " + scratchLocation);
            appendBytePtr(data, ref writePos, ref scratchLocation, keyData.serverModulus.keyData);

            /*
            writeKeyData(data, ref writePos, ref scratchLocation);
            appendInt32(data, ref writePos, privKey.Length); //privKeyLen
            appendInt32(data, ref writePos, pubKey.Length); //pubKeyLen
            appendPtr(data, ref writePos, scratchLocation); //privKey (ptr);
            appendData(data, ref scratchLocation, privKey); //privKey (value);
            appendPtr(data, ref writePos, scratchLocation); //pubkey (ptr);
            appendData(data, ref scratchLocation, pubKey); //pubkey(value)
            */

            appendInt32(data, ref writePos, cache.timeOTPHeader.timeInfos.Count); //timeTokens
            appendInt32(data, ref writePos, 0); //padding
            appendInt64(data, ref writePos, cache.timeOTPHeader.timeStart); //timeStart;
            appendPtr(data, ref writePos, scratchLocation); //timeInfos (ptr)
            {
                int structWP = (int)scratchLocation; //the location we will write the structure info to

                //fix scratch location to point past the end of where we will write structure data to
                scratchLocation += (UInt32)(timeTokenStructSize * cache.timeOTPHeader.timeInfos.Count);
                foreach (timeOTPInfo toi in cache.timeOTPHeader.timeInfos)
                {
                    appendInt32(data, ref structWP, toi.genInfo.tokenSize); //tokenSize
                    appendInt32(data, ref structWP, toi.genInfo.count); //tokenCount
                    appendInt32(data, ref structWP, toi.genInfo.lastUsed); //lastUsed
                    appendPtr(data, ref structWP, scratchLocation); //values (ptr)
                    EncryptTokenList(toi.genInfo);
                    appendData(data, ref scratchLocation, toi.genInfo.values); //values (value)
                    appendInt32(data, ref structWP, toi.timeSteps); //timeStep Size
                }
            }//timeInfos (data)

            appendInt32(data, ref writePos, cache.eventOTPHeader.eventInfos.Count); //eventTokens
            appendPtr(data, ref writePos, scratchLocation); //eventInfos (ptr)
            {
                int structWP = (int)scratchLocation; //the location we will write the structure info to

                //fix scratch location to point past the end of where we will write structure data to
                scratchLocation += (UInt32)(eventTokenStructSize * cache.eventOTPHeader.eventInfos.Count);
                foreach (eventOTPInfo eoi in cache.eventOTPHeader.eventInfos)
                {
                    appendInt32(data, ref structWP, eoi.genInfo.tokenSize); //tokenSize
                    appendInt32(data, ref structWP, eoi.genInfo.count); //tokenCount
                    appendInt32(data, ref structWP, eoi.genInfo.lastUsed); //lastUsed
                    appendPtr(data, ref structWP, scratchLocation); //values (ptr)
                    EncryptTokenList(eoi.genInfo);
                    appendData(data, ref scratchLocation, eoi.genInfo.values); //values (value)
                    if (versionID >= 5)
                    {
                        appendInt32(data, ref structWP, eoi.IDLength); //ID Length
                        appendData(data, ref structWP, eoi.ID); //ID Value (12 bytes)
                    }
                }
            }//eventInfos(data)

            if (scratchLocation != length)
                throw new InvalidOperationException("Scratch Location != Buffer length, length calculation must have been off");

            writeDebugLogLine("-generateCacheFile");
            return data;
        }

        byte[] emulateAPICall()
        {
            //AnonVerifyOTPCached
            FileStream fstream = new FileStream("C:\\Debug\\AnonVerifyOTPCached", FileMode.Open);
            byte[] data = new byte[(int)fstream.Length];
            fstream.Read(data, 0, (int)fstream.Length);
            fstream.Close();
            return data;
        }

        void copyByteArr(byte[] source, out byte[] dest)
        {
            dest = new byte[source.Length];
            Array.Copy(source, dest, dest.Length);
        }

        void copyRSAKeys(ref RSAParameters paramPriv, ref RSAParameters paramPub)
        {
            copyByteArr(keyData.D.keyData, out paramPriv.D);
            copyByteArr(keyData.DP.keyData, out paramPriv.DP);
            copyByteArr(keyData.DQ.keyData, out paramPriv.DQ);
            copyByteArr(keyData.exponent.keyData, out paramPriv.Exponent);
            copyByteArr(keyData.inverseQ.keyData, out paramPriv.InverseQ);
            copyByteArr(keyData.modulus.keyData, out paramPriv.Modulus);
            copyByteArr(keyData.P.keyData, out paramPriv.P);
            copyByteArr(keyData.Q.keyData, out paramPriv.Q);
            copyByteArr(keyData.serverExponent.keyData, out paramPub.Exponent);
            copyByteArr(keyData.serverModulus.keyData, out paramPub.Modulus);
        }

#if DEPRECIATED
        void demoWriteInt32(FileStream stream, int value)
        {
            byte[] data = new byte[4];
            for (int i = 0; i < 4; ++i)
            {
                data[i] = (byte)(value & 0xFF);
                value >>= 8;
            }

            stream.Write(data, 0, 4);
        }

        void demoWriteByteArr(FileStream stream, byte[] arr)
        {
            demoWriteInt32(stream, arr.Length);
            stream.Write(arr, 0, arr.Length);
        }
        void demoWritePubKeyFile()
        {
            FileStream fstream = new FileStream("Server_Pub.key", FileMode.Create);
            demoWriteByteArr(fstream, keyData.exponent.keyData);
            demoWriteByteArr(fstream, keyData.modulus.keyData);
            fstream.Close();
        }
#endif
        struct CacheSettings
        {
            public string customerName;
            public int pollPeriod;
            public int waitPeriod;
            public string remoteServer;
            public bool enabled;

        };
        bool GetCacheSettings(out CacheSettings settings)
        {
            settings = new CacheSettings();

            try
            {
                using (RegistryKey key = Registry.LocalMachine.OpenSubKey(s_regKey, false))
                {
                    if (key != null)
                    {
                        if ((key.GetValueKind("CacheServerCustomerName") == RegistryValueKind.String) &&
                            (key.GetValueKind("CacheServerAddress") == RegistryValueKind.String) &&
                            (key.GetValueKind("CachePollInSeconds") == RegistryValueKind.DWord) &&
                            (key.GetValueKind("CacheClientWaitInMin") == RegistryValueKind.DWord) &&
                            (key.GetValueKind("EnableOTPCaching") == RegistryValueKind.DWord))
                        {
                            //everything is correct

                            settings.customerName = (string)key.GetValue("CacheServerCustomerName");
                            settings.remoteServer = (string)key.GetValue("CacheServerAddress");
                            settings.pollPeriod = (int)key.GetValue("CachePollInSeconds");
                            settings.pollPeriod = Math.Max(30, settings.pollPeriod);                   // no less than 30 sec
                            settings.pollPeriod *= 1000;                                               //settings stores it in ms not seconds.
                            settings.waitPeriod = (int)key.GetValue("CacheClientWaitInMin");
                            settings.waitPeriod = Math.Max(30, settings.waitPeriod);                   // no less than 30 min
                            settings.enabled = (int)key.GetValue("EnableOTPCaching") > 0;
                            writeDebugLogLine(string.Format("Read Settings from Registry\n\tName : {0}\n\tRemote Server : {1}\n\tPoll Period : {2}\n\tEnabled : {3}", settings.customerName, settings.remoteServer, settings.pollPeriod, settings.enabled));
                            return !string.IsNullOrWhiteSpace(settings.customerName) && !string.IsNullOrWhiteSpace(settings.remoteServer);
                        }
                        else
                        {
                            writeDebugLogLine("Missing or invalid Registry key data: " + s_regKey);
                        }
                    }
                    else
                    {
                        writeDebugLogLine("Missing Registry key: " + s_regKey);
                    }
                }

                return false;
            }
            catch (Exception ex)
            {
                writeDebugLogLine(string.Format("Failed to read settings, exception {0}: {1}\r\n", ex.GetType().ToString(), ex.Message));
                return false;
            }
        }

        byte[] ClientAPI()
        {

            //demoWritePubKeyFile();
            CacheSettings settings;
            if (GetCacheSettings(out settings))
            {
                writeDebugLogLine("Checking if enabled");
                if (settings.enabled)
                {
                    writeDebugLogLine("Creating CacheOTPClientThread");
                    var client = new CacheOTPClient(new Action<string, bool>(writeDebugLogLine), settings.customerName, settings.pollPeriod, settings.remoteServer, username, false);
                    copyRSAKeys(ref client.decryptKey, ref client.encryptKey);

                    var dic = new ConcurrentDictionary<Exception, int>();
                    byte[] result = Utils.WaitWithTimeout(s_timeoutFetch, new Func<object, byte[]>(client.Start), dic);

                    if (result == null || result.Length == 0)
                    {
                        if (dic.Count > 0)
                        {
                            foreach (var ex in dic.Keys)
                            {
                                var e = ex.InnerException == null ? ex : ex.InnerException;
                                writeDebugLogLine(string.Format("Exception {0} while fetching the cache: {1}", e.GetType().ToString(), e.Message));
                            }
                        }
                    }

                    return result;
                }
            }
            return null;
        }
        string username;

#if DEPRECIATED
        void debugExportInt(FileStream stream, int value)
        {
            byte[] data = new byte[4];
            for (int i = 0; i < 4; ++i)
            {
                data[i] = (byte)(value & 0xFF);
                value >>= 8;
            }
            stream.Write(data, 0, 4);
        }

        void debugExportKey(FileStream stream, KeyData key)
        {
            debugExportInt(stream, key.keyData.Length);
            stream.Write(key.keyData, 0, key.keyData.Length);
        }

        void debugExportKeys()
        {
            FileStream stream = new FileStream("ExportKeys.key", FileMode.Create);
            debugExportKey(stream, keyData.exponent);
            debugExportKey(stream, keyData.modulus);
            debugExportKey(stream, keyData.D);
            debugExportKey(stream, keyData.DP);
            debugExportKey(stream, keyData.DQ);
            debugExportKey(stream, keyData.inverseQ);
            debugExportKey(stream, keyData.P);
            debugExportKey(stream, keyData.Q);
            stream.Close();
 
        }
#endif
#if DEBUG
        //Only allow UseProtectData on DEBUG builds
        bool UseProtectData()
        {
            writeDebugLogLine("Checking if we should encrypt the data");
            RegistryKey key = null;
            key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\Logon");
            if (key != null)
            {
                object disableEncrypt = key.GetValue("DisableCacheEncrypt", null);
                if (disableEncrypt != null)
                {
                    if (((Int32)disableEncrypt) == 1)
                    {
                        writeDebugLogLine("Data will not be encrypted");
                        key.Close();
                        return false; //no encryption is needed
                    }
                }
                key.Close();
            }
            writeDebugLogLine("Data will be encrypted");
            return true;
        }
#else
        bool UseProtectData()
        {
            return true;
        }
#endif

        bool EncryptFile(ref byte[] fileData)
        {
            try
            {
                if (UseProtectData())
                {
                    Trace("Encrypting file...");
                    fileData = ProtectedData.Protect(fileData, null, DataProtectionScope.CurrentUser);
                    writeDebugLogLine("Done", true);
                }
                return true;
            }
            catch (Exception ex)
            {
                writeDebugLogLine("Failed\r\n" + ex.Message);
                return false; //failed for some reason.
            }
        }

        bool DecryptFile(ref byte[] fileData)
        {
            try
            {
                if (UseProtectData())
                {
                    Trace("Decrypting file...");
                    fileData = ProtectedData.Unprotect(fileData, null, DataProtectionScope.CurrentUser);
                    writeDebugLogLine("Done", true);
                }
                return true;
            }
            catch (Exception ex)
            {
                writeDebugLogLine("Exception: " + ex.Message);
                return false;
            }
        }

        void workProcess()
        {
            //we need to grab the Username from the file
            writeDebugLogLine("Started work");

            int cacheFileVersion;

            #region Inital Setup
            try
            {
                writeDebugLogLine("Opening Cache.mtc");
                FileStream fstream = new FileStream(s_Filepath, FileMode.Open);

                if (fstream.Length < 4)
                {
                    //invalid file - do nothing
                    fstream.Close();
                    return;
                }
                writeDebugLogLine("Reading File Data");
                byte[] file = new byte[(int)fstream.Length];
                fstream.Read(file, 0, file.Length);
                writeDebugLogLine("Closing File");
                fstream.Close();

                if (!DecryptFile(ref file)) //decrypt the file
                {
                    Log("Cache file not be decrypted. Make sure you are using the latest version of the Mi-Token Credential Provider.", EventLogEntryType.Warning);
                    return;
                }
                int pos = 0;

                Trace("Getting cache file version...");
                cacheFileVersion = convertStreamToInt32(file, ref pos);
                writeDebugLogLine(string.Format("cache file version {0:X4}", cacheFileVersion), true);

                bool validCacheHeader = false;
                foreach (Int32 i32 in s_validHeaders)
                {
                    if (cacheFileVersion == i32)
                    {
                        validCacheHeader = true;
                        break;
                    }
                }

                if (!validCacheHeader)
                {
                    writeDebugLogLine("Cache file version is invalid");
                    return; //invalid cache version
                }

                writeDebugLogLine("Getting Pointer Size...");
                ptrSize = convertStreamToInt32(file, ref pos);
                writeDebugLogInt(ptrSize);
                //check the file is large enough, and ptrSize was a valid value
                if ((file.Length < (16 + ptrSize)) || ((ptrSize != 4) && (ptrSize != 8)))
                {
                    //invalid file - do nothing
                    writeDebugLogLine("Invalid cache file length");
                    return;
                }

                Trace("Getting Username Size...");
                //get the size of the username
                pos += 8; //we are currently at curmode and we want userlen
                int usernameLength = convertStreamToInt32(file, ref pos);
                writeDebugLogInt(usernameLength);


                Trace("Getting Username Offset...");
                //get the offset to the username
                int offset = (int)convertStreamToPtr(file, ptrSize, ref pos);
                writeDebugLogInt(offset);

                Trace("Getting username...");
                m_bUsername = new byte[usernameLength];
                Array.Copy(file, offset, m_bUsername, 0, usernameLength);

                //convert the string
                username = Encoding.Unicode.GetString(m_bUsername);
                writeDebugLogLine(string.Format("Username: {0}", username));

                //now to grab the private and public key from the file cache
                //int privKeyLen, pubKeyLen;
                pos = 20 + ptrSize; //20 + ptrSize so we skip (cacheHeader/pointerSize/curMode/totalSize/userlen/ username(ptr)) and get is to privKeyLen


                writeDebugLogLine("Getting Key Data");
                writeDebugLogLine("\tExponent");
                keyData.exponent.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tModulus");
                keyData.modulus.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tD");
                keyData.D.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tDP");
                keyData.DP.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tDQ");
                keyData.DQ.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tQ^-1");
                keyData.inverseQ.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tP");
                keyData.P.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tQ");
                keyData.Q.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tServer Exponent");
                keyData.serverExponent.keyData = convertStreamToByteArr(file, ptrSize, ref pos);
                writeDebugLogLine("\tServer Modulus");
                keyData.serverModulus.keyData = convertStreamToByteArr(file, ptrSize, ref pos);

                /*
                 If cache file is v5 then it has storage space to accomodate extra data (public identity of HOTP tokens and its length)
                 so it can store both v5 and v4 data received from server updater. So no point to run 'can upgrade v4 to v5' check on v5 cache file.
                 We need only cater for the case when the cache file is v4 and the data received is v5 in which case we do need to check the received
                 data (not file) to see if it actually contaions HOTP tokens with public identity. And even in this case we don't need an extra check
                 to run through all HOTP tokens, we can use routine provcessing of HOTP tokens to raise a falg 'HOTP token with public identity found'.
                
                if (m_cacheFileVersion == cacheHeaderV5)
                {
                    writeDebugLogLine("Cache file version is v5."); 

                    canUpgradeV4ToV5 = true;
                    //the location where the offset for eventInfos is
                    int eventData = (pos + 16 + ptrSize);
                    writeDebugLogLine(string.Format("EventData is at {0}", eventData));
                    int eventCount = convertStreamToInt32(file, ref eventData);
                    uint eventOff = convertStreamToPtr(file, ptrSize, ref eventData);
                    writeDebugLogLine(string.Format("Checking HOTP data for {0} Tokens at {1}", eventCount, eventOff));
                    for (int i = 0; i < eventCount; ++i)
                    {
                        int eventInnerData = (int)eventOff + (i * (30 + ptrSize)); //30 + ptrsize = size of v5 HOTP block
                        int loc = eventInnerData + (12 + ptrSize); //12 + ptrSize = GENERIC_OTP_INFO block
                        int idlen = convertStreamToInt32(file, ref loc);
                        //we can upgrade a v4 block to v5 as long as no HOTP tokens have an ID
                        writeDebugLogLine(string.Format("Token {0} has IDLen of {1}", i, idlen));
                        if (idlen != 0)
                        {
                            canUpgradeV4ToV5 = false;
                        }
                    }
                    writeDebugLogLine(string.Format("Can Upgrade V4 to V5 : {0}", (canUpgradeV4ToV5? "TRUE": "FALSE")));
                }
                else
                {
                    writeDebugLogLine("Cache file version is not v5.");
                }
                */
            }
            catch (System.IO.DirectoryNotFoundException ex)
            {
                //directory doesn't exist, do nothing
                writeDebugLogLine(ex.Message);
                return;
            }
            catch (System.IO.FileNotFoundException ex)
            {
                //file could not be found... do nothing
                writeDebugLogLine(ex.Message);
                return;
            }
            catch (Exception ex)
            {
                writeDebugLogLine(ex.Message);
                return;
            }
            #endregion

            //string APIRequest = string.Format("https://{0}/mi-token/api/authentication/v4/rawhttpwithssl/AnonVerifyOTPCached?username={1}", APIServer, username);

            var tsWait = new TimeSpan(0, m_settings.waitPeriod, 0);
            DateTime dtLastFetch = DateTime.MinValue;
            DateTime dtLastNoFetch = DateTime.MinValue;
            TimeSpan tsLastFetchInterval = new TimeSpan(0, m_settings.waitPeriod * 4, 0);
            TimeSpan tsLastNoFetchInterval = new TimeSpan(0, 15, 0);

            while (true)
            {
                try
                {
                    writeDebugLogLine(string.Format("Checkpoint 1 ({0})", m_settings.waitPeriod)); 
                    s_waitHandle.WaitOne(tsWait);
                    writeDebugLogLine(string.Format("Checkpoint 2 ({0})", m_settings.pollPeriod));

                    if (Interlocked.Read(ref s_stop) > 0L)
                    {
                        writeDebugLogLine("Exiting due to stopping request");
                        break;
                    }

                    if (dtLastFetch != DateTime.MinValue)
                    {
                        TimeSpan tsInterval = DateTime.Now - dtLastFetch;

                        if (tsInterval < tsLastFetchInterval)
                        {
                            writeDebugLogLine("Skipping update");
                            continue;
                        }
                    }

                    if (dtLastNoFetch != DateTime.MinValue)
                    {
                        TimeSpan tsInterval = DateTime.Now - dtLastNoFetch;

                        if (tsInterval < tsLastNoFetchInterval)
                        {
                            writeDebugLogLine("Skipping over the update");
                            continue;
                        }
                    }


                    Log("Starting a new Cache Update Call");
                    writeDebugLogLine("Starting API Call");

                    byte[] apiResult = ClientAPI(); // doApiCall(APIRequest);
                    if (apiResult != null)
                    {
                        writeDebugLogLine("API call returned " + apiResult.Length + " bytes");
                        Log(string.Format("Cache Update returned {0} bytes", apiResult.Length));
                        dtLastFetch = DateTime.Now;

                        TokenCache cache = processCacheStream(apiResult);
                        writeDebugLogLine(string.Format("Received cache version {0}", cache.versionID));

                        //check if we can do the update needed
                        if (cacheFileVersion == s_cacheHeaderMTCV4 && cache.versionID == 5 && cache.m_canStoredInV4file == false)
                        {
                            //trying to update v4 data with a v5 update, don't allow this
                            var err = "Cache file is v4, cache received is v5. Not updating the cache due to detected incompatibility.";
                            writeDebugLogLine(err);
                            Log(err, EventLogEntryType.Error);
                        }
                        else
                        {
                            if (cacheFileVersion == s_cacheHeaderMTCV4 && cache.versionID == 5 && cache.m_canStoredInV4file == true)
                            {
                                string err = "Mismatch between the cache file (V4) and cache update received (V5).\r\nCache Updater is still able to update the cache because no HOTP tokens contain Public IDs.";
                                writeDebugLogLine(err);
                                Log(err, EventLogEntryType.Warning);  // warning because the ability to update is fragile, as soon as a HOTP token with public identity is added the cache update will be rejected.
                            }

                            if (cacheFileVersion == s_cacheHeaderMTCV5 && cache.versionID == 4)
                            {
                                string err = "Mismatch between the cache file (V5) and cache update received (V4).\r\nCache Updater is still able to update the cache.";
                                writeDebugLogLine(err);
                                Log(err, EventLogEntryType.Information);
                            }

                            //now we have a new cache - we need to generate a new bytestream for the output file
                            byte[] fileData = generateCacheFile(cache, cacheFileVersion == s_cacheHeaderMTCV4 ? 4 : 5);

                            //encrypt the file
                            if (!EncryptFile(ref fileData))
                            {
                                Log("Cache could not be encrypted. New cache not saved!", EventLogEntryType.Error);
                            }
                            else
                            {
                                FileStream fstream = new FileStream(s_Filepath, FileMode.Create);
                                fstream.Write(fileData, 0, fileData.Length);
                                fstream.Close();
                                Log("Cache Updated");
                            }
                        }
                    }
                    else
                    {
                        writeDebugLogLine("API Call failed");
                        Log("Cache Update Failed", EventLogEntryType.Error);
                        dtLastNoFetch = DateTime.Now;
                    }
                }
                catch (ArgumentException e)
                {
                    Log(string.Format("The API Stream Returned by the Server Cache Updater was invalid. Mi-Token Client Cache Updater was unable to update the Cache on this machine.\r\nExtra Info : {1}\r\nStack Trace:\r\n{0}", e.StackTrace, e.Message), EventLogEntryType.Error);
                    //Invalid API Stream - while we cannot do anything here, it isn't critical enough to close the process
                    writeDebugLogLine("Error : Invalid API Stream! :\r\n" + e.Message);
                    writeDebugLogLine(e.StackTrace);
                    if (e.InnerException != null)
                    {
                        writeDebugLogLine("\r\nInner exception: \r\n" + e.Message);
                    }
                    dtLastNoFetch = DateTime.Now;
                }
                catch (Exception e)
                {
                    writeDebugLogLine("Exception : " + e.Message);
                    dtLastNoFetch = DateTime.Now;
                }

            }  // while(true)
        }
        #endregion

        private static void CheckTip(object state, bool timeout)
        {
            if (Interlocked.Read(ref s_stop) > 0L)
            {
                writeDebugLogLine("CheckTip - exiting due to stopping request");
                return;
            }

            EventWaitHandle h = state as EventWaitHandle;

            if (h == null)
            {
                writeDebugLogLine("CheckTip - invalid argument");
                return;
            }

            if (s_ewh == null)
            try
            {
                s_ewh = EventWaitHandle.OpenExisting(s_eventName, EventWaitHandleRights.FullControl /*EventWaitHandleRights.Synchronize | EventWaitHandleRights.Modify*/);
                writeDebugLogLine("CheckTip - opened");
            }
            catch (WaitHandleCannotBeOpenedException)
            {
                SecurityIdentifier sidSY = new SecurityIdentifier(WellKnownSidType.LocalSystemSid, null);
                EventWaitHandleAccessRule ruleSY = new EventWaitHandleAccessRule(sidSY, EventWaitHandleRights.FullControl /*EventWaitHandleRights.Synchronize | EventWaitHandleRights.Modify*/, AccessControlType.Allow);
                EventWaitHandleSecurity security = new EventWaitHandleSecurity();
                security.AddAccessRule(ruleSY);
#if DEBUG
                SecurityIdentifier sidBA = new SecurityIdentifier(WellKnownSidType.BuiltinAdministratorsSid, null);
                EventWaitHandleAccessRule ruleBA = new EventWaitHandleAccessRule(sidBA, EventWaitHandleRights.FullControl /*EventWaitHandleRights.Synchronize | EventWaitHandleRights.Modify*/, AccessControlType.Allow);
                security.AddAccessRule(ruleBA);
#endif
                try
                {
                    bool wasCreated;
                    s_ewh = new EventWaitHandle(false, EventResetMode.ManualReset, s_eventName, out wasCreated, security);

                    if (!wasCreated)
                    {
                        writeDebugLogLine(string.Format("CheckTip - failure to create"));
                        return;
                    }

                    writeDebugLogLine("CheckTip - created");
                }
                catch (Exception ex)
                {
                    writeDebugLogLine(string.Format("CheckTip - failed to create {0} : {1}", ex.GetType().ToString(), ex.Message));
                    return;
                }
            }
            catch (Exception ex)
            {
                writeDebugLogLine(string.Format("CheckTip - exception {0} : {1}", ex.GetType().ToString(), ex.Message));
                return;
            }

            try
            {
                var ret = s_ewh.WaitOne(0);

                if (ret)
                {
                    s_ewh.Reset();

                    if (!System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
                    {
                        writeDebugLogLine("Tip unavailable");
                        return;
                    }

                    int delay = s_rnd.Next(0, 1000 * 300);
                    Utils.ExecuteWithDelay(delay, () => h.Set(), writeDebugLogLine);
                    writeDebugLogLine("Tip delay: " + delay.ToString());
                }
            }
            catch (Exception ex)
            {
                writeDebugLogLine(string.Format("CheckTip - exception caught {0} : {1}", ex.GetType().ToString(), ex.Message));
            }
        }
    }
}
    
