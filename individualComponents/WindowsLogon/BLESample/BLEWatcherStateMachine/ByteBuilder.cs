using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLEWatcherStateMachine
{
    class ByteBuilder
    {
        private List<byte[]> _vals;

        public ByteBuilder()
        {
            _vals = new List<byte[]>();
        }

        public void Add(byte[] input)
        {
            _vals.Add(input);
        }

        public void Add(byte input)
        {
            _vals.Add(new byte[] { input });
        }

        public int Length
        {
            get
            {
                int val = 0;
                foreach (byte[] b in _vals)
                {
                    val += b.Length;
                }
                return val;
            }
        }

        public byte[] ToArray
        {
            get
            {
                int len = Length;
                byte[] output = new byte[len];
                int offset = 0;
                foreach (byte[] b in _vals)
                {
                    Array.Copy(b, 0, output, offset, b.Length);
                    offset += b.Length;
                }

                return output;
            }
        }
    }
}
