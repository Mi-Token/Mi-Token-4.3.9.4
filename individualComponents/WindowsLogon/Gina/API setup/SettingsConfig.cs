using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using Microsoft.Win32;
using System.Security.Cryptography;
using System.Security.Principal;


namespace API_setup
{
    static class PersistSettings
    {
        internal class Settings
        {
            public bool debugLogging;
            public int filterXPLocal;
            public int filterXPRDP;
            public int filterCPLocal;
            public int filterCPRemote;
            public int useDynamicPasswords;
            public int useHTTP;
            public int useCredInSafeMode;
            public string[] servers;
            public class Bypass
            {
                public string userSID;
                public string[] bypassCodes;
            }
            public Bypass[] bypass;
            public int hideLastUsername;
        }

        internal static string readXMLFromStream(StreamReader stream, out Settings settings)
        {
            settings = new Settings();
            StreamReader SR = stream;
            string s = SR.ReadToEnd();
            int pos = 0;
            if (s[pos] == 'V')
            {
                //version header
                pos++;
                if (s[pos] == 0)
                {
                    //version 2
                    pos++;
                    settings.debugLogging = (s[pos] == '1' ? true : false);
                    settings.filterXPLocal = s[pos + 1];
                    settings.filterXPRDP = s[pos + 2];
                    settings.filterCPLocal = s[pos + 3];
                    settings.filterCPRemote = s[pos + 4];
                    pos += 5;
                }
                else if (s[pos] == 1)
                {
                    //version 2.1
                    pos++;
                    settings.debugLogging = (s[pos] == '1' ? true : false);
                    settings.filterXPLocal = s[pos + 1];
                    settings.filterXPRDP = s[pos + 2];
                    settings.filterCPLocal = s[pos + 3];
                    settings.filterCPRemote = s[pos + 4];
                    settings.useDynamicPasswords = s[pos + 5];
                    pos += 6;
                }
                else if (s[pos] == 2)
                {
                   pos++;
                   settings.debugLogging = (s[pos] == '1' ? true : false);
                   settings.filterXPLocal = s[pos + 1];
                   settings.filterXPRDP = s[pos + 2];
                   settings.filterCPLocal = s[pos + 3];
                   settings.filterCPRemote = s[pos + 4];
                   settings.useDynamicPasswords = s[pos + 5];
                   settings.useHTTP = s[pos + 6];
                   settings.useCredInSafeMode = s[pos + 7];
                   settings.hideLastUsername = s[pos + 8];
                   pos += 9;
                }
            }
            else
            {
                settings.debugLogging = (s[pos] == '1' ? true : false);
                pos++;
            }
            List<string> servers = new List<string>();
            List<Settings.Bypass> bypasses = new List<Settings.Bypass>();
            List<string> bypassOTPs = new List<string>();
            while (s[pos] != '|')
            {
                int end = s.IndexOf('|', pos);
                servers.Add(s.Substring(pos, end - pos));
                pos = end;
                if (s[pos] != '|')
                {
                    return "Error - Invalid File Format (0)";
                }
                pos++;
            }
            ++pos;
            while ((pos < s.Length) && (s[pos] == '{'))
            {
                pos++;
                int end = s.IndexOf(',', pos);
                string userSID = s.Substring(pos, end - pos);
                Settings.Bypass b = new Settings.Bypass();
                b.userSID = userSID;
                pos = end + 1;
                while (s[pos] == '[')
                {
                    pos++;
                    end = s.IndexOf(']', pos);
                    bypassOTPs.Add(s.Substring(pos, end - pos));
                    pos = end + 1;
                    if (s[pos] != '|') 
                    {
                        //error
                        return "Error - Invalid File Format (2)";
                    }
                    pos++;
                }
                if ((s[pos] != '|') || (s[pos + 1] != '}'))
                {
                    //error
                    return "Error - Invalid File Format (3)";
                }
                b.bypassCodes = bypassOTPs.ToArray();
                bypasses.Add(b);
                bypassOTPs = new List<string>();
                
                pos += 2;

            }
            if (pos != s.Length)
            {
                //error
                return "Error - Expected EOF (4)";
            }

            //worked
            settings.bypass = bypasses.ToArray();
            settings.servers = servers.ToArray();
            
            return null;
        }

        internal static void writeXMLToStream(StreamWriter stream, Settings settings)
        {
            StreamWriter SW = stream;
            //version 2 header V | version ID
            SW.Write("V");
            SW.Write((char)2);
            SW.Write((char)(settings.debugLogging ? 1 : 0));
            SW.Write((char)(settings.filterXPLocal));
            SW.Write((char)(settings.filterXPRDP));
            SW.Write((char)(settings.filterCPLocal));
            SW.Write((char)(settings.filterCPRemote));
            SW.Write((char)(settings.useDynamicPasswords));
            SW.Write((char)(settings.useHTTP));
            SW.Write((char)(settings.useCredInSafeMode));
            SW.Write((char)(settings.hideLastUsername));
            foreach (string s in settings.servers)
            {
                SW.Write("{0}|", s);
            }
            SW.Write("|");
            foreach (var user in settings.bypass)
            {
                SW.Write("{{{0},", user.userSID);
                foreach (var bypass in user.bypassCodes)
                {
                    SW.Write("[{0}]|", bypass);
                }
                SW.Write("|}");
            }
        }

