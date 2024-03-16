using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using BLE_API;

namespace BLE_Provisioning_Tool
{
    public partial class FormOad : Form
    {
        private enum OadState
        {
            Init,
            Start,
            Progress,
            Cancel
        };
        Dictionary<OadState, String> m_oadStateDict = new Dictionary<OadState, string>
        {
            {OadState.Init, "No file selected"},
            {OadState.Start, OAD_TEXT_START},
            {OadState.Cancel, "Cancelling..."}
        };

        ImgHdr m_currentImgHdr = null;
        ImgHdr m_newImgHdr = null;
        String m_newImgFileName = null;
        BLE.V2_2.Device m_connectedDevice = null;
        ManualResetEventSlim m_connectionDisconnectedEvent = null;
        OadService m_oadService = null;
        private static readonly String OAD_TEXT_START = "Press 'Start' to begin";

        public FormOad(OadService oadService, BLE.V2_2.Device connectedDevice, ManualResetEventSlim connectionDisconnectedEvent)
        {
            InitializeComponent();

            // Position form
            this.StartPosition = FormStartPosition.CenterParent;

            // Copy OAD service
            m_oadService = oadService;

            // Copy current image header
            m_currentImgHdr = new ImgHdr();
            m_currentImgHdr = oadService.getTargetImgHdr();

            // Copy BLE device
            m_connectedDevice = connectedDevice;

            // Copy Connection Disconnection Event
            m_connectionDisconnectedEvent = connectionDisconnectedEvent;

            // Display current image header
            labelCurImgSize.Text = m_currentImgHdr.getByteSizeStr();
            labelCurImgType.Text = m_currentImgHdr.getTypeStr();
            labelCurImgUid.Text = m_currentImgHdr.getUidStr();
            labelCurImgVer.Text = m_currentImgHdr.getVerNumStr();

            // Clear out new image header
            labelNewImgSize.Text = "-";
            labelNewImgType.Text = "-";
            labelNewImgUid.Text = "-";
            labelNewImgVer.Text = "-";

            // Clear out new image text box
            textBoxNewImg.Text = "";

            // Assign status text
            labelPrgStatus.Text = m_oadStateDict[OadState.Init];

            // Enable/disable programming buttons
            buttonPrgStart.Enabled = false;
            buttonPrgCancel.Enabled = false;

            m_bOadResult = false;

            // Display prompt if attempting to update Image A
            if (!m_currentImgHdr.isImgA())
            {
                MessageBox.Show("You are attempting to update the Application Updater (Image A).\nPlease proceed with caution.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void buttonNewImg_Click(object sender, EventArgs e)
        {
            ImgHdr newImgHdr = new ImgHdr();

            // Select binary image            
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "Binaries (*.bin)|*.bin";
            openFileDialog.FilterIndex = 1;
            openFileDialog.RestoreDirectory = true;

            // Open and read file
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                using (BinaryReader br = new BinaryReader(openFileDialog.OpenFile()))
                {
                    // Parse image file header
                    byte[] buff = new byte[ImgHdr.TOTAL_LEN];
                    br.Read(buff, 0, ImgHdr.TOTAL_LEN);
                    if (!newImgHdr.ReadNewImgHdr(buff))
                    {
                        MessageBoxEx.Show("Image header not correct length", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }
                }

                //*** Validate image
                // Ensure version type is different
                if (newImgHdr.getTypeStr().CompareTo(m_currentImgHdr.getTypeStr()) == 0)
                {
                    MessageBoxEx.Show(String.Format("The image selected is an invalid type: {0}\nMust be different to current image: {1}", newImgHdr.getTypeStr(), m_currentImgHdr.getTypeStr()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                // Ensure image size is correct
                if (newImgHdr.getByteSize() > (newImgHdr.isImgA() ?  OadService.IMAGE_A_MAX_SIZE_BYTES : OadService.IMAGE_B_MAX_SIZE_BYTES))
                {
                    MessageBoxEx.Show(String.Format("The image selected is an invalid size: {0} bytes\nImage size must not exceed a size of: {1}", newImgHdr.getByteSize(), (newImgHdr.isImgA() ? OadService.IMAGE_A_MAX_SIZE_BYTES : OadService.IMAGE_B_MAX_SIZE_BYTES)), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                else if (newImgHdr.getByteSize() == 0)
                {
                    MessageBoxEx.Show(String.Format("The image selected is an invalid size: {0} bytes\nImage size cannot be empty.", newImgHdr.getByteSize()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                // Check User ID
                if (!m_currentImgHdr.isUidValid(newImgHdr))
                {
                    MessageBoxEx.Show(String.Format("The image selected has an invalid ID: {0}\n", newImgHdr.getUidStr()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // Save new image header and file name
                m_newImgHdr = new ImgHdr();
                m_newImgHdr = newImgHdr;
                m_newImgFileName = openFileDialog.FileName;

                // Display new image header
                labelNewImgSize.Text = m_newImgHdr.getByteSizeStr();
                labelNewImgType.Text = m_newImgHdr.getTypeStr();
                labelNewImgUid.Text = m_newImgHdr.getUidStr();
                labelNewImgVer.Text = m_newImgHdr.getVerNumStr();
                
                // Display new imagefile selected
                textBoxNewImg.Text = openFileDialog.FileName;
                
                // Update OAD start string
                labelPrgStatus.Text = m_oadStateDict[OadState.Start];

                // Enable/disable programming buttons
                buttonPrgStart.Enabled = true;
                buttonPrgCancel.Enabled = false;
            }

            // Dispose dialog
            openFileDialog.Dispose();
        }        

        // Cancellation token
        CancellationTokenSource cancelSource = null;

        bool m_bOadResult = false;        

        private async void buttonPrgStart_Click(object sender, EventArgs e)
        {
            bool isFastMode = false;
            DialogResult dlgResult = MessageBox.Show("Would you like to use a (~x5) faster but less reliable transfer?\nOtherwise a slower but reliable transfer will be selected.\n\nFor best results place Token next to Blugiga Dongle.", "Update", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            switch( dlgResult)
            {
                case DialogResult.Yes:
                    isFastMode = true;
                    break;
                case DialogResult.No:
                    isFastMode = false;
                    break;
                default:
                    return;
            }
            m_bOadResult = false;

            // Create new cancellation token source
            cancelSource = new CancellationTokenSource();

            // Update controls for OAD in progress
            // labelPrgStatus is updated during OAD process
            buttonPrgStart.Enabled = false;
            buttonPrgCancel.Enabled = true;
            buttonNewImg.Enabled = false;

            // Update progress bar asynchronously so that it does not block UI thread
            IProgress<OadService.UiData> uiData = new Progress<OadService.UiData>(ud => 
            {
                progressBarStatus.Value = ud.StatusPercent;
                labelPrgStatus.Text = String.Format("Time[mm:ss]: {0:mm\\:ss}\n{1:n0}/{2:n0} bytes ({3}%)", ud.EstTimeRemaining, ud.CurrentBytes, ud.TotalBytes, ud.StatusPercent); 
            });

            try
            {
                // Initiate OAD process
#if DEBUG_DIAGNOSTICS
                Stopwatch stopwatch = new Stopwatch();
                stopwatch.Start();
#endif
                m_bOadResult = await Task.Run<bool>(() => m_oadService.PerformOAD(m_connectedDevice, m_connectionDisconnectedEvent, m_newImgHdr, m_newImgFileName, uiData, cancelSource.Token, isFastMode));
#if DEBUG_DIAGNOSTICS
                stopwatch.Stop();
                Debug.WriteLine(String.Format("Total Time elapsed: {0}", stopwatch.Elapsed));
#endif

                // Update controls for OAD in progress
                if (m_bOadResult)
                {                    
                    // OAd Success
                    buttonPrgStart.Enabled = false;
                    buttonPrgCancel.Enabled = false;
                    buttonNewImg.Enabled = false;
                    // Allow labelPrgStatus and progressBarStatus to display last values
                }
                else
                {
                    // OAD Fail
                    buttonPrgStart.Enabled = true;
                    buttonPrgCancel.Enabled = false;
                    buttonNewImg.Enabled = true;
                    // Allow labelPrgStatus and progressBarStatus to display last values
                    progressBarStatus.Value = 0;
                }
            }
            finally
            {
                // Check if user requested to cancel OAD
                if (!cancelSource.IsCancellationRequested)
                {
                    if (m_bOadResult)
                    {
                        MessageBoxEx.Show("Update Successful!", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);

                        // Exit form as complete
                        DialogResult = DialogResult.OK;
                        Close();
                    }
                    else
                    {
                        // Set start text and re-enable start button
                        labelPrgStatus.Text = m_oadStateDict[OadState.Start];
                        buttonPrgStart.Enabled = true;
                        buttonPrgCancel.Enabled = false;

                        if (m_connectionDisconnectedEvent.IsSet)
                        {
                            MessageBoxEx.Show("Update Failed - Token Disconnected!", "Status", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        }
                        else
                        {
                            MessageBoxEx.Show("Update Failed!", "Status", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        }
                    }
                }
                else
                {
                    // Set cancellation text, reset progress bar and disable start & cancel buttons
                    labelPrgStatus.Text = m_oadStateDict[OadState.Cancel];
                    progressBarStatus.Value = 0;
                    buttonPrgStart.Enabled = false;
                    buttonPrgCancel.Enabled = false;
                    buttonNewImg.Enabled = false;
                }                
            }
        }

        private async void buttonPrgCancel_Click(object sender, EventArgs e)
        {
            // Ensure cancellation flag is set in order for background task to end gracefully
            if (cancelSource != null)
            {
                cancelSource.Cancel();
            }

            // Allow a delay to show cancellation text
            await Task.Delay(1000);

            // Set start text and re-enable start button
            labelPrgStatus.Text = m_oadStateDict[OadState.Start];
            buttonPrgStart.Enabled = true;
            buttonPrgCancel.Enabled = false;
            buttonNewImg.Enabled = true;
        }

        private void FormOad_FormClosing(object sender, FormClosingEventArgs e)
        {
            // Prompt if OAD process was not sucessful / cancelled or not started
            if (!m_bOadResult)
            {
                if (DialogResult.OK == MessageBoxEx.Show("Are you sure you want to Exit?\nOAD will be cancelled.", "Exit", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning))
                {
                    // Ensure cancellation flag is set in order for background task to end gracefully
                    if (cancelSource != null)
                    {
                        cancelSource.Cancel();
                    }

                    // Exit form as complete
                    DialogResult = DialogResult.OK;
                }
                else
                {
                    e.Cancel = true;
                }
            }
        }
    }
}
