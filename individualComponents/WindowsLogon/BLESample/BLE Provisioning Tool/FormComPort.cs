using System;   
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Management;
using Microsoft.Win32;
using System.Diagnostics;

namespace BLE_Provisioning_Tool
{
    public partial class FormComPort : Form
    {
        // Properties
        public string ComPort { get; private set; }

        // Constructors
        public FormComPort()
        {
            InitializeComponent();

            ComPort = String.Empty;

            Init();

            this.DoubleBuffered = true;
        }

        // Functions
        public void Init()
        {
            // Initial refresh
            RefreshComPorts();

            // Position form
            this.StartPosition = FormStartPosition.CenterParent;
        }

        private void RefreshComPorts()
        {
            // Clear combobox
            comboBoxPort.Items.Clear();

            // Populate with serial ports only if a bluegiga bluetooth dongle
            string[] serialPorts = System.IO.Ports.SerialPort.GetPortNames();
            using (var searcher = new ManagementObjectSearcher("SELECT * FROM WIN32_SerialPort"))
            {
                var ports = searcher.Get().Cast<ManagementBaseObject>().ToList();
                var tList = (from n in serialPorts join p in ports on n equals p["DeviceID"].ToString() select p["Caption"]).ToList();
                
                foreach (string s in tList)
                {
                    if (s.Contains("Bluegiga Bluetooth Low Energy"))
                    {
                        string[] splitStr = s.Split('(', ')');
                        foreach (string sx in splitStr)
                        {
                            if (sx.Contains("COM"))
                                comboBoxPort.Items.Add(sx);
                        }
                    }
                }
            }

            // Select default value if available
            try
            {
                comboBoxPort.SelectedIndex = 0;
            }
            catch
            {
                Debug.WriteLine(String.Format("No serial ports available"));
            }
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            // Check if null or empty
            try
            {
                if (comboBoxPort.SelectedItem == null)
                {
                    return;
                }
                string itemStr = comboBoxPort.SelectedItem.ToString();
                if (!string.IsNullOrEmpty(itemStr))
                {
                    ComPort = itemStr;
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                }
            }
            catch
            {
                // Ignore
            }
        }

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            RefreshComPorts();
        }

        public void SaveToRegistry()
        {
            RegistryKey key = Registry.LocalMachine.OpenSubKey(Constants.BASE_REGISTRY_PATH, true);
            {
                if (key == null)
                {
                    key = Registry.LocalMachine.CreateSubKey(Constants.BASE_REGISTRY_PATH);
                }

                key.SetValue(Constants.SERIAL_PORT_REGISTER, ComPort, RegistryValueKind.String);
            }

            key.Close();
        }

        public void RestoreFromRegistry()
        {
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey(Constants.BASE_REGISTRY_PATH, false))
            {
                if (key != null)
                {
                    ComPort = (string)key.GetValue(Constants.SERIAL_PORT_REGISTER, "");
                }
            }
        }
    }
}
