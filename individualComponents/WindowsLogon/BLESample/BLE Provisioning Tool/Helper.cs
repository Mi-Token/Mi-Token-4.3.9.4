using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Security.Cryptography;
using System.Globalization;

namespace BLE_Provisioning_Tool
{
    class Helper
    {
        static public bool ArraysEqual<T>(T[] a1, T[] a2)
        {
            if (a1 == null || a2 == null)
                return false;

            if (ReferenceEquals(a1, a2))
                return true;

            if (a1.Length != a2.Length)
                return false;

            EqualityComparer<T> comparer = EqualityComparer<T>.Default;
            for (int i = 0; i < a1.Length; i++)
            {
                if (!comparer.Equals(a1[i], a2[i])) return false;
            }
            return true;
        }

        static public string GetExportHexString<T>(T[] data)
        {
            int len = data.Length;
            if (len < 6)
                return "";

            StringBuilder hex = new StringBuilder(len * 2 + (len - 1));
            for (int i = len - 1; i >= 0; --i)
            {
                hex.AppendFormat("{0:X2}", data.ElementAt(i));
            }
            return hex.ToString();
        }

        static public string GetSystemIdString(byte[] data)
        {
            int len = data.Length;
            if (len < Constants.SYSTEM_ID_LEN)
                return "";

            return BitConverter.ToString(data);
        }

        //Function to get random number
        private static readonly RNGCryptoServiceProvider rand = new RNGCryptoServiceProvider();
        private static readonly object syncLock = new object();
        public static void CreateRandomBytes(byte[] result)
        {
            lock (syncLock)
            { // synchronize
                rand.GetBytes(result);
            }
        }

        static public byte[] CreateDeviceID(DeviceAddress address)
        {
            // Copy byte by byte in order to fill ID buffer
            byte[] deviceId = new byte[SessionData.SESSION_ID_LEN];
            for (int offset = 0; offset < SessionData.SESSION_ID_LEN; ++offset)
            {
                Buffer.BlockCopy(address.Value, offset % address.Length, deviceId, offset, 1);
            }

            return deviceId;
        }

        static public byte[] BuildByte( byte[][] data)
        {
            // Determine length of return byte array
            int totalLength = 0;
            foreach (byte[] innerArray in data)
            {
                totalLength += innerArray.Length;
            }

            if (totalLength <= 0)
                throw new ArgumentOutOfRangeException();

            byte[] combinedArray = new byte[totalLength];
            int offset = 0;
            foreach (byte[] innerArray in data)
            {
                // Copy data from array
                Buffer.BlockCopy(innerArray, 0, combinedArray, offset, innerArray.Length);

                // Apply offset
                offset += innerArray.Length;
            }
            return combinedArray;
        }

        static public String GetTimestamp(DateTime value)
        {
            return value.ToString("G", CultureInfo.CreateSpecificCulture("en-AU")); // Stick to one format
        }

        // Range from 0x00 to 0xff
        static public int ConvertFromTwoComplement(byte val)
        {
            return Convert.ToInt32(val) - 127;
        }

        // Range from 0 to 128
        static public byte ConvertToTwoComplement(int val)
        {
            return Convert.ToByte(val + 127);
        }
    }
}
