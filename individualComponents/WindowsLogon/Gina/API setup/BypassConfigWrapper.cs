using System;
using System.Collections.Generic;
using System.Text;
using System.Security.Principal;
using System.Security.Cryptography;

using Microsoft.Win32;
namespace API_setup
{
    //Interface for bypass to pull the hashes it has / set new ones
    //Allows both the API Autoconfig and API Setup to have the same code despite running in 2 different ways
    //Autoconfig : 1 layer hashes stored in lists
    //Setup : 2 layer hashes stored in the registry
    public interface IBypassConfig
    {
        byte[] getHashes(string username);
        int getHashCount(string username);
        bool hasHash(string username, byte[] hash);
        bool appendHash(string username, byte[] newHash);
        void removeAllUserHashes(string username);
        string[] getUsernames();
        byte[] computeHash(string bypass); //TODO
        bool removeAllHashes();
        void setBypassTime(string username, byte[] timeMask);
        byte[] getBypassTime(string username);
     }

    static class GenericBypassFunctions
    {
        public static readonly byte[] staticSalt = { 0xF7, 0x05, 0x8C, 0xE3, 0x30, 0x86, 0x2E, 0x07, 0x22, 0x3C, 0x61, 0x7A, 0xDC, 0xBE, 0xDC, 0x7D, 0x65, 0x1A, 0x25, 0x9F };
    
        public static bool hasHash(byte[] hashes, byte[] hash)
        {
            if (hashes == null)
                return false;
            for (int i = 21; i < hashes.Length; i += 20)
            {
                bool found = true;
                for (int j = 0; (j < 20) && (found) ; ++j)
                {
                    if (hashes[i + j] != hash[j])
                        found = false;
                }
                if (found)
                    return true;
            }
            return false;
        }
        public static byte[] appendHash(byte[] hashes, byte[] hash)
        {
            if (hashes == null)
                return addDefaultLimits(hash);

            byte[] d = new byte[hashes.Length + hash.Length];
            Array.Copy(hashes, d, hashes.Length);
            Array.Copy(hash, 0, d, hashes.Length, hash.Length);
            return d;
        }
        public static byte[] hash1(string bypass)
        {
            HMACSHA1 hmac = new HMACSHA1(staticSalt);
            hmac.ComputeHash(ASCIIEncoding.ASCII.GetBytes(bypass));
            return hmac.Hash;
        }
        public static byte[] hash2(byte[] data, byte[] salt)
        {
            HMACSHA1 hmac = new HMACSHA1(salt);
            hmac.ComputeHash(data);
            return hmac.Hash;
        }

        private static byte[] addDefaultLimits(byte[] hash)
        {
            byte[] b = new byte[hash.Length + 21];
            for (int i = 0; i < 21; ++i)
                b[i] = 0xFF;
            Array.Copy(hash, 0, b, 21, hash.Length);
            return b;
        }

        public static byte[] setTimeLimits(byte[] newLimits, byte[] oldHashes)
        {
            if (oldHashes == null)
                return newLimits;
            else
            {
                Array.Copy(newLimits, oldHashes, 21);
                return oldHashes;
            }
        }
    }

    class BypassFromRegistry : IBypassConfig
    {
        public BypassFromRegistry() : this(@"Software\Mi-Token\Logon\Bypass")
        {
        }

        protected bool usePCSalt; //should the hashes be double hashed or not. (double hashed as in Hash = SHA(SHA(value, static salt), pc salt); ]

        //accessable only by this object and children objects - allows you to override the regLocation
        protected BypassFromRegistry(string location)
        {
            registryLocation = location;
            usePCSalt = true;
            // Load existing bypasses
            RegistryKey rk = Registry.LocalMachine.CreateSubKey(registryLocation);

            if (rk != null)
            {
                //Check if the PC Salt exists, and if not generate it
                salt = (byte[])rk.GetValue("");
                if (salt == null)
                {
                    rk.Close();
                    rk = Registry.LocalMachine.OpenSubKey(registryLocation, true);
                    salt = new byte[20];
                    System.Security.Cryptography.RNGCryptoServiceProvider rng = new RNGCryptoServiceProvider();
                    rng.GetBytes(salt);
                    rk.SetValue("", salt);
                    rk.Close();
                    rk = Registry.LocalMachine.OpenSubKey(registryLocation);
                }
                string[] subkeys = rk.GetValueNames();
                foreach (string user in subkeys)
                {
                    if (user == "")
                        continue;
                    if (user == "serverhash") //ignore the serverhash key - that isn't a user
                        continue;
                    usernames.Add(user);
                }
            }
            rk.Close();
        }

        protected string registryLocation;