        private static byte[] getHash(byte[] tempHash, byte[] salt)
        {
            HMACSHA1 hsha1 = new HMACSHA1(salt);
            hsha1.ComputeHash(tempHash);
            return hsha1.Hash;
        }

        private static bool containsHash(byte[] hashList, byte[] hash)
        {
            for (int i = 0; i < hashList.Length; i += 20)
            {
                bool found = true;
                for (int j = 0; (j < 20) && (!found); ++j)
                {
                    if (hashList[i + j] != hash[j])
                        found = false;
                }
                if (found)
                    return true;
            }
            return false;
        }

        private static byte[] appendHash(byte[] hash1, byte[] hash2)
        {
            if(containsHash(hash1, hash2))
                return hash1;

            byte[] d = new byte[hash1.Length + hash2.Length];
            Array.Copy(hash1, d, hash1.Length);
            Array.Copy(hash2, 0, d, hash1.Length, hash2.Length);
            return d;
        }

        public struct Errors
        {
            public string Section;
            public string Data;
            public string Message;
        }

        internal static void persistXMLtoRegistry(string filePath, out List<Errors> errors)
        {
            errors = new List<Errors>();

            bool bReadXMLfinished = false;
            PersistSettings.Settings userSettings;

            try
            {
                using (System.IO.StreamReader sr = new System.IO.StreamReader(filePath))
                {
                    PersistSettings.readXMLFromStream(sr, out userSettings);
                }

                bReadXMLfinished = true;
                PersistSettings.persist(userSettings, out errors);
            }
            catch (Exception ex)
            {
                errors.Add(new Errors
                            {
                                Section = bReadXMLfinished? "XML file reading": "XML file persisting",
                                Data = ex.Message,
                                Message = "Exception: " + ex.GetType().ToString()
                            }
                          );
            }
        }

        private static void persist(Settings settings, out List<Errors> errors)
        {
            errors = new List<Errors>();
            //write all the settings to the registry
            RegistryKey API = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token", RegistryKeyPermissionCheck.ReadWriteSubTree);
            foreach (string server in settings.servers)
            {
                bool valid = true;
                foreach (string value in API.GetValueNames())
                {
                    if ((API.GetValueKind(value) == RegistryValueKind.String) &&
                        ((string)API.GetValue(value) == server))
                    {
                        //already exists
                        valid = false;
                        break;
                    }
                }
                if (valid)
                {
                    System.Net.IPAddress validIP;
                    if (!System.Net.IPAddress.TryParse(server, out validIP))
                    {
                        //Not an IP Address, check hostname
                        try
                        {
                            System.Net.Dns.GetHostAddresses(server);
                        }
                        catch
                        {
                            valid = false;
                            errors.Add(new Errors
                                            {
                                                Section = "API Server",
                                                Data = server,
                                                Message = "Could not resolve to an IP Address"
                                            });
                        }
                    }

                    if(valid)
                    {
                        int count = API.GetValueNames().Length;
                        while (API.GetValue("Server" + (count + 1), null) != null)
                        {
                            //Server[n] exists as a value. Increment count
                            count++;
                        }
                        API.SetValue("Server" + (count + 1), server);
                    }
                }
            }
            API.Close();

            RegistryKey Bypass = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token\Bypass", RegistryKeyPermissionCheck.ReadWriteSubTree);
            if (Bypass != null)
            {
                byte[] salt = (byte[])Bypass.GetValue("");
                if (salt == null)
                {
                    salt = new byte[20];
                    System.Security.Cryptography.RNGCryptoServiceProvider rng = new System.Security.Cryptography.RNGCryptoServiceProvider();
                    rng.GetBytes(salt);
                    Bypass.SetValue("", salt);
                }
                foreach (Settings.Bypass bypass in settings.bypass)
                {
                    //Check user SID
                    try
                    {
                        if (bypass.userSID != "*")
                        {
                            SecurityIdentifier SID = new SecurityIdentifier(bypass.userSID);
                            NTAccount NTA = (NTAccount)SID.Translate(typeof(NTAccount));
                        }
                    }
                    catch
                    {
                        errors.Add(new Errors
                                    {
                                        Section = "User Bypass",
                                        Data = bypass.userSID,
                                        Message = "Supplied SID did not resolve to a User / Group"
                                    });
                        continue;
                    }
                    byte[] hashes = new byte[0];
                    foreach (string pass in bypass.bypassCodes)
                    {
                        byte[] hash;
                        try
                        {
                            hash = Convert.FromBase64String(pass);
                        }
                        catch
                        {
                            errors.Add(new Errors
                                    {
                                        Section = "User Bypass",
                                        Data = pass,
                                        Message = "Supplied Bypass is not a valid base64 string"
                                    });
                            continue;
                        }
                        if (hash.Length != 20)
                        {
                            errors.Add(new Errors
                                        {
                                            Section = "User Bypass",
                                            Data = pass,
                                            Message = "Supplied Bypass is not a valid hash"
                                        });
                        }
                        else
                        {
                            hashes = appendHash(hashes, getHash(hash, salt));
                        }
                    }
                    Bypass.SetValue(bypass.userSID, hashes);
                }
            }
            Bypass.Close();
            RegistryKey DebugLogging = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token");
            DebugLogging.SetValue("CPDebugMode", settings.debugLogging ? 1 : 0);
        }
    }
}
