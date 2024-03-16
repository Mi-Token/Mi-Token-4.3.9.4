using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MiTokenBLEWatcher
{
    class PingBurstCfg
    {
        private static readonly int TOTAL_PING_MAX = 3;
        public UInt16 DurationMs = 0;
        public UInt16 IntervalMs = 0;
        private List<UInt16> FrequenciesHz;

        public PingBurstCfg()
        {
            FrequenciesHz = new List<UInt16>(TOTAL_PING_MAX);
        }

        public bool AddPing(UInt16 freq)
        {
            if (FrequenciesHz.Count < TOTAL_PING_MAX)
            {
                FrequenciesHz.Add(freq);
                return true;
            }
            return false;
        }

        public byte[] Serialize()
        {
            List<byte> freqBytes = new List<byte>();
            foreach (UInt16 freq in FrequenciesHz)
            {
                foreach (byte b in BitConverter.GetBytes(Convert.ToUInt16(freq)))
                {
                    freqBytes.Add(b);
                }
            }

            return BuildByte(new byte[][] {
                    new byte[] { (byte)FrequenciesHz.Count },
                    BitConverter.GetBytes(Convert.ToUInt16(DurationMs)),
                    BitConverter.GetBytes(Convert.ToUInt16(IntervalMs)),
                    freqBytes.ToArray()
                });
        }

        static private byte[] BuildByte(byte[][] data)
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
    }
}
