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
    public partial class FormBulkOAD : Form
    {
        ImgHdr _newImgHdr = null;
        String _newImgFileName = null;
        FormProvTool _formProvTool = null;
        CancellationTokenSource _cancelSource = null;
        String _fwRevStr = "";
        String _hwRevStr = "";
        
        public FormBulkOAD(FormProvTool formProvTool)
        {            
            InitializeComponent();

            // Position form
            this.StartPosition = FormStartPosition.CenterParent;

            _formProvTool = formProvTool;

            buttonSelect.Enabled = true;
            buttonStart.Enabled = false;
            buttonCancel.Enabled = false;
            buttonVerify.Enabled = false;
        }

        public void SetStatus(String msg)
        {
            if (!richTextBoxStatus.IsDisposed)
            {
                richTextBoxStatus.AppendText(DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt: ") + msg + Environment.NewLine);
            }
        }

        private void buttonSelect_Click(object sender, EventArgs e)
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
                // Ensure image size is correct
                if (newImgHdr.getByteSize() > (newImgHdr.isImgA() ? OadService.IMAGE_A_MAX_SIZE_BYTES : OadService.IMAGE_B_MAX_SIZE_BYTES))
                {
                    MessageBoxEx.Show(String.Format("The image selected is an invalid size: {0} bytes\nImage size must not exceed a size of: {1}", newImgHdr.getByteSize(), (newImgHdr.isImgA() ? OadService.IMAGE_A_MAX_SIZE_BYTES : OadService.IMAGE_B_MAX_SIZE_BYTES)), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                else if (newImgHdr.getByteSize() == 0)
                {
                    MessageBoxEx.Show(String.Format("The image selected is an invalid size: {0} bytes\nImage size cannot be empty.", newImgHdr.getByteSize()), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // Save new image header and file name
                _newImgHdr = new ImgHdr();
                _newImgHdr = newImgHdr;
                _newImgFileName = openFileDialog.FileName;
                
                // Find Hardware and Firmware Revisions
                _fwRevStr = "";
                _hwRevStr = "";
                byte[] allBytes = File.ReadAllBytes(openFileDialog.FileName);
                List<byte[]> searchHwRevBytes = new List<byte[]>();
                searchHwRevBytes.Add(Encoding.UTF8.GetBytes("F1_CC2540"));
                searchHwRevBytes.Add(Encoding.UTF8.GetBytes("VTAG_CC2541"));
                searchHwRevBytes.Add(Encoding.UTF8.GetBytes("MiniBeacon_CC2541"));
                searchHwRevBytes.Add(Encoding.UTF8.GetBytes("WellCore_CC2541"));
                int resultIdx = ByteSearch(allBytes, searchHwRevBytes, ImgHdr.TOTAL_LEN);                
                String revisionStrStatus = "";
                if (resultIdx != -1)
                {
                    // Get Hardware String
                    List<byte> bb = new List<byte>();
                    int idx = resultIdx;
                    while ((allBytes[idx] != 0x00) && (idx != (allBytes.Length - 1)))
                    {
                        bb.Add(allBytes[idx++]);
                    }                    
                    _hwRevStr = Encoding.UTF8.GetString(bb.ToArray());

                    // Get Firmware String
                    bb.Clear();
                    idx++; // Skip null terminator
                    while ((allBytes[idx] != 0x00) && (idx != (allBytes.Length - 1)))
                    {
                        bb.Add(allBytes[idx++]);
                    }
                    _fwRevStr = Encoding.UTF8.GetString(bb.ToArray());

                    if (_fwRevStr != "" && _hwRevStr != "")
                    {
                        revisionStrStatus = " Firmware:\t" + _fwRevStr + Environment.NewLine +
                                " Hardware:\t" + _hwRevStr + Environment.NewLine;
                        buttonVerify.Enabled = true;
                    }
                }

                // Display new image header
                SetStatus(Environment.NewLine + Environment.NewLine + "Selected image header:" + Environment.NewLine +
                        " Filename:\t" + openFileDialog.FileName + Environment.NewLine +
                        " Size:\t\t" + _newImgHdr.getByteSizeStr() + Environment.NewLine +
                        " Type:\t\t" + _newImgHdr.getTypeStr() + Environment.NewLine +
                        " Version:\t\t" + _newImgHdr.getVerNumStr() + Environment.NewLine +
                        " User ID:\t\t" + _newImgHdr.getUidStr() + Environment.NewLine +
                        revisionStrStatus);

                // Enable/disable buttons
                buttonStart.Enabled = true;
            }
        }
        
        async private void buttonStart_Click(object sender, EventArgs e)
        {
            buttonSelect.Enabled = false;
            buttonStart.Enabled = false;
            buttonCancel.Enabled = true;
            buttonVerify.Enabled = false;

            // Create new cancellation token source
            _cancelSource = new CancellationTokenSource();

            Stopwatch sw = new Stopwatch();
            sw.Start();

            SetStatus("Bulk Update Started");
            using (UpdateStateMachine stateMachine = new UpdateStateMachine(this))
            {
                try
                {
                    bool isComplete = false;
                    while (!isComplete)
                    {
                        int waitHandleIndex = EventWaitHandle.WaitAny(stateMachine.WaitEventHandles());
                        switch (waitHandleIndex)
                        {
                            case 0:
                                sw.Stop();
                                SetStatus("Bulk Update Complete: Total Time=" + sw.Elapsed);
                                isComplete = true;
                                break;
                            case 1:
                                stateMachine._newStateEvent.Reset();
                                SetStatus("State: " + stateMachine.State.GetName());
                                await stateMachine.Handle();
                                break;

                            default:
                                throw new Exception("Invalid Wait Handle Index");
                        }
                    }
                }
                catch (Exception exp)
                {
                    SetStatus("Exception: " + exp.Message);
                }

                // Update status with summary                                
                StringBuilder sb = new StringBuilder();
                sb.AppendLine();
                sb.AppendLine();
                sb.AppendLine("Summary:");
                sb.AppendLine();
                foreach (Tuple<DeviceAddress, String, int, TimeSpan> summary in stateMachine._summary)
                {
                    sb.AppendLine(" Device Address:\t" + summary.Item1.GetDisplayString());
                    sb.AppendLine(" Update Status:\t" + summary.Item2);
                    sb.AppendLine(" Attempts:\t\t" + summary.Item3);
                    sb.AppendLine(" Total Time:\t" + summary.Item4);
                    sb.AppendLine();
                }
                if (stateMachine._summary.Count > 0)
                {
                    SetStatus(sb.ToString());
                }
            }

            if ((_fwRevStr != "") && (_hwRevStr != ""))
            {
                buttonVerify.Enabled = true;
            }

            buttonSelect.Enabled = true;
            buttonStart.Enabled = true;
            buttonCancel.Enabled = false;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            // Ensure cancellation flag is set in order for background task to end gracefully
            if (_cancelSource != null)
            {
                SetStatus("Cancelling...");
                _cancelSource.Cancel();
                buttonCancel.Enabled = false;
            }
        }

        class UpdateStateMachine : IDisposable
        {
            private UpdateState _state;
            public DeviceAddress _validDevAddr = new DeviceAddress();
            public OadService _oadService = new OadService();
            public List<Tuple<DeviceAddress, String, int, TimeSpan>> _summary = new List<Tuple<DeviceAddress, String, int, TimeSpan>>();

            public ManualResetEventSlim _completeEvent = new ManualResetEventSlim(false);
            public ManualResetEventSlim _newStateEvent = new ManualResetEventSlim(false);

            public LookingForTokenState _lookingForTokenState = null;
            public UpdateTokenState _updateTokenState = null;
            public VerifyTokenState _verifyTokenState = null;

            public Stopwatch _updateTime = new Stopwatch();
            
            public FormBulkOAD _form = null;

            public WaitHandle[] WaitEventHandles()
            {
                return new WaitHandle[]
                {
                    _completeEvent.WaitHandle,
                    _newStateEvent.WaitHandle
                };                
            }

            public UpdateStateMachine(FormBulkOAD form)
            {
                _form = form;

                _lookingForTokenState = new LookingForTokenState();
                _updateTokenState = new UpdateTokenState();
                _verifyTokenState = new VerifyTokenState();

                // Initial state
                this.State = _lookingForTokenState;
            }

            public void Dispose()
            {
                // Stop searching for tokens
                GetMiTokenBLE().RemovePollThreadInstance();
                GetMiTokenBLE().NewPollResults -= _lookingForTokenState.PollResults;                
            }

            public UpdateState State
            {
                get { return _state; }
                set 
                { 
                    _state = value;
                    if (_state.GetType() == typeof(LookingForTokenState))
                    {
                        _lookingForTokenState.IgnoreCurrentDevice();
                        _updateTokenState.Attempt = 0;
                    }
                    _newStateEvent.Set();
                }
            }

            public async Task Handle()
            {
                if (_form._cancelSource.IsCancellationRequested)
                {
                    // Ends state machine
                    _completeEvent.Set();
                    return;
                }
                await _state.Handle(this);
            }

            public BLE.V2_2.MiTokenBLE GetMiTokenBLE()
            {
                return _form._formProvTool.GetMiTokenBLE();
            }

            public FormProvTool GetFormProvTool()
            {
                return _form._formProvTool;
            }

            public void Start()
            {
                _updateTime.Restart();
                State = _updateTokenState;
            }

            public void Stop(String resultStr)
            {
                _updateTime.Stop();
                _form.SetStatus("Adding to ignore list: " + _validDevAddr.GetDisplayString());
                _summary.Add(new Tuple<DeviceAddress, String, int, TimeSpan>(_validDevAddr, resultStr, _updateTokenState.Attempt, _updateTime.Elapsed));
                State = _lookingForTokenState;
            }

            public abstract class UpdateState
            {
                abstract public String GetName();
                public virtual async Task Handle(UpdateStateMachine stateMachine) { }
            }

            public class LookingForTokenState : UpdateState
            {
                List<DeviceAddress> _ignoreDevices = new List<DeviceAddress>();
                ManualResetEventSlim _locateValidToken = new ManualResetEventSlim(false);
                DeviceAddress _validDevAddr = new DeviceAddress();
                BLE.V2_2.MiTokenBLE _MiTokenBLE = null;

                public void IgnoreCurrentDevice()
                {
                    // Add current device to ignore lsit
                    if (!_validDevAddr.IsEmpty())
                    {
                        _ignoreDevices.Add(_validDevAddr);
                    }

                    // Reset current device
                    _validDevAddr = new DeviceAddress();
                }

                override public String GetName()
                {
                    return "Looking For Token";
                }

                public override async Task Handle(UpdateStateMachine stateMachine)
                {
                    _MiTokenBLE = stateMachine.GetMiTokenBLE();

                    // Ensure to reset that a token has been found
                    _locateValidToken.Reset();

                    // Search for Tokens                
                    stateMachine.GetMiTokenBLE().NewPollResults += new BLE.V2_1.MiTokenBLE.NewPollResultsDelegate(PollResults);
                    stateMachine.GetMiTokenBLE().AddPollThreadInstance();

                    // Wait for a valid token to return or timeout to occur
                    bool isResult = await Task.Run<bool>(() =>
                    {
                        try
                        {
                            return _locateValidToken.Wait(5000, stateMachine._form._cancelSource.Token);
                        }
                        catch (OperationCanceledException)
                        {
                            return false;
                        }
                    });

                    // Stop searching for tokens
                    stateMachine.GetMiTokenBLE().RemovePollThreadInstance();
                    stateMachine.GetMiTokenBLE().NewPollResults -= PollResults;

                    // Change state depending on if a valid token was found
                    if (!_validDevAddr.IsEmpty() && isResult)
                    {
                        // Save valid device address
                        stateMachine._validDevAddr = _validDevAddr;

                        // Selects next state
                        stateMachine._form.SetStatus("Found: " + _validDevAddr.GetDisplayString());
                        stateMachine.Start();
                    }
                    // Check if cancellation has been requested
                    else if (stateMachine._form._cancelSource.IsCancellationRequested)
                    {
                        // Ends state machine
                        stateMachine._completeEvent.Set();
                    }
                    else
                    {
                        // Ends state machine
                        stateMachine._form.SetStatus("No valid tokens were found");
                        stateMachine._completeEvent.Set();
                    }
                }

                public void PollResults(BLE.V2_1.MiTokenBLE sender)
                {
                    // Cast to V2.2 object
                    sender = (BLE.V2_2.MiTokenBLE)sender;

                    // Exit early if a valid token has already been found
                    if (_locateValidToken.IsSet)
                        return;

                    // Process advertising data
                    int devFoundCount = sender.GetDeviceFoundCount();
                    for (int i = 0; i < devFoundCount; ++i)
                    {
                        // Retrieve device name
                        string devName = "";
                        byte[] nameArray;
                        if (_MiTokenBLE.GetDeviceMetaInfo(i, GapAdvertDataTypes.GAP_ADTYPE_LOCAL_NAME_COMPLETE, out nameArray) == BLE.Core.API_RET.BLE_API_SUCCESS)
                        {
                            devName = System.Text.Encoding.UTF8.GetString(nameArray);
                        }

                        // Ensure dealing with "MT-BOOT" devices
                        if (devName.CompareTo(DevNameFilterType.getTypeStr(DevNameFilterType.eType.MtBoot)) == 0)
                        {
                            // Get Device Address
                            DeviceAddress currentDevAddr = null;
                            {
                                BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                                _MiTokenBLE.GetDeviceInfo(i, ref devInfo);
                                currentDevAddr = new DeviceAddress(devInfo.macAddress);
                            }

                            // Check if device on ignore list
                            bool onIgnoreList = false;
                            foreach (DeviceAddress deviceAddr in _ignoreDevices)
                            {
                                if (deviceAddr == currentDevAddr)
                                {
                                    onIgnoreList = true;
                                    break;
                                }
                            }

                            // Exit early and update current device address if not on ignore list
                            if (!onIgnoreList)
                            {
                                _validDevAddr = currentDevAddr;
                                _locateValidToken.Set();
                                return;
                            }
                        }
                        else if (devName != "")
                        {
                            _ignoreDevices.Add(_validDevAddr);
                        }
                    }
                }
            }

            public class UpdateTokenState : UpdateState
            {
                public UpdateTokenState()
                {
                    Attempt = 0;
                }

                public const int MAX_FAST_MODE_ATTEMPTS = 3;
                private int _attemptCount = 0;

                public int Attempt
                {
                    get
                    {
                        return _attemptCount;
                    }

                    set
                    {
                        _attemptCount = value;
                    }
                }

                private int OadBlocksPerReqFastMode()
                {
                    // Safeguard to ensure value is valid
                    if (_attemptCount == 0)
                    {
                        return 1;
                    }

                    int oadBlocksPerReq = OadService.MAX_OAD_BLOCKS_PER_REQ_FAST_MODE - (_attemptCount - 1);

                    // Safeguard to ensure value is valid
                    if (oadBlocksPerReq <= 0)
                    {
                        return 1;
                    }

                    return oadBlocksPerReq;
                }

                override public String GetName()
                {
                    return "Updating Token";
                }

                public override async Task Handle(UpdateStateMachine stateMachine)
                {
                    // Start Session
                    stateMachine._form.SetStatus("Connecting");
                    using (var session = new FormProvTool.ConnectedSession(stateMachine.GetMiTokenBLE(), stateMachine._validDevAddr, stateMachine.GetFormProvTool().GetProcedureCompletedEvent(), stateMachine.GetFormProvTool().GetConnectionDisconnected()))
                    {
                        try
                        {
                            // Increment number of update attempts
                            Attempt++;

                            // Check device is connected
                            if (!await session.Open()) throw new Exception("");

                            // Update Connection Paramteres
                            stateMachine._form.SetStatus("Updating connection parameters");
                            if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, stateMachine.GetFormProvTool().GetConnectionUpdateResponseEvent(), stateMachine.GetFormProvTool().GetConnectionUpdateResponseSemaphore())) throw new Exception("");

                            // Retrieve target header
                            stateMachine._form.SetStatus("Retrieving target header");
                            if (!await Task.Run(() => stateMachine._oadService.RetrieveTargetImgHdr(session.Device))) throw new TimeoutException();

                            // Check Target header
                            ImgHdr targetImgHdr = stateMachine._oadService.getTargetImgHdr();
                            if (targetImgHdr == null) throw new TimeoutException();

                            // Check User ID
                            if (!targetImgHdr.isUidValid(stateMachine._form._newImgHdr))
                            {
                                stateMachine._form.SetStatus(String.Format("Update failed; Image ID did not match={0}", stateMachine._form._newImgHdr.getUidStr()));
                                stateMachine.Stop(String.Format("Failed - Image ID did not match={0}", stateMachine._form._newImgHdr.getUidStr()));
                                return;
                            }

                            // Ensure correct image
                            if (targetImgHdr.isImgA())
                            {
                                // Perform update based on number of attempts
                                stateMachine._form.SetStatus("Correct image type: " + targetImgHdr.getTypeStr());
                                IProgress<OadService.UiData> uiData = new Progress<OadService.UiData>(ud =>
                                {
                                    // Trottle updates
                                    string secondlLastLine = "";
                                    string newLine = String.Format("ETC[mm:ss]: {0:mm\\:ss} {1:n0}/{2:n0} bytes ({3}%)", ud.EstTimeRemaining, ud.CurrentBytes, ud.TotalBytes, ud.StatusPercent);
                                    if (stateMachine._form.richTextBoxStatus.Lines.Length >= 2)
                                    {
                                        secondlLastLine = stateMachine._form.richTextBoxStatus.Lines[stateMachine._form.richTextBoxStatus.Lines.Length - 2];
                                    }
                                    string[] s = secondlLastLine.Split(new char[] { '(', ')' });
                                    if (s.Length > 1)
                                    {
                                        string currPercentStr = s.ElementAt(1);
                                        if (currPercentStr != String.Format("{0}%", ud.StatusPercent))
                                        {
                                            stateMachine._form.SetStatus(newLine);
                                        }
                                    }
                                    else
                                    {
                                        stateMachine._form.SetStatus(newLine);
                                    }
                                });
                                bool oadResult = false;

                                // Select fast or normal mode                        
                                if (OadBlocksPerReqFastMode() == 1)
                                {
                                    // Use normal mode
                                    stateMachine._form.SetStatus("Performing update - Mode: Normal; Attempt: " + Attempt);
                                    oadResult = await Task.Run<bool>(() => stateMachine._oadService.PerformOAD(session.Device, session.ConnectionDisconnectedEvent, stateMachine._form._newImgHdr, stateMachine._form._newImgFileName, uiData, stateMachine._form._cancelSource.Token, false));
                                }
                                else
                                {
                                    // Use fast mode
                                    stateMachine._form.SetStatus("Performing update - Mode: Fast; Attempt: " + Attempt + "; OAD Blocks per Transfer: " + OadBlocksPerReqFastMode());
                                    oadResult = await Task.Run<bool>(() => stateMachine._oadService.PerformOAD(session.Device, session.ConnectionDisconnectedEvent, stateMachine._form._newImgHdr, stateMachine._form._newImgFileName, uiData, stateMachine._form._cancelSource.Token, true, OadBlocksPerReqFastMode()));
                                }

                                // Selects next state
                                if (oadResult)
                                {
                                    stateMachine._form.SetStatus("Update complete");
                                    stateMachine.State = stateMachine._verifyTokenState;
                                }
                                else
                                {
                                    // Check if connection has disconnected
                                    if (session.ConnectionDisconnectedEvent.IsSet)
                                    {
                                        stateMachine._form.SetStatus("Update failed; Token disconnected");
                                        stateMachine.Stop("Failed - Token disconnected");
                                    }
                                    // Check if cancellation
                                    else if (stateMachine._form._cancelSource.IsCancellationRequested)
                                    {
                                        stateMachine.Stop("Failed - Cancelled");
                                    }
                                    // Check if maximum attempts made
                                    else if (Attempt == MAX_FAST_MODE_ATTEMPTS)
                                    {
                                        stateMachine._form.SetStatus("Update failed; Maximum attempts reached");
                                        stateMachine.Stop("Failed - Maximum attempts reached");
                                    }
                                    else
                                    {
                                        stateMachine._form.SetStatus("Update failed");
                                        stateMachine.State = stateMachine._updateTokenState;
                                    }                                    
                                }
                            }
                            else
                            {
                                // Selects next state
                                stateMachine._form.SetStatus("Update failed; Invalid image type: " + targetImgHdr.getTypeStr());
                                stateMachine.Stop("Failed - Invalid image type");
                            }
                        }
                        catch (Exception e)
                        {
                            // Check if maximum attempts made
                            if (stateMachine._form._cancelSource.IsCancellationRequested)
                            {
                                stateMachine.Stop("Failed - Cancelled");
                            }
                            else if (Attempt == MAX_FAST_MODE_ATTEMPTS)
                            {
                                stateMachine._form.SetStatus("Update failed; Maximum attempts reached");
                                stateMachine.Stop("Failed - Maximum attempts reached");
                            }
                            else
                            {
                                stateMachine._form.SetStatus("Update failed");
                                stateMachine.State = stateMachine._updateTokenState;
                            }
                        }
                        finally
                        {
                            // Check if cancellation has been requested
                            if (stateMachine._form._cancelSource.IsCancellationRequested)
                            {
                                // Ends state machine
                                stateMachine._completeEvent.Set();
                            }
                        }
                    }
                }
            }

            public class VerifyTokenState : UpdateState
            {
                override public String GetName()
                {
                    return "Verify Token";
                }

                public override async Task Handle(UpdateStateMachine stateMachine)
                {
                    // Start Session
                    stateMachine._form.SetStatus("Connecting to: " + stateMachine._validDevAddr.GetDisplayString());
                    using (var session = new FormProvTool.ConnectedSession(stateMachine.GetMiTokenBLE(), stateMachine._validDevAddr, stateMachine.GetFormProvTool().GetProcedureCompletedEvent(), stateMachine.GetFormProvTool().GetConnectionDisconnected()))
                    {
                        try
                        {
                            // Check device is connected
                            if (!await session.Open())
                            {
                                // Assume verification passed as can no longer can see "MT-BOOT"
                                // Ends state machine
                                stateMachine._form.SetStatus("Verification Passed: Cannot detect");
                                stateMachine.Stop("Success");
                                return;
                            }

                            // Update Connection Paramteres
                            stateMachine._form.SetStatus("Updating connection parameters");
                            if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, stateMachine.GetFormProvTool().GetConnectionUpdateResponseEvent(), stateMachine.GetFormProvTool().GetConnectionUpdateResponseSemaphore()))
                            {
                                // Assume verification passed as can no longer can see "MT-BOOT"
                                // Ends state machine
                                stateMachine._form.SetStatus("Verification Passed: Cannot detect");
                                stateMachine.Stop("Success");
                                return;
                            }

                            // Retrieve target header
                            stateMachine._form.SetStatus("Retrieving target header");
                            if (!await Task.Run(() => stateMachine._oadService.RetrieveTargetImgHdr(session.Device)))
                            {
                                // Assume verification passed as can no longer can see "MT-BOOT"
                                // Ends state machine
                                stateMachine._form.SetStatus("Verification Passed: Cannot detect");
                                stateMachine.Stop("Success");
                                return;
                            }

                            // Check Target header
                            ImgHdr targetImgHdr = stateMachine._oadService.getTargetImgHdr();
                            if (targetImgHdr == null) throw new TimeoutException();

                            // Ensure correct image type has been updated
                            if (stateMachine._form._newImgHdr.isImgA() == targetImgHdr.isImgA())
                            {
                                // Verification passed as type is the same
                                stateMachine._form.SetStatus("Verification Passed: Current and image types are same");
                                stateMachine.Stop("Success");
                            }
                            else
                            {
                                // Verification failed as type is different

                                // Check if maximum attempts made
                                if (stateMachine._updateTokenState.Attempt == UpdateTokenState.MAX_FAST_MODE_ATTEMPTS)
                                {
                                    stateMachine._form.SetStatus("Verification Failed: Current and image types are different; Maximum attempts reached");
                                    stateMachine.Stop("Failed - Current and image types are different; Maximum attempts reached");
                                }
                                else
                                {
                                    stateMachine._form.SetStatus("Verification Failed: Current and image types are different; Retrying update");
                                    stateMachine.State = stateMachine._updateTokenState;
                                }
                            }
                        }
                        catch (Exception e)
                        {
                            // Selects next state
                            stateMachine._form.SetStatus(e.Message.Equals("") ? "Verification Failed: " : e.Message);
                            stateMachine.State = stateMachine._updateTokenState;
                        }
                        finally
                        {
                            // Check if cancellation has been requested
                            if (stateMachine._form._cancelSource.IsCancellationRequested)
                            {
                                // Ends state machine
                                stateMachine._completeEvent.Set();
                            }
                        }
                    }
                }
            }
        }

        private void FormBulkOAD_FormClosing(object sender, FormClosingEventArgs e)
        {
            buttonCancel_Click(null, null);
        }

        async private void buttonVerify_Click(object sender, EventArgs e)
        {
            if ((_fwRevStr == "") || (_hwRevStr == ""))
            {
                return;
            }

            if (DialogResult.OK != MessageBox.Show("Before pressing 'OK', please ensure all tokens for verification are advertising/discoverable.", "Verify Revision", MessageBoxButtons.OK, MessageBoxIcon.Information))
            {
                return;
            }

            buttonSelect.Enabled = false;
            buttonStart.Enabled = false;
            buttonCancel.Enabled = true;
            buttonVerify.Enabled = false;
            
            // Create new cancellation token source
            _cancelSource = new CancellationTokenSource();

            Stopwatch sw = new Stopwatch();
            sw.Start();

            SetStatus("Bulk Verify Started");
            using (VerifyStateMachine stateMachine = new VerifyStateMachine(this))
            {
                try
                {
                    bool isComplete = false;
                    while (!isComplete)
                    {
                        int waitHandleIndex = EventWaitHandle.WaitAny(stateMachine.WaitEventHandles());
                        switch (waitHandleIndex)
                        {
                            case 0:
                                sw.Stop();
                                SetStatus("Bulk Verify Complete: Total Time=" + sw.Elapsed);
                                isComplete = true;
                                break;
                            case 1:
                                stateMachine._newStateEvent.Reset();
                                SetStatus("State: " + stateMachine.State.GetName());
                                await stateMachine.Handle();
                                break;

                            default:
                                throw new Exception("Invalid Wait Handle Index");
                        }
                    }
                }
                catch (Exception exp)
                {
                    SetStatus("Exception: " + exp.Message);
                }

                // Update status with summary                                
                StringBuilder sb = new StringBuilder();
                sb.AppendLine();
                sb.AppendLine();
                sb.AppendLine("Summary:");
                sb.AppendLine();
                foreach (Tuple<DeviceAddress, String, TimeSpan> summary in stateMachine._summary)
                {
                    sb.AppendLine(" Device Address:\t" + summary.Item1.GetDisplayString());
                    sb.AppendLine(" Verify Status:\t" + summary.Item2);
                    sb.AppendLine(" Total Time:\t" + summary.Item3);
                    sb.AppendLine();
                }
                if (stateMachine._summary.Count > 0)
                {
                    SetStatus(sb.ToString());
                }
            }

            if ((_fwRevStr != "") && (_hwRevStr != ""))
            {
                buttonVerify.Enabled = true;
            }
            buttonSelect.Enabled = true;
            buttonStart.Enabled = true;
            buttonCancel.Enabled = false;
        }

        private static int ByteSearch(byte[] searchIn, List<byte[]> searchBytesList, int start = 0)
        {
            int found = -1;
            bool matched = false;
            foreach (byte[] searchBytes in searchBytesList)
            {
                //only look at this if we have a populated search array and search bytes with a sensible start            
                if (searchIn.Length > 0 && searchBytes.Length > 0 && start <= (searchIn.Length - searchBytes.Length) && searchIn.Length >= searchBytes.Length)
                {
                    //iterate through the array to be searched
                    for (int i = start; i <= searchIn.Length - searchBytes.Length; i++)
                    {
                        //if the start bytes match we will start comparing all other bytes
                        if (searchIn[i] == searchBytes[0])
                        {
                            if (searchIn.Length > 1)
                            {
                                //multiple bytes to be searched we have to compare byte by byte
                                matched = true;
                                for (int y = 1; y <= searchBytes.Length - 1; y++)
                                {
                                    if (searchIn[i + y] != searchBytes[y])
                                    {
                                        matched = false;
                                        break;
                                    }
                                }
                                //everything matched up
                                if (matched)
                                {
                                    found = i;
                                    break;
                                }

                            }
                            else
                            {
                                //search byte is only one bit nothing else to do
                                found = i;
                                break; //stop the loop
                            }

                        }
                    }
                }
            }
            return found;
        }

        class VerifyStateMachine : IDisposable
        {
            private VerifyState _state;
            public DeviceAddress _validDevAddr = new DeviceAddress();
            public List<Tuple<DeviceAddress, String, TimeSpan>> _summary = new List<Tuple<DeviceAddress, String, TimeSpan>>();

            public ManualResetEventSlim _completeEvent = new ManualResetEventSlim(false);
            public ManualResetEventSlim _newStateEvent = new ManualResetEventSlim(false);

            public LookingForTokenState _lookingForTokenState = null;
            public VerifyRevisionState _verifyRevisionState = null;

            public Stopwatch _verifyTime = new Stopwatch();

            public FormBulkOAD _form = null;

            public WaitHandle[] WaitEventHandles()
            {
                return new WaitHandle[]
                {
                    _completeEvent.WaitHandle,
                    _newStateEvent.WaitHandle
                };
            }

            public VerifyStateMachine(FormBulkOAD form)
            {
                _form = form;
                _lookingForTokenState = new LookingForTokenState(ConfigurationData.GetUseCase(_form._fwRevStr));
                _verifyRevisionState = new VerifyRevisionState();

                // Initial state
                this.State = _lookingForTokenState;
            }

            public void Dispose()
            {
                // Stop searching for tokens
                GetMiTokenBLE().RemovePollThreadInstance();
                GetMiTokenBLE().NewPollResults -= _lookingForTokenState.PollResults;
            }

            public VerifyState State
            {
                get { return _state; }
                set
                {
                    _state = value;
                    if (_state.GetType() == typeof(LookingForTokenState))
                    {
                        _lookingForTokenState.IgnoreCurrentDevice();
                    }
                    _newStateEvent.Set();
                }
            }

            public async Task Handle()
            {
                if (_form._cancelSource.IsCancellationRequested)
                {
                    // Ends state machine
                    _completeEvent.Set();
                    return;
                }
                await _state.Handle(this);
            }

            public BLE.V2_2.MiTokenBLE GetMiTokenBLE()
            {
                return _form._formProvTool.GetMiTokenBLE();
            }

            public FormProvTool GetFormProvTool()
            {
                return _form._formProvTool;
            }

            public void Start()
            {
                _verifyTime.Restart();
                State = _verifyRevisionState;
            }

            public void Stop(String resultStr)
            {
                _verifyTime.Stop();
                _form.SetStatus("Adding to ignore list: " + _validDevAddr.GetDisplayString());
                _summary.Add(new Tuple<DeviceAddress, String, TimeSpan>(_validDevAddr, resultStr, _verifyTime.Elapsed));
                State = _lookingForTokenState;
            }

            public abstract class VerifyState
            {
                abstract public String GetName();
                public virtual async Task Handle(VerifyStateMachine stateMachine) { }
            }

            public class LookingForTokenState : VerifyState
            {
                List<DeviceAddress> _ignoreDevices = new List<DeviceAddress>();
                ManualResetEventSlim _locateValidToken = new ManualResetEventSlim(false);
                DeviceAddress _validDevAddr = new DeviceAddress();
                ConfigurationData.eUseCase _useCaseToLookFor = ConfigurationData.eUseCase.eInvalid;
                BLE.V2_2.MiTokenBLE _MiTokenBLE = null;

                public LookingForTokenState(ConfigurationData.eUseCase useCaseToLookFor)
                {
                    _useCaseToLookFor = useCaseToLookFor;
                }

                public void IgnoreCurrentDevice()
                {
                    // Add current device to ignore lsit
                    if (!_validDevAddr.IsEmpty())
                    {
                        _ignoreDevices.Add(_validDevAddr);
                    }

                    // Reset current device
                    _validDevAddr = new DeviceAddress();
                }

                override public String GetName()
                {
                    return "Looking For Token";
                }

                public override async Task Handle(VerifyStateMachine stateMachine)
                {
                    _MiTokenBLE = stateMachine.GetMiTokenBLE();

                    // Ensure to reset that a token has been found
                    _locateValidToken.Reset();

                    // Search for Tokens                
                    stateMachine.GetMiTokenBLE().NewPollResults += new BLE.V2_1.MiTokenBLE.NewPollResultsDelegate(PollResults);
                    stateMachine.GetMiTokenBLE().AddPollThreadInstance();

                    // Wait for a valid token to return or timeout to occur
                    bool isResult = await Task.Run<bool>(() =>
                    {
                        try
                        {
                            return _locateValidToken.Wait(5000, stateMachine._form._cancelSource.Token);
                        }
                        catch (OperationCanceledException)
                        {
                            return false;
                        }
                    });

                    // Stop searching for tokens
                    stateMachine.GetMiTokenBLE().RemovePollThreadInstance();
                    stateMachine.GetMiTokenBLE().NewPollResults -= PollResults;

                    // Change state depending on if a valid token was found
                    if (!_validDevAddr.IsEmpty() && isResult)
                    {
                        // Save valid device address
                        stateMachine._validDevAddr = _validDevAddr;

                        // Selects next state
                        stateMachine._form.SetStatus("Found: " + _validDevAddr.GetDisplayString());
                        stateMachine.Start();
                    }
                    // Check if cancellation has been requested
                    else if (stateMachine._form._cancelSource.IsCancellationRequested)
                    {
                        // Ends state machine
                        stateMachine._completeEvent.Set();
                    }
                    else
                    {
                        // Ends state machine
                        stateMachine._form.SetStatus("No valid tokens were found");
                        stateMachine._completeEvent.Set();
                    }
                }

                public void PollResults(BLE.V2_1.MiTokenBLE sender)
                {
                    // Cast to V2.2 object
                    sender = (BLE.V2_2.MiTokenBLE)sender;

                    // Exit early if a valid token has already been found
                    if (_locateValidToken.IsSet)
                        return;

                    // Process advertising data
                    int devFoundCount = sender.GetDeviceFoundCount();
                    for (int i = 0; i < devFoundCount; ++i)
                    {
                        // Retrieve device name
                        string devName = "";
                        byte[] nameArray;
                        if (_MiTokenBLE.GetDeviceMetaInfo(i, GapAdvertDataTypes.GAP_ADTYPE_LOCAL_NAME_COMPLETE, out nameArray) == BLE.Core.API_RET.BLE_API_SUCCESS)
                        {
                            devName = System.Text.Encoding.UTF8.GetString(nameArray);
                        }

                        // Determine use case to look for 
                        DevNameFilterType.eType typeToLookFor = DevNameFilterType.eType.None;
                        switch (_useCaseToLookFor)
                        {
                            case ConfigurationData.eUseCase.eAuthBeacon:
                            case ConfigurationData.eUseCase.eBeacon:
                                typeToLookFor = DevNameFilterType.eType.MtBcn;
                                break;
                            case ConfigurationData.eUseCase.eBlu:
                                typeToLookFor = DevNameFilterType.eType.MtBlu;
                                break;
                            case ConfigurationData.eUseCase.eBoot:
                                typeToLookFor = DevNameFilterType.eType.MtBoot;
                                break;
                        }

                        // Ensure dealing with correct devices
                        if (devName.CompareTo(DevNameFilterType.getTypeStr(typeToLookFor)) == 0)
                        {
                            // Get Device Address
                            DeviceAddress currentDevAddr = null;
                            {
                                BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                                _MiTokenBLE.GetDeviceInfo(i, ref devInfo);
                                currentDevAddr = new DeviceAddress(devInfo.macAddress);
                            }

                            // Check if device on ignore list
                            bool onIgnoreList = false;
                            foreach (DeviceAddress deviceAddr in _ignoreDevices)
                            {
                                if (deviceAddr == currentDevAddr)
                                {
                                    onIgnoreList = true;
                                    break;
                                }
                            }

                            // Exit early and update current device address if not on ignore list
                            if (!onIgnoreList)
                            {
                                _validDevAddr = currentDevAddr;
                                _locateValidToken.Set();
                                return;
                            }
                        }
                        else if (devName != "")
                        {
                            _ignoreDevices.Add(_validDevAddr);
                        }
                    }
                }
            }

            public class VerifyRevisionState : VerifyState
            {
                override public String GetName()
                {
                    return "Verifying Revision";
                }

                public override async Task Handle(VerifyStateMachine stateMachine)
                {
                    // Start Session
                    stateMachine._form.SetStatus("Connecting");
                    using (var session = new FormProvTool.ConnectedSession(stateMachine.GetMiTokenBLE(), stateMachine._validDevAddr, stateMachine.GetFormProvTool().GetProcedureCompletedEvent(), stateMachine.GetFormProvTool().GetConnectionDisconnected()))
                    {
                        try
                        {
                            // Check device is connected
                            if (!await session.Open()) throw new Exception("");

                            // Update Connection Paramteres
                            stateMachine._form.SetStatus("Updating connection parameters");
                            if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, stateMachine.GetFormProvTool().GetConnectionUpdateResponseEvent(), stateMachine.GetFormProvTool().GetConnectionUpdateResponseSemaphore())) throw new Exception("");

                            // Get firmware revision
                            String fwRevStr = "";
                            bool isFwRevMatch = false;
                            {
                                var result = await BLE_AdvFunctions.ReadRequest(session.Device, DevInfoService.ATT_FWREV_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
                                if (result.Item1)
                                {
                                    fwRevStr = System.Text.Encoding.UTF8.GetString(result.Item2);
                                    isFwRevMatch = (fwRevStr == stateMachine._form._fwRevStr);
                                }
                            }

                            // Get hardware revision
                            String hwRevStr = "";
                            bool isHwRevMatch = false;
                            {
                                var result = await BLE_AdvFunctions.ReadRequest(session.Device, DevInfoService.ATT_HWREV_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
                                if (result.Item1)
                                {
                                    hwRevStr = System.Text.Encoding.UTF8.GetString(result.Item2);
                                    isHwRevMatch = (hwRevStr == stateMachine._form._hwRevStr);
                                }
                            }

                            // Process result
                            if (isFwRevMatch && isHwRevMatch)
                            {
                                stateMachine._form.SetStatus("Verify complete - Correct Revision");
                                stateMachine.Stop("Success");
                            }
                            else
                            {
                                stateMachine._form.SetStatus("Verify failed - Incorrect Revision");
                                if ((fwRevStr != "") && (fwRevStr != ""))
                                {
                                    stateMachine.Stop("Failed - Incorrect Revision - Firmware[" + fwRevStr + "] Hardware[" + hwRevStr + "]");
                                }
                                else
                                {
                                    stateMachine.Stop("Failed - Incorrect Revision");
                                }

                            }                            
                        }
                        catch (Exception e)
                        {
                            stateMachine._form.SetStatus("Verify failed - Processing Error");
                            stateMachine.Stop("Failed - Processing Error");
                        }
                        finally
                        {
                            // Check if cancellation has been requested
                            if (stateMachine._form._cancelSource.IsCancellationRequested)
                            {
                                // Ends state machine
                                stateMachine._completeEvent.Set();
                            }
                        }
                    }
                }
            }

        }
    }
}
