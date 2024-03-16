using System;
using System.Collections;
using System.Collections.Generic;
using Microsoft.Win32;

namespace MiTokenWindowsLogon
{
    public struct RadiusServer : IComparer

    {
        public string IPAddressKey { get; set; }
		public string ServerPortKey{get; set;}
		public string ServerSharedSecretKey{get; set;}
        public string IPAddress { get; set; }
		public int ServerPort{get; set;}
		public string ServerSharedSecret{get; set;}
        public string DisplayOrder { get; set; }

        int IComparer.Compare(object a, object b)
        {
            return (((RadiusServer)a).DisplayOrder.CompareTo(((RadiusServer)b).DisplayOrder));
        }

    }

    class RegistryHelper
    {
         private const string MITOKEN_GINA_SERVER_VALUE_NAME = "Server_Name_";
         private const string MITOKEN_GINA_RADIUS_PORT_VALUE_NAME = "Server_Port_";
         private const string MITOKEN_GINA_SHARED_SECRET_VALUE_NAME = "Server_Secret_";

         public static List<RadiusServer> RetrieveRadiusServers(string keyPath)
         {
             List<RadiusServer> servers = new List<RadiusServer>();
             RadiusServer server;
             string serverId;

             using ( RegistryKey key = Registry.LocalMachine.CreateSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
             {
                 if (key != null)
                 {
                     foreach (string valueName in key.GetValueNames())
                     {
                         try
                         {
                             if(valueName.Contains(MITOKEN_GINA_SERVER_VALUE_NAME))
                             {
                                 server = new RadiusServer();
                                 server.IPAddressKey = valueName;
                                 server.IPAddress = RetrieveSingleStringValue(keyPath, valueName);

                                 server.DisplayOrder = valueName.Replace(MITOKEN_GINA_SERVER_VALUE_NAME, string.Empty);

                                 server.ServerPortKey = MITOKEN_GINA_RADIUS_PORT_VALUE_NAME + server.DisplayOrder;
                                 server.ServerPort = RetrieveSingleInt32Value(keyPath, server.ServerPortKey);

                                 server.ServerSharedSecretKey = MITOKEN_GINA_SHARED_SECRET_VALUE_NAME + server.DisplayOrder;
                                 server.ServerSharedSecret = RetrieveSingleStringValue(keyPath, server.ServerSharedSecretKey);

                                 servers.Add(server);
                             }
                         }
                         catch
                         {
                            //Ignore all errors. Go to the next radius server entry.
                         }
                     }
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }

             
             return servers;
         }

         private static string RetrieveSingleStringValue(string keyPath, string valueName)
         {
             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadSubTree))
             {
                 if (key != null)
                 {
                     object value = key.GetValue(valueName);

                     if (value == null)
                         throw new RegistryHelperException("Unable to find value name " + valueName);
                     
                     return value.ToString();
                 }
                 else
                    throw new RegistryHelperException("Unable to find key " + keyPath);
             }
         }

         private static int RetrieveSingleInt32Value(string keyPath, string valueName)
         {
             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadSubTree))
             {
                 if (key != null)
                 {
                     object value = key.GetValue(valueName);

                     if (value == null)
                         throw new RegistryHelperException("Unable to find value name " + valueName);
                     else
                     {
                        int result;
                        if(!int.TryParse(value.ToString(), out result))
                            throw new RegistryHelperException("Invalid int value for value name " + valueName);

                        return result;
                     }
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }
         }


