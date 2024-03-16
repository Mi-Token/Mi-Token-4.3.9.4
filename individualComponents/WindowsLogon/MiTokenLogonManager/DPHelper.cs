using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Text;

namespace MiTokenWindowsLogon
{
    /*Based on http://msdn.microsoft.com/en-us/library/ff649246.aspx*/
    class DPHelper
    {
        /*http://msdn.microsoft.com/en-us/library/aa381414%28VS.85%29.aspx*/
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct DATA_BLOB
        {
            public int cbData;
            public IntPtr pbData;
        }

        /*http://msdn.microsoft.com/en-us/library/aa380263%28VS.85%29.aspx*/
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct CRYPTPROTECT_PROMPTSTRUCT
        {
            public int cbSize;
            public int dwPromptFlags;
            public IntPtr hwndApp;
            public string szPrompt;
        }

        /*http://msdn.microsoft.com/en-us/library/aa380261%28VS.85%29.aspx*/
        [DllImport("crypt32.dll",
               SetLastError = true,
               CharSet = System.Runtime.InteropServices.CharSet.Auto)]
        private static extern
            bool CryptProtectData(ref DATA_BLOB pDataIn,
                                        string szDataDescr,
                                    ref DATA_BLOB pOptionalEntropy,
                                        IntPtr pvReserved,
                                    ref CRYPTPROTECT_PROMPTSTRUCT pPromptStruct,
                                        int dwFlags,
                                    ref DATA_BLOB pDataOut);

        /*http://msdn.microsoft.com/en-us/library/aa380882%28VS.85%29.aspx*/
        [DllImport("crypt32.dll",
                    SetLastError = true,
                    CharSet = System.Runtime.InteropServices.CharSet.Auto)]
        private static extern
            bool CryptUnprotectData(ref DATA_BLOB pDataIn,
                                    ref string ppszDataDescr,
                                    ref DATA_BLOB pOptionalEntropy,
                                        IntPtr pvReserved,
                                    ref CRYPTPROTECT_PROMPTSTRUCT pPromptStruct,
                                        int dwFlags,
                                    ref DATA_BLOB pDataOut);

        private const int CRYPTPROTECT_UI_FORBIDDEN = 0x1;
        private const int CRYPTPROTECT_LOCAL_MACHINE = 0x4;
        private static IntPtr NullPtr = ((IntPtr)((int)(0)));
        public enum Store { USE_MACHINE_STORE = 1, USE_USER_STORE };


        public static string Encrypt(Store store, string plainText, string entropy, string description)
        {
            if (string.IsNullOrEmpty(plainText))
                plainText = String.Empty;

            if (string.IsNullOrEmpty(entropy))
                entropy = String.Empty;

            return Convert.ToBase64String(Encrypt(store, Encoding.UTF8.GetBytes(plainText), Encoding.UTF8.GetBytes(entropy), description));
        }

        public static byte[] Encrypt(Store store, byte[] plainTextBytes, byte[] entropyBytes, string description)
        {
            CRYPTPROTECT_PROMPTSTRUCT prompt = new CRYPTPROTECT_PROMPTSTRUCT();
            DATA_BLOB plainTextBlob = new DATA_BLOB();
            DATA_BLOB cipherTextBlob = new DATA_BLOB();
            DATA_BLOB entropyBlob = new DATA_BLOB();

            try
            {
                InitDataBLOB(plainTextBytes, ref plainTextBlob);
                InitDataBLOB(entropyBytes, ref entropyBlob);
                InitPromptStruct(ref prompt);

                int flags = CRYPTPROTECT_LOCAL_MACHINE;
               // int flags = CRYPTPROTECT_UI_FORBIDDEN;

                //if (store == Store.USE_MACHINE_STORE)
                //    flags |= CRYPTPROTECT_LOCAL_MACHINE;

                bool success = CryptProtectData(ref plainTextBlob,
                                                    description,
                                                ref entropyBlob,
                                                    IntPtr.Zero,
                                                ref prompt,
                                                    flags,
                                                ref cipherTextBlob);
                if (!success)
                    throw new Exception("Encryption failed.", new Win32Exception(Marshal.GetLastWin32Error()));

                byte[] cipherTextBytes = new byte[cipherTextBlob.cbData];
                Marshal.Copy(cipherTextBlob.pbData, cipherTextBytes, 0,cipherTextBlob.cbData);

                return cipherTextBytes;
            }
            catch (Exception ex)
            {
                throw new Exception("CryptProtectData was unable to encrypt data.", ex);
            }
            finally
            {
                if (plainTextBlob.pbData != IntPtr.Zero)
                    Marshal.FreeHGlobal(plainTextBlob.pbData);

                if (cipherTextBlob.pbData != IntPtr.Zero)
                    Marshal.FreeHGlobal(cipherTextBlob.pbData);

                if (entropyBlob.pbData != IntPtr.Zero)
                    Marshal.FreeHGlobal(entropyBlob.pbData);
            }
        }

