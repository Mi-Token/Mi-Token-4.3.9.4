using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;


namespace BLE_API
{
    public partial class BLE
    {
        public partial class Core
        {
            public class Debug
            {
                public delegate void DebugMessage(BLE.V2_2.MiTokenBLE sender, string message);
                public static event DebugMessage DebugMessageCallback;

                internal static void _debugMessageCallback(IntPtr handle, string message)
                {
                    if (DebugMessageCallback != null)
                    {
                        BLE.V2_2.MiTokenBLE sender = Core.LookupOnIntPtrV2_2(handle);
                        DebugMessageCallback(sender, message);
                    }
                }

                public static bool RegisterForDebugMessages(BLE.V2_2.MiTokenBLE instance, out string errorMessage)
                {
                    errorMessage = "Successful Registration";
                    try
                    {
                        //Attach to the event handler
                        Assembly assembly = Assembly.LoadFrom("BLE_API_Debugger.dll");
                        Type type = assembly.GetType("BLE_API_Debugger.Debugger");
                        object inst = Activator.CreateInstance(type);
                        EventInfo einfo = type.GetEvent("Debugger_SendDebugMessageCallback");
                        MethodInfo handler = typeof(Debug).GetMethod("_debugMessageCallback", BindingFlags.NonPublic | BindingFlags.Static);
                        Delegate del = Delegate.CreateDelegate(einfo.EventHandlerType, null, handler);
                        einfo.AddEventHandler(inst, del);

                        //Create the debug command chain and attach it
                        MethodInfo minfo = type.GetMethod("AttachDebugger");
                        minfo.Invoke(inst, new object[] { instance });
                    }
                    catch (Exception ex)
                    {
                        errorMessage = ex.Message;
                        return false;
                    }

                    return true;
                }
            }
        }
    }
}
