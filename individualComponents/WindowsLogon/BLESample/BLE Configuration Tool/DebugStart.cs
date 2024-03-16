using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;

using BLE_API;
using System.Reflection;


namespace BLE_Configuration_Tool
{
    public partial class DebugStart : Form
    {
        public DebugStart()
        {
            InitializeComponent();
        }

        BLE_Configuration config = null;

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                if (config == null)
                {
                    config = new BLE_Configuration();
                    config.ShowDialog();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Exception raised : \r\n" + ex.Message + "\r\nStack : \r\n" + ex.StackTrace);
            }
            finally
            {
                config = null;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                MessageBox.Show(string.Format("DLL Version is : {0:X}", BLE.Core.DLL_Version));
            }
            catch (Exception ex)
            {
                MessageBox.Show("Exception raised : \r\n" + ex.Message + "\r\nStack : \r\n" + ex.StackTrace);
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            Thread T = new Thread(new ThreadStart(() =>
                {

                    try
                    {
                        BLE.V2_2.MiTokenBLE BLE = new BLE_API.BLE.V2_2.MiTokenBLE();
                        BLE.Initialize("COM11", "debugppipe", true);
                        
                        Thread.Sleep(20000);
                        int DevCount = BLE.GetDeviceFoundCount();
                        int bestDeviceID = 0;
                        int bestRSSI = 0;

                        BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();

                        for (int i = 0; i < DevCount; ++i)
                        {
                            BLE.GetDeviceInfo(i, ref devInfo);
                            if (i == 0)
                            {
                                bestRSSI = devInfo.RSSI;
                            }
                            else
                            {
                                if (devInfo.RSSI > bestRSSI)
                                {
                                    bestDeviceID = i;
                                    bestRSSI = devInfo.RSSI;
                                }
                            }
                        }

                        BLE.GetDeviceInfo(bestDeviceID, ref devInfo);

                        MessageBox.Show("Best Device was found with mac of " + devInfo.macString());

                        BLE.V2_2.Device dev = BLE.connectToDevice(devInfo);
                        dev.SyncWaitForConnection(-1);
                        dev.SyncScanServicesInRange(MiToken_BLE_UUIDs.BLE_UUID_SERVICE_SECURE_BOND, 0xFF, -1);
                        byte[] cmdBuffer = new byte[17];
                        cmdBuffer[0] = 0x01;
                        cmdBuffer[1] = 0xFF;
                        dev.SyncSetAttribute(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_CONFIG, cmdBuffer, -1);
                        List<byte[]> states = new List<byte[]>();
                        states.Add(new byte[] { 0x01 });
                        states.Add(new byte[] { 0x02 });
                        dev.SyncWaitForState(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, false, states, -1);
                        byte[] read;
                        dev.SyncReadAttribute(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_PUBLIC_KEY, true, out read, -1);

                        MessageBox.Show("Done");

                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.Message);
                    }
                    
                }));
            T.Start();


        }


        static BLE.CommandChain.CommandChainCall ccc_attribute_value = (IntPtr caller, IntPtr data) =>
            {
                BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_VALUE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_VALUE(data);
                return;
            };

        static BLE.CommandChain.CommandChainCall ccc_scan_response = (IntPtr caller, IntPtr data) =>
            {
                BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE(data);
                Console.WriteLine("Got scan response!");
                return;
            };