        public static string Decrypt(string cipherText, string entropy, out string description)
        {
            if (string.IsNullOrEmpty(entropy)) 
                entropy = String.Empty;

            return Encoding.UTF8.GetString( Decrypt(Convert.FromBase64String(cipherText), Encoding.UTF8.GetBytes(entropy), out description));
        }

        public static byte[] Decrypt(byte[] cipherTextBytes, byte[] entropyBytes, out string description)
        {
            DATA_BLOB plainTextBlob = new DATA_BLOB();
            DATA_BLOB cipherTextBlob = new DATA_BLOB();
            DATA_BLOB entropyBlob = new DATA_BLOB();
            CRYPTPROTECT_PROMPTSTRUCT prompt = new CRYPTPROTECT_PROMPTSTRUCT();
           
            description = String.Empty;

            try
            {
               InitDataBLOB(cipherTextBytes, ref cipherTextBlob);
               InitDataBLOB(entropyBytes, ref entropyBlob);
               InitPromptStruct(ref prompt);

               int flags = CRYPTPROTECT_LOCAL_MACHINE;//CRYPTPROTECT_UI_FORBIDDEN;

               bool success = CryptUnprotectData(ref cipherTextBlob,
                                                  ref description,
                                                  ref entropyBlob,
                                                      IntPtr.Zero,
                                                  ref prompt,
                                                      flags,
                                                  ref plainTextBlob);

                if (!success)
                    throw new Exception("Decryption failed.", new Win32Exception(Marshal.GetLastWin32Error()));

                byte[] plainTextBytes = new byte[plainTextBlob.cbData];
                Marshal.Copy(plainTextBlob.pbData, plainTextBytes, 0, plainTextBlob.cbData);

                return plainTextBytes;
            }
            catch (Exception ex)
            {
                throw new Exception("CryptUnprotectData was unable to decrypt data.", ex);
            }
            finally
            {
                if (plainTextBlob.pbData != IntPtr.Zero)
                    Marshal.FreeHGlobal(plainTextBlob.pbData);

                if (cipherTextBlob.pbData != IntPtr.Zero)
                    Marshal.FreeHGlobal(cipherTextBlob.pbData);

                if (entropyBlob.pbData != IntPtr.Zero)
                    Marshal.FreeHGlobal(entropyBlob.pbData);
            }
        }

        private static void InitPromptStruct(ref CRYPTPROTECT_PROMPTSTRUCT ps)
        {
            ps.cbSize = Marshal.SizeOf(typeof(CRYPTPROTECT_PROMPTSTRUCT));
            ps.dwPromptFlags = 0;
            ps.hwndApp = NullPtr;
            ps.szPrompt = null;
        }

        private static void InitDataBLOB(byte[] data, ref DATA_BLOB blob)
        {
            if (data == null)
                data = new byte[0];

            try
            {
                blob.pbData = Marshal.AllocHGlobal(data.Length);

                if (blob.pbData == IntPtr.Zero)
                    throw new Exception("Unable to allocate data buffer for BLOB structure.");

                blob.cbData = data.Length;
                Marshal.Copy(data, 0, blob.pbData, data.Length);
                blob.cbData = data.Length;
            }
            catch (Exception ex)
            {
                throw new Exception("Exception marshalling data. " + ex.Message);
            }
        }
    }
}
