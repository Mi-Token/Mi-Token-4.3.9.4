using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BLE_Provisioning_Tool
{
    public partial class FormAutoProv : Form
    {
        private BeaconData m_originalBeaconData = new BeaconData();
        public BeaconData m_beaconData = new BeaconData();
        public int m_advertIntervalMs = 0;
        public int m_originalAdvertIntervalMs = 0;

        public FormAutoProv(BeaconData beaconData, int advertIntervalMs)
        {
            InitializeComponent();

            // Position form
            this.StartPosition = FormStartPosition.CenterParent;

            // Update member variables
            m_originalBeaconData = beaconData;
            m_beaconData = beaconData;
            m_originalAdvertIntervalMs = advertIntervalMs;
            m_advertIntervalMs = advertIntervalMs;

            // Update controls
            textBoxUuid.Text = m_originalBeaconData.UUID.GetString();
            textBoxMajor.Text = m_originalBeaconData.Major.ToString();
            textBoxMinor.Text = m_originalBeaconData.Minor.ToString();
            textBoxAdvertInterval.Text = m_originalAdvertIntervalMs.ToString();
        }

        private void textBoxUuid_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Ignore validation if cancel button has been selected
                if (buttonCancel.Focused)
                    return;

                string originalStr = ((Control)sender).Text;

                // Ensure correct length
                if (originalStr.Length != Constants.DEVICE_UUID_STRING_LEN)
                {
                    throw new System.Exception(string.Format("{0}:\nLength must be {1}", labelUuid.Text, Constants.DEVICE_UUID_STRING_LEN));
                }

                // Ensure markers in correct position
                int[] delimMarkers = { 8, 13, 18, 23 };
                foreach (int marker in delimMarkers)
                {
                    if (originalStr.ElementAt(marker) != '-')
                    {
                        throw new System.Exception(string.Format("{0}:\nIncorrect format.\nFormat: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX", labelUuid.Text));
                    }
                }

                // Remove deliminators and rebuild string
                string[] splitStr = originalStr.Split('-');
                StringBuilder builder1 = new StringBuilder();
                foreach (string split in splitStr)
                {
                    builder1.Append(split);
                }

                // Seperate into hex values
                string noDelimString = builder1.ToString();
                DeviceUUID hexVals = new DeviceUUID();
                for (int i = 0; i < hexVals.Length; ++i)
                {
                    // Covert to hex
                    hexVals.Value[i] = Convert.ToByte(noDelimString.Substring(i * 2, 2), 16);
                }

                // Save value
                m_beaconData.UUID = hexVals;
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                MessageBoxEx.Show(exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

                // Reset to original value
                if (m_originalBeaconData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalBeaconData.UUID.GetString();
                }
            }
        }

        private void textBoxUuid_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
        }

        private void textBoxMajor_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Ignore validation if cancel button has been selected
                if (buttonCancel.Focused)
                    return;

                // Attempt to convert to decimal
                UInt16 val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value within bounds
                if (val < ConfigurationData.MIN_MAJOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelMajor.Text, ConfigurationData.MIN_MAJOR));
                }
                else if (val > ConfigurationData.MAX_MAJOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelMajor.Text, ConfigurationData.MAX_MAJOR));
                }
                else
                {
                    // Save Value
                    m_beaconData.Major = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                MessageBoxEx.Show(exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

                // Reset to original value
                if (m_originalBeaconData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalBeaconData.Major.ToString();
                }
            }
        }

        private void textBoxMajor_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
        }

        private void textBoxMinor_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Ignore validation if cancel button has been selected
                if (buttonCancel.Focused)
                    return;

                // Attempt to convert to decimal
                UInt16 val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value within bounds
                if (val < ConfigurationData.MIN_MINOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelMinor.Text, ConfigurationData.MIN_MINOR));
                }
                else if (val > ConfigurationData.MAX_MINOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelMinor.Text, ConfigurationData.MAX_MINOR));
                }
                else
                {
                    // Save value
                    m_beaconData.Minor = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                MessageBoxEx.Show(exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

                // Reset to original value
                if (m_originalBeaconData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalBeaconData.Minor.ToString();
                }
            }
        }

        private void textBoxMinor_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
        }

        private void buttonApply_Click(object sender, EventArgs e)
        {
            if (this.ValidateChildren())
            {
                DialogResult = DialogResult.OK;
                Close();
            }
        }

        private void textBoxAdvertInterval_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Ignore validation if cancel button has been selected
                if (buttonCancel.Focused)
                    return;

                // Attempt to convert to decimal
                int val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value is within bounds
                if (val < ConfigurationData.MIN_ADVERT_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelAdvertInterval.Text, ConfigurationData.MIN_ADVERT_INTERVAL_MS));
                }
                else if (val > ConfigurationData.MAX_ADVERT_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelAdvertInterval.Text, ConfigurationData.MAX_ADVERT_INTERVAL_MS));
                }
                else
                {
                    // Save value
                    m_advertIntervalMs = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                MessageBoxEx.Show(exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

                // Reset to original value
                ((Control)sender).ForeColor = System.Drawing.Color.Black;
                ((Control)sender).Text = m_originalAdvertIntervalMs.ToString();
            }
        }

        private void textBoxAdvertInterval_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;   
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }

    }
}
