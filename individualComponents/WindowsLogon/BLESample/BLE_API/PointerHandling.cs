using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace BLE_API
{
    public partial class BLE
    {
        internal static class PointerHandling
        {
            internal static IntPtr allocatePtrForString(string str)
            {
                byte[] bdata = Encoding.ASCII.GetBytes(str);

                byte[] tdata = new byte[bdata.Length + 1];
                Array.Copy(bdata, tdata, bdata.Length);
                tdata[tdata.Length - 1] = 0;
                return allocatePtrForData(tdata);
            }

            internal static IntPtr allocatePtrForData(byte[] data)
            {
                IntPtr ip = Marshal.AllocHGlobal(data.Length);
                try
                {
                    Marshal.Copy(data, 0, ip, data.Length);
                }
                catch (Exception ex)
                {
                    Marshal.FreeHGlobal(ip);
                    throw ex;
                }

                return ip;
            }

            internal static IntPtr allocateForSize(int size)
            {
                return Marshal.AllocHGlobal(size);
            }

            internal static IntPtr callocateForSize(int size)
            {
                IntPtr ptr = Marshal.AllocHGlobal(size);
                //AllocHGlobal does NOT zero fill the data, we need to do that, so create a new buffer of the size, and set it to all zeros.
                byte[] b = new byte[size];
                for (int i = 0; i < size; ++i)
                {
                    b[i] = 0;
                }
                Marshal.Copy(b, 0, ptr, size);

                return ptr;
            }

            internal static void setFromPtr(IntPtr source, byte[] data)
            {
                Marshal.Copy(source, data, 0, data.Length);
            }

            internal static IntPtr offsetPointer(IntPtr ptr, int offset)
            {
                if (IntPtr.Size == 8)
                {
                    Int64 i64 = (Int64)ptr;
                    i64 += offset;
                    return (IntPtr)i64;
                }
                else if (IntPtr.Size == 4)
                {
                    Int32 i32 = (Int32)ptr;
                    i32 += offset;
                    return (IntPtr)i32;
                }

                return IntPtr.Zero;
            }

            internal static void deallocatePtr(IntPtr ptr)
            {
                if (ptr != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(ptr);
                }
            }

            internal static IntPtr functionPointerFromDelegate(Delegate function)
            {
                return Marshal.GetFunctionPointerForDelegate(function);
            }

            internal static void writeCallbackToChainPtr(IntPtr chainPointer, int offset, int index, CommandChain.CommandChainCall valueToWrite)
            {
                int realOffset = (offset + (IntPtr.Size * index));
                IntPtr mptr = PointerHandling.offsetPointer(chainPointer, realOffset);
                IntPtr ipF = IntPtr.Zero;
                if (valueToWrite != null)
                {
                    ipF = PointerHandling.functionPointerFromDelegate(valueToWrite);
                }

                byte[] bd = null;
                if (IntPtr.Size == 8)
                {
                    Int64 i64 = (Int64)ipF;
                    bd = BitConverter.GetBytes(i64);
                }
                else if (IntPtr.Size == 4)
                {
                    Int32 i32 = (Int32)ipF;
                    bd = BitConverter.GetBytes(i32);
                }

                Marshal.Copy(bd, 0, mptr, bd.Length);
            }
        }
    }
}
