using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;


namespace BLEWatcherStateMachine
{
    public class PasswordHandler
    {
        internal Dictionary<string, byte[]> internalPasswordDB = new Dictionary<string, byte[]>();

        internal const int IV_SIZE = 16;

        public PasswordHandler()
        { 
            volatileKey = new byte[32];
            crng = new RNGCryptoServiceProvider();
            crng.GetBytes(volatileKey);
        }

        internal byte[] volatileKey;
        public RNGCryptoServiceProvider crng;

        internal byte[] encryptPassword(string password)
        {
            byte[] IV = new byte[IV_SIZE];
            crng.GetBytes(IV);
            byte[] plainTextBytes = Encoding.UTF8.GetBytes(password);
            byte[] encryptedData;
            using (AesManaged aes = new AesManaged())
            {
                aes.Mode = CipherMode.CBC;
                using (ICryptoTransform encryptor = aes.CreateEncryptor(volatileKey, IV))
                {
                    using (System.IO.MemoryStream ms = new System.IO.MemoryStream())
                    {
                        using (CryptoStream cs = new CryptoStream(ms, encryptor, CryptoStreamMode.Write))
                        {
                            cs.Write(plainTextBytes, 0, plainTextBytes.Length);
                            cs.FlushFinalBlock();
                            encryptedData = ms.ToArray();
                        }
                    }
                }
                aes.Clear();
            }

            byte[] ret = new byte[IV.Length + encryptedData.Length];
            Array.Copy(IV, 0, ret, 0, IV.Length);
            Array.Copy(encryptedData, 0, ret, IV.Length, encryptedData.Length);

            return ret;
        }

        internal string decryptPassword(byte[] encPassword)
        {
            byte[] IV = new byte[IV_SIZE];
            Array.Copy(encPassword, 0, IV, 0, IV.Length);
            byte[] encText = new byte[encPassword.Length - IV.Length];
            Array.Copy(encPassword, IV.Length, encText, 0, encText.Length);
            byte[] plainBytes = null;
            string ret = "";
            using (AesManaged aes = new AesManaged())
            {
                aes.Mode = CipherMode.CBC;
                using (ICryptoTransform decryptor = aes.CreateDecryptor(volatileKey, IV))
                {
                    using (System.IO.MemoryStream ms = new System.IO.MemoryStream(encText))
                    {
                        using (CryptoStream cs = new CryptoStream(ms, decryptor, CryptoStreamMode.Read))
                        {
                            plainBytes = new byte[encText.Length];
                            int pbc = cs.Read(plainBytes, 0, plainBytes.Length);
                            ret = Encoding.UTF8.GetString(plainBytes, 0, pbc);
                        }
                    }
                }
            }
            return ret;
        }

        public bool getPasswordForUsername(string username, out string password)
        {
            password = "";
            if (internalPasswordDB.ContainsKey(username))
            {
                byte[] encPass = internalPasswordDB[username];
                password = decryptPassword(encPass);
                return true;
            }
            return false;
        }

        public bool setPasswordForUsername(string username, string password)
        {
            byte[] encPass = encryptPassword(password);
            if (internalPasswordDB.ContainsKey(username))
            {
                internalPasswordDB[username] = encPass;
            }
            else
            {
                internalPasswordDB.Add(username, encPass);
            }
            return true;
        }

        public bool resetPasswordForUsername(string username)
        {
            if(internalPasswordDB.ContainsKey(username))
            {
                internalPasswordDB.Remove(username);
            }

            return true;
        }
    }
}
