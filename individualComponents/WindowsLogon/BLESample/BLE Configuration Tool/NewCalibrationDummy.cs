using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using BLE_API;


namespace BLE_Configuration_Tool
{
    public partial class NewCalibrationDummy : Form
    {
        public NewCalibrationDummy()
        {
            InitializeComponent();
            mStage = Stage.SInit;
        }

        NewCalibrationConfig NCN = new NewCalibrationConfig();

        Stage mStage;
        enum Stage
        {
            SInit,
            TokenLevel,
            SignalStrength,
            Sensitivity,
            Done,
        };

        BLE.V2_2.MiTokenBLE MiTokenBLE = null;
        string selectedTokenMac = "";

        int LockPercent = 0, UnlockPercent = 0;


        private void NewCalibrationDummy_Load(object sender, EventArgs e)
        {
            try
            {
                MiTokenBLE = new BLE.V2_2.MiTokenBLE();
                MiTokenBLE.Initialize(Program.portID, @"\\.\pipe\Mi-TokenBLEV2", false);

                MiTokenBLE.NewPollResults += MiTokenBLE_NewPollResults;
                MiTokenBLE.AddPollThreadInstance();

            }

            catch(Exception ex)
            {
                MessageBox.Show("Exception : thrown during form load\r\n" + ex.Message + "\r\nStack:\r\n" + ex.StackTrace);
            }
        }

        void MiTokenBLE_NewPollResults(BLE.V2_1.MiTokenBLE sender)
        {
            if(InvokeRequired)
            {
                Invoke((MethodInvoker)delegate{MiTokenBLE_NewPollResults(sender); });
                return;
            }

            BLE.V2_2.MiTokenBLE bv2_2 = (BLE.V2_2.MiTokenBLE)sender;
            int icount = bv2_2.GetDeviceFoundCount();
            BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
            bool foundDevice = false;

            for (int i = 0; i < icount; ++i)
            {
                bv2_2.GetDeviceInfo(i, ref devInfo);
                if (devInfo.MacString == selectedTokenMac)
                {
                    foundDevice = true;
                    break;
                }
            }

            if (!foundDevice)
            {
                devInfo = new BLE.Core.Structures.DeviceInfo();
            }

            switch (mStage)
            {
                case Stage.SInit:
                    {
                        
                        string selectedItem = null;
                        if ((listBox1.Items.Count > 0) && (listBox1.SelectedIndex >= 0))
                        {
                            selectedItem = (string)listBox1.Items[listBox1.SelectedIndex];
                            selectedItem = selectedItem.Substring(0, selectedItem.IndexOf(' '));
                        }
                        listBox1.Items.Clear();
                        
                        
                        for (int i = 0; i < icount; ++i)
                        {
                            bv2_2.GetDeviceInfo(i, ref devInfo);
                            listBox1.Items.Add(devInfo.MacString + " " + devInfo.SignalQuality);
                        }

                        if (selectedItem != null)
                        {
                            for (int i = 0; i < listBox1.Items.Count; ++i)
                            {
                                string s = (string)listBox1.Items[i];
                                s = s.Substring(0, s.IndexOf(' '));
                                if (s == selectedItem)
                                {
                                    listBox1.SelectedIndex = i;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case Stage.TokenLevel:
                    {
                        groupLocationCalibration.Enabled = true;
                        labLatestRSSI.Text = "Current Signal : " + devInfo.SignalQuality + "%";
                        
                    }
                    break;
                case Stage.SignalStrength:
                    {
                        groupBox1.Enabled = true;
                        
                        label1.Text = "Current Signal : " + devInfo.SignalQuality + "%";
                        NCN.AddSignalPoll(devInfo.SignalQuality, devInfo.sRSSI);
                        progressBar1.Value = NCN.Progress;
                        if (NCN.Progress == 100)
                        {
                            button4.Enabled = true;
                            label12.Text = NCN.PollReadAverage + "%";
                            label13.Text = NCN.LockRSSIPercent + "%";

                            UnlockPercent = NCN.PollReadAverage;
                            LockPercent = NCN.LockRSSIPercent;
                        }
                        else
                        {
                            button4.Enabled = false;
                        }
                    }
                    break;
                case Stage.Sensitivity:
                    {
                        groupBox2.Enabled = true;
                        label14.Text = "Current Signal : " + devInfo.SignalQuality + "%";
                        
                        Invoke((MethodInvoker)delegate
                        {
                            checkBox1.Checked = devInfo.SignalQuality >= UnlockPercent;

                            checkBox2.Checked = devInfo.SignalQuality < LockPercent;
                        });

                    }
                    break;
                case Stage.Done:
                    { }
                    break;
                        
            }

        }

        private void button1_Click(object sender, EventArgs e)
        {
            button1.Enabled = false;
            button2.Enabled = true;
            string s = (string)listBox1.Items[listBox1.SelectedIndex];
            s = s.Substring(0, s.IndexOf(' '));
            selectedTokenMac = s;
            mStage = Stage.TokenLevel;
        }

        private void button4_Click(object sender, EventArgs e)
        {
            groupBox1.Enabled = groupBox2.Enabled = groupLocationCalibration.Enabled = false;
            switch (mStage)
            {
                case Stage.Done:
                    break;
                case Stage.Sensitivity:
                    mStage = Stage.Done;
                    break;
                case Stage.SignalStrength:
                    mStage = Stage.Sensitivity;
                    break;
                case Stage.SInit:
                    MessageBox.Show("Please select a token and select start");
                    break;
                case Stage.TokenLevel:
                    mStage = Stage.SignalStrength;
                    break;
            }
        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            int lockPer = 0, unlockPer = 0;
            double sen = trackBar1.Value * 0.25;

            NCN.getLockUnlockForSensitivity(sen, ref unlockPer, ref lockPer);

            label12.Text = unlockPer + "%";
            label13.Text = lockPer + "%";

            UnlockPercent = unlockPer;
            LockPercent = lockPer;
        }
    }
}
