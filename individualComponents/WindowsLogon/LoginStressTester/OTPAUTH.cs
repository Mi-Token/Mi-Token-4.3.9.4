using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Security.Cryptography;

namespace LoginStressTester
{
    class OTPAUTH
    {

        static HMACSHA1 Generator(byte[] key)
        {
            return new HMACSHA1(key);
        }


        public static string GenOTP(UInt32 counter, byte[] key)
        {
            
            byte[] counterBytes = new byte[] { 0, 0, 0, 0, 0, 0, 0, 0 };
            counterBytes[7] = (byte)(counter & 255);
            counter >>= 8;
            counterBytes[6] = (byte)(counter & 255);
            counter >>= 8;
            counterBytes[5] = (byte)(counter & 255);
            counter >>= 8;
            counterBytes[4] = (byte)(counter & 255);

            if (key == null)
                return "No Key";

            byte[] result = Generator(key).ComputeHash(counterBytes);

            int offset = result[19] & 0xF;

            int iresult = (result[offset] & 0x7F);
            iresult <<= 8;
            iresult += result[offset + 1];
            iresult <<= 8;
            iresult += result[offset + 2];
            iresult <<= 8;
            iresult += result[offset + 3];
            string tempRes = (iresult % 1000000).ToString();
            while (tempRes.Length != 6)
                tempRes = "0" + tempRes;
            return tempRes;
        }
    }
}