         private static byte[] RetrieveBinaryValue(string keyPath, string valueName)
         {
             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadSubTree))
             {
                 if (key != null)
                 {
                     object value = key.GetValue(valueName);

                     if (value == null)
                         throw new RegistryHelperException("Unable to find value name " + valueName);
                     else
                         return (byte[])value;
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }
         }

         private static string GetNewServerId(string keyPath)
         {
             string serverIdString;
             int oldServerId = 0;
             int currentServerId = 0;

             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadSubTree))
             {
                 if (key != null)
                 {
                     foreach (string valueName in key.GetValueNames())
                     {
                         try
                         {
                             if (valueName.Contains(MITOKEN_GINA_SERVER_VALUE_NAME))
                             {
                                 serverIdString = valueName.Replace(MITOKEN_GINA_SERVER_VALUE_NAME, string.Empty);
                                 if (int.TryParse(serverIdString, out currentServerId))
                                 {
                                     if (currentServerId > oldServerId)
                                         oldServerId = currentServerId;
                                 }
                             }
                         }
                         catch
                         {
                             //Ignore all errors. Go to the next radius server entry.
                         }
                     }
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }

             ++oldServerId;
             return oldServerId.ToString();
         }

         private static void WriteSingleInt32Value(string keyPath, string valueName, int value)
         {
             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
             {
                 if (key != null)
                 {
                     key.SetValue(valueName, value, RegistryValueKind.DWord);
                     key.Flush();
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }
         }

         private static void WriteSingleStringValue(string keyPath, string valueName, string value)
         {
             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
             {
                 if (key != null)
                 {
                     key.SetValue(valueName, value, RegistryValueKind.String);
                     key.Flush();
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }
         }


         private static void WriteBinaryValue(string keyPath, string valueName, byte[] value)
         {
             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
             {
                 if (key != null)
                 {
                     key.SetValue(valueName, value, RegistryValueKind.Binary);
                     key.Flush();
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }
         }

         private static void DeleteValue(string keyPath, string valueName)
         {
             using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyPath, RegistryKeyPermissionCheck.ReadWriteSubTree))
             {
                 if (key != null)
                 {
                     key.DeleteValue(valueName);
                     key.Flush();
                 }
                 else
                     throw new RegistryHelperException("Unable to find key " + keyPath);
             }
         }

         public static void WriteRadiusServer(string keyPath, ref RadiusServer value)
         {             
             if (string.IsNullOrEmpty(value.IPAddressKey) && string.IsNullOrEmpty(value.ServerSharedSecretKey) && string.IsNullOrEmpty(value.ServerPortKey))
                 value.DisplayOrder = GetNewServerId(keyPath);

             value.IPAddressKey = string.IsNullOrEmpty(value.IPAddressKey) ? MITOKEN_GINA_SERVER_VALUE_NAME + value.DisplayOrder : value.IPAddressKey;
             value.ServerSharedSecretKey = string.IsNullOrEmpty(value.ServerSharedSecretKey) ? MITOKEN_GINA_SHARED_SECRET_VALUE_NAME + value.DisplayOrder : value.ServerSharedSecretKey;
             value.ServerPortKey = string.IsNullOrEmpty(value.ServerPortKey) ? MITOKEN_GINA_RADIUS_PORT_VALUE_NAME + value.DisplayOrder : value.ServerPortKey;


             value.ServerSharedSecret = DPHelper.Encrypt(DPHelper.Store.USE_MACHINE_STORE, value.ServerSharedSecret, string.Empty, "Shared secret for RADIUS server.");

             WriteSingleStringValue(keyPath, value.IPAddressKey, value.IPAddress);
           //  WriteBinaryValue(keyPath, value.ServerSharedSecretKey + "whop", Convert.FromBase64String(value.ServerSharedSecret));

             WriteSingleStringValue(keyPath, value.ServerSharedSecretKey, value.ServerSharedSecret);
             WriteSingleStringValue(keyPath, value.ServerPortKey, value.ServerPort.ToString());

             //WriteSingleInt32Value(keyPath, value.ServerPortKey, value.ServerPort);
         }

         public static void DeleteRadiusServer(string keyPath, RadiusServer value)
         {
             DeleteValue(keyPath, value.IPAddressKey);
             DeleteValue(keyPath, value.ServerPortKey);
             DeleteValue(keyPath, value.ServerSharedSecretKey);
          //   DeleteValue(keyPath, value.ServerSharedSecretKey + "whop");
         }

    }

    public class RegistryHelperException : Exception
    {
        public RegistryHelperException(string reason) : base(reason) { }
        public RegistryHelperException(string reason, Exception inner) : base(reason, inner) { }
    }
}