        protected List<string> usernames = new List<string>();

        protected byte[] salt;
        
        public string[] getUsernames()
        {
            return usernames.ToArray();
        }
        public byte[] getHashes(string username)
        {
            RegistryKey rk = Registry.LocalMachine.OpenSubKey(registryLocation, true);
            byte[] data = (byte[])rk.GetValue(username, null);
            rk.Close();
            return data;
        }
        public int getHashCount(string username)
        {
            byte[] hashes = getHashes(username);
            if(hashes == null)
                return 0;
            return (hashes.Length - 21) / 20;
        }
        public bool hasHash(string username, byte[] hash)
        {
            byte[] hashes = getHashes(username);
            return GenericBypassFunctions.hasHash(hashes, hash);
        }
        public bool appendHash(string username, byte[] newHash)
        {
            byte[] hashes = getHashes(username);
            bool hadHash = GenericBypassFunctions.hasHash(hashes, newHash);
            if (hadHash)
                return false;
            hashes = GenericBypassFunctions.appendHash(hashes, newHash);
            RegistryKey rk = Registry.LocalMachine.OpenSubKey(registryLocation, true);
            rk.SetValue(username, hashes);
            rk.Close();
            return true;
        }
        public void removeAllUserHashes(string username)
        {
            RegistryKey rk = Registry.LocalMachine.OpenSubKey(registryLocation, true);
            try
            {
                rk.DeleteValue(username);
            }
            catch
            {
                
            }
            finally
            {
                rk.Close();
            }
        }
        public byte[] computeHash(string bypass)
        {
            if (usePCSalt)
                return GenericBypassFunctions.hash2(GenericBypassFunctions.hash1(bypass), salt);
            else
                return GenericBypassFunctions.hash1(bypass);
        }

        public void setBypassTime(string username, byte[] timeMask)
        {
            byte[] hashes = getHashes(username);
            byte[] newHashes = GenericBypassFunctions.setTimeLimits(timeMask, hashes);
            RegistryKey rk = Registry.LocalMachine.OpenSubKey(registryLocation, true);
            rk.SetValue(username, newHashes);
            rk.Close();
            return;
        }

        public byte[] getBypassTime(string username)
        {
            byte[] hashes = getHashes(username);
            if (hashes.Length < 21)
                return null;
            byte[] r = new byte[21];
            Array.Copy(hashes, r, 21);
            return r;
        }

        public bool removeAllHashes() { return false; } //does not support Remove All Hashes
    }

    class BypassFromGlobalRegistry : BypassFromRegistry
    {
        //Override the Registry Location - everything else is fine as is.
        public BypassFromGlobalRegistry() : base(@"Software\Mi-Token\GlobalBypass")
        {
            usePCSalt = false;
        }
    }

    public class BypassFromMemory : IBypassConfig
    {
        Dictionary<string, byte[]> internalData = new Dictionary<string, byte[]>();
        public BypassFromMemory() { }
        public string[] getUsernames()
        {
            string[] keys = new string[internalData.Keys.Count];
            internalData.Keys.CopyTo(keys, 0);
            return keys;
        }
        public byte[] getHashes(string username)
        {
            byte[] ret;
            if (internalData.TryGetValue(username, out ret))
                return ret;
            return null;
        }
        public int getHashCount(string username)
        {
            return (getHashes(username).Length - 21) / 20;
        }
        public bool hasHash(string username, byte[] hash)
        {
            byte[] hashes = getHashes(username);
            return GenericBypassFunctions.hasHash(hashes, hash);
        }
        public bool appendHash(string username, byte[] newHash)
        {
            byte[] hashes = getHashes(username);
            if (GenericBypassFunctions.hasHash(hashes, newHash))
                return false;
            hashes = GenericBypassFunctions.appendHash(hashes, newHash);
            internalData[username] = hashes;
            return true;
        }
        public void removeAllUserHashes(string username)
        {
            internalData.Remove(username);
        }
        public byte[] computeHash(string bypass)
        {
            return GenericBypassFunctions.hash1(bypass);
        }
        public void setBypassTime(string username, byte[] timeMask)
        {
            byte[] hashes = getHashes(username);
            byte[] newHashes = GenericBypassFunctions.setTimeLimits(timeMask, hashes);
            internalData[username] = hashes;
        }
        public byte[] getBypassTime(string username)
        {
            byte[] hashes = getHashes(username);
            if (hashes.Length < 21)
                return null;
            byte[] r = new byte[21];
            Array.Copy(hashes, r, 21);
            return r;
        }
        public bool removeAllHashes() { internalData = new Dictionary<string, byte[]>(); return true; }
    }
}