        private void loadAndBindDebuggerDLL(BLE.V2_2.MiTokenBLE ble)
        {
            try
            {
                Assembly assembly = Assembly.LoadFrom("BLE_API_Debugger.dll");


                Type type = assembly.GetType("BLE_API_Debugger.Debugger");
                object instance = Activator.CreateInstance(type);
                EventInfo[] events = type.GetEvents();

                MethodInfo handler = typeof(DebugStart).GetMethod("GotDebugMessage");
                foreach (EventInfo eventInfo in events)
                {
                    if (eventInfo.Name == "Debugger_SendDebugMessageCallback")
                    {
                        Delegate del = Delegate.CreateDelegate(eventInfo.EventHandlerType, null, handler);
                        eventInfo.AddEventHandler(instance, del);
                    }
                }


                
                MethodInfo methodInfo = type.GetMethod("AttachDebugger");
                methodInfo.Invoke(instance, new object[] {ble});

                
            }
            catch (Exception ex)
            {
                Invoke((MethodInvoker)delegate
                {
                    MessageBox.Show(ex.Message);
                });
 
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            Thread T = new Thread(new ThreadStart(() =>
                {

                    BLE.V2_2.MiTokenBLE ble = new BLE.V2_2.MiTokenBLE();
                    bleDictionary.Add(ble, this);

                    BLE.Core.Debug.DebugMessageCallback += GDM2;
                    string errorCode;
                    if (!BLE.Core.Debug.RegisterForDebugMessages(ble, out errorCode))
                    {
                        Invoke((MethodInvoker)delegate
                        {
                            listBox1.Items.Add("Failed to attach debugger");
                            listBox1.Items.Add("\t" + errorCode);
                        });
                    }

                    ble.Initialize("COM11", "debugpipe", true);
                    BLE.CommandChain cc = new BLE.CommandChain();
                    cc.evt.attclient.attribute_value = ccc_attribute_value;
                    cc.evt.gap.scan_response = ccc_scan_response;

                    ble.AppendCommandChain(cc);

                    Thread.Sleep(20000);
                    int devCount = ble.GetDeviceFoundCount();
                    int bestDeviceID = 0;
                    int bestRSSI = 0;

                    BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                    for (int i = 0; i < devCount; ++i)
                    {
                        ble.GetDeviceInfo(i, ref devInfo);
                        if (i == 0)
                        {
                            bestRSSI = devInfo.RSSI;
                        }
                        else
                        {
                            if (devInfo.RSSI > bestRSSI)
                            {
                                bestDeviceID = i;
                                bestRSSI = devInfo.RSSI;
                            }
                        }
                    }

                    ble.GetDeviceInfo(bestDeviceID, ref devInfo);


                    MessageBox.Show("Best Device was found with mac of " + devInfo.macString());

                    BLE.V2_2.Device dev = ble.connectToDevice(devInfo);
                    dev.SyncWaitForConnection(-1);
                    dev.SyncWaitForDisconnection(100);
                    dev.SyncScanServicesInRange(MiToken_BLE_UUIDs.BLE_UUID_SERVICE_SECURE_BOND, 0xFF, -1);
                    byte[] cmdBuffer = new byte[17];
                    cmdBuffer[0] = 0x01;
                    cmdBuffer[1] = 0xFF;
                    dev.SyncSetAttribute(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_CONFIG, cmdBuffer, -1);
                    List<byte[]> states = new List<byte[]>();
                    states.Add(new byte[] { 0x01 });
                    states.Add(new byte[] { 0x02 });
                    dev.SyncWaitForState(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, false, states, -1);
                    byte[] read;
                    dev.SyncReadAttribute(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_PUBLIC_KEY, true, out read, -1);

                }));
            T.Start();


            /* // DLL WRAPPER CODE
            BLE_API_DLL.DLLWrapper.V2_2.MiTokenBLEV2_2 BLE = new BLE_API_DLL.DLLWrapper.V2_2.MiTokenBLEV2_2();
            BLE.Initialize("COM3", "debugpipe", true);
            BLE_API_DLL.DLLWrapper.CommandChain CC = new BLE_API_DLL.DLLWrapper.CommandChain();
            CC.evt.attclient.attribute_value = attribute_value;
            CC.evt.gap.scan_response = scan_response;

            BLE.AppendCommandChain(CC);


            Thread.Sleep(2000);
            int DevCount = BLE.GetDeviceFoundCount();
            int bestDeviceID = 0;
            int bestRSSI = 0;

            BLE_API_DLL.DLLWrapper.BLEDeviceInfo devInfo = new BLE_API_DLL.DLLWrapper.BLEDeviceInfo();

            for (int i = 0; i < DevCount; ++i)
            {
                BLE.GetDeviceInfo(i, ref devInfo);
                if (i == 0)
                {
                    bestRSSI = devInfo.RSSI;
                }
                else
                {
                    if (devInfo.RSSI > bestRSSI)
                    {
                        bestDeviceID = i;
                        bestRSSI = devInfo.RSSI;
                    }
                }
            }

            BLE.GetDeviceInfo(bestDeviceID, ref devInfo);

            MessageBox.Show("Best Device was found with mac of " + devInfo.macString());

            BLE_API_DLL.DLLWrapper.V2_2.MiTokenBLEV2_2.DeviceV2_2 dev = (BLE_API_DLL.DLLWrapper.V2_2.MiTokenBLEV2_2.DeviceV2_2)BLE.connectToDevice(devInfo);
            dev.SyncWaitForConnection(-1);
            dev.SyncScanServicesInRange(MiToken_BLE_UUIDs.BLE_UUID_SERVICE_SECURE_BOND, 0xFF, -1);
            byte[] cmdBuffer = new byte[17];
            cmdBuffer[0] = 0x01;
            cmdBuffer[1] = 0xFF;
            dev.SyncSetAttribute(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_CONFIG, cmdBuffer, -1);
            List<byte[]> states = new List<byte[]>();
            states.Add(new byte[] { 0x01 });
            states.Add(new byte[] { 0x02 });
            dev.SyncWaitForState(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS, false, states, -1);
            byte[] read;
            dev.SyncReadAttribute(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_SECURE_BOND_PUBLIC_KEY, true, out read, -1);
            */
        }


        public static Dictionary<BLE.V2_2.MiTokenBLE, DebugStart> bleDictionary = new Dictionary<BLE.V2_2.MiTokenBLE, DebugStart>();

        public static void GotDebugMessage(IntPtr sender, string message)
        {
            BLE.V2_2.MiTokenBLE senderInstance = BLE.Core.LookupOnIntPtrV2_2(sender);

            GDM2(senderInstance, message);
        }

        public static void GDM2(BLE.V2_2.MiTokenBLE instance, string message)
        {
            if (bleDictionary.ContainsKey(instance))
            {
                bleDictionary[instance].Debugger_Debugger_SendDebugMessageCallback(message);
            }
        }

        
        void Debugger_Debugger_SendDebugMessageCallback(string message)
        {
            if (InvokeRequired)
            {
                BeginInvoke((MethodInvoker)delegate

                {
                    Debugger_Debugger_SendDebugMessageCallback(message);
                    return;
                });

                return;
            }

            string[] msplit = message.Split(new string[] { "\r\n" }, StringSplitOptions.None);
            bool useindex = false;
            foreach (string s in msplit)
            {
                useindex = true;
                listBox1.Items.Add(s);
            }
            if (useindex && (!checkPause.Checked))
            {
                listBox1.SelectedIndex = listBox1.Items.Count - 1;
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {

        }

        private void button6_Click(object sender, EventArgs e)
        {
            NewCalibrationDummy NCD = new NewCalibrationDummy();
            NCD.Show();
        }
    }
}
