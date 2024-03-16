namespace BLE_Configuration_Tool
{
    partial class BLE_Configuration
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {            
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(BLE_Configuration));
            this.groupCOMPort = new System.Windows.Forms.GroupBox();
            this.labelCOMStatus = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.butTest = new System.Windows.Forms.Button();
            this.butRefreshCOM = new System.Windows.Forms.Button();
            this.comboCOM = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.butNext = new System.Windows.Forms.Button();
            this.butBack = new System.Windows.Forms.Button();
            this.groupDeviceFilter = new System.Windows.Forms.GroupBox();
            this.button2 = new System.Windows.Forms.Button();
            this.lstDevices = new System.Windows.Forms.ListBox();
            this.txtDeviceName = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.groupAPIServers = new System.Windows.Forms.GroupBox();
            this.butRemoveMiTokenAPIServer = new System.Windows.Forms.Button();
            this.butValidateMiTokenAPIServer = new System.Windows.Forms.Button();
            this.lstAPIServers = new System.Windows.Forms.ListBox();
            this.txtMiTokenAPIServer = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.groupLocationCalibration = new System.Windows.Forms.GroupBox();
            this.txtStageID = new System.Windows.Forms.Label();
            this.progStageProgress = new System.Windows.Forms.ProgressBar();
            this.groupConfigSummary = new System.Windows.Forms.GroupBox();
            this.butSaveSettings = new System.Windows.Forms.Button();
            this.butServiceStartStop = new System.Windows.Forms.Button();
            this.labPrimaryMiToken = new System.Windows.Forms.Label();
            this.labDeviceName = new System.Windows.Forms.Label();
            this.labCOMPort = new System.Windows.Forms.Label();
            this.labServiceStatus = new System.Windows.Forms.Label();
            this.butUpdateService = new System.Windows.Forms.Button();
            this.butDebug = new System.Windows.Forms.Button();
            this.groupBLELocalBond = new System.Windows.Forms.GroupBox();
            this.labelBLELocalBondDeviceSelected = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.butBondToDevice = new System.Windows.Forms.Button();
            this.buttonRemoveBonds = new System.Windows.Forms.Button();
            this.txtBondUser = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.labLatestRSSI = new System.Windows.Forms.Label();
            this.labUnlockRSSI = new System.Windows.Forms.Label();
            this.buttonContinue = new System.Windows.Forms.Button();
            this.labLockRSSI = new System.Windows.Forms.Label();
            this.txtAction = new System.Windows.Forms.Label();
            this.groupUserPreferences = new System.Windows.Forms.GroupBox();
            this.radioButtonUserMulti = new System.Windows.Forms.RadioButton();
            this.radioButtonUserSingle = new System.Windows.Forms.RadioButton();
            this.label4 = new System.Windows.Forms.Label();
            this.groupCOMPort.SuspendLayout();
            this.groupDeviceFilter.SuspendLayout();
            this.groupAPIServers.SuspendLayout();
            this.groupLocationCalibration.SuspendLayout();
            this.groupConfigSummary.SuspendLayout();
            this.groupBLELocalBond.SuspendLayout();
            this.groupUserPreferences.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupCOMPort
            // 
            this.groupCOMPort.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupCOMPort.Controls.Add(this.labelCOMStatus);
            this.groupCOMPort.Controls.Add(this.label6);
            this.groupCOMPort.Controls.Add(this.butTest);
            this.groupCOMPort.Controls.Add(this.butRefreshCOM);
            this.groupCOMPort.Controls.Add(this.comboCOM);
            this.groupCOMPort.Controls.Add(this.label1);
            this.groupCOMPort.Location = new System.Drawing.Point(4, 12);
            this.groupCOMPort.Name = "groupCOMPort";
            this.groupCOMPort.Size = new System.Drawing.Size(416, 535);
            this.groupCOMPort.TabIndex = 0;
            this.groupCOMPort.TabStop = false;
            this.groupCOMPort.Text = "Port Selection (Step 1 of 7)";
            this.groupCOMPort.Enter += new System.EventHandler(this.groupCOMPort_Enter);
            // 
            // labelCOMStatus
            // 
            this.labelCOMStatus.AutoSize = true;
            this.labelCOMStatus.Location = new System.Drawing.Point(146, 144);
            this.labelCOMStatus.Name = "labelCOMStatus";
            this.labelCOMStatus.Size = new System.Drawing.Size(49, 13);
            this.labelCOMStatus.TabIndex = 20;
            this.labelCOMStatus.Text = "<Status>";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(100, 144);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(40, 13);
            this.label6.TabIndex = 19;
            this.label6.Text = "Status:";
            // 
            // butTest
            // 
            this.butTest.Enabled = false;
            this.butTest.Location = new System.Drawing.Point(257, 139);
            this.butTest.Name = "butTest";
            this.butTest.Size = new System.Drawing.Size(102, 23);
            this.butTest.TabIndex = 0;
            this.butTest.Text = "Select";
            this.butTest.UseVisualStyleBackColor = true;
            this.butTest.Click += new System.EventHandler(this.butTest_Click);
            // 
            // butRefreshCOM
            // 
            this.butRefreshCOM.Location = new System.Drawing.Point(257, 106);
            this.butRefreshCOM.Name = "butRefreshCOM";
            this.butRefreshCOM.Size = new System.Drawing.Size(102, 23);
            this.butRefreshCOM.TabIndex = 18;
            this.butRefreshCOM.Text = "Refresh";
            this.butRefreshCOM.UseVisualStyleBackColor = true;
            this.butRefreshCOM.Click += new System.EventHandler(this.butRefreshCOM_Click);
            // 
            // comboCOM
            // 
            this.comboCOM.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboCOM.FormattingEnabled = true;
            this.comboCOM.Location = new System.Drawing.Point(146, 106);
            this.comboCOM.Name = "comboCOM";
            this.comboCOM.Size = new System.Drawing.Size(105, 21);
            this.comboCOM.TabIndex = 17;
            this.comboCOM.SelectedIndexChanged += new System.EventHandler(this.comboCOM_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(66, 111);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(74, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Bluegiga BLE:";
            // 
            // butNext
            // 
            this.butNext.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.butNext.Location = new System.Drawing.Point(345, 558);
            this.butNext.Name = "butNext";
            this.butNext.Size = new System.Drawing.Size(75, 35);
            this.butNext.TabIndex = 19;
            this.butNext.Text = "Next";
            this.butNext.UseVisualStyleBackColor = true;
            this.butNext.Click += new System.EventHandler(this.butNext_Click);
            // 
            // butBack
            // 
            this.butBack.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.butBack.Location = new System.Drawing.Point(4, 558);
            this.butBack.Name = "butBack";
            this.butBack.Size = new System.Drawing.Size(75, 35);
            this.butBack.TabIndex = 20;
            this.butBack.Text = "Back";
            this.butBack.UseVisualStyleBackColor = true;
            this.butBack.Click += new System.EventHandler(this.butBack_Click);
            // 
            // groupDeviceFilter
            // 
            this.groupDeviceFilter.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupDeviceFilter.Controls.Add(this.button2);
            this.groupDeviceFilter.Controls.Add(this.lstDevices);
            this.groupDeviceFilter.Controls.Add(this.txtDeviceName);
            this.groupDeviceFilter.Controls.Add(this.label2);
            this.groupDeviceFilter.Location = new System.Drawing.Point(4, 12);
            this.groupDeviceFilter.Name = "groupDeviceFilter";
            this.groupDeviceFilter.Size = new System.Drawing.Size(413, 535);
            this.groupDeviceFilter.TabIndex = 19;
            this.groupDeviceFilter.TabStop = false;
            this.groupDeviceFilter.Text = "Device Selection (Step 4 of 7)";
            this.groupDeviceFilter.Enter += new System.EventHandler(this.groupBox2_Enter);
            // 
            // button2
            // 
            this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.button2.Location = new System.Drawing.Point(316, 25);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(85, 23);
            this.button2.TabIndex = 23;
            this.button2.Text = "Update";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // lstDevices
            // 
            this.lstDevices.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstDevices.Font = new System.Drawing.Font("Courier New", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lstDevices.FormattingEnabled = true;
            this.lstDevices.ItemHeight = 15;
            this.lstDevices.Location = new System.Drawing.Point(14, 65);
            this.lstDevices.Name = "lstDevices";
            this.lstDevices.Size = new System.Drawing.Size(387, 454);
            this.lstDevices.TabIndex = 21;
            this.lstDevices.SelectedIndexChanged += new System.EventHandler(this.lstDevices_SelectedIndexChanged);
            // 
            // txtDeviceName
            // 
            this.txtDeviceName.Location = new System.Drawing.Point(78, 27);
            this.txtDeviceName.Name = "txtDeviceName";
            this.txtDeviceName.Size = new System.Drawing.Size(232, 20);
            this.txtDeviceName.TabIndex = 22;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(14, 30);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(63, 13);
            this.label2.TabIndex = 21;
            this.label2.Text = "Name Filter:";
            // 
            // groupAPIServers
            // 
            this.groupAPIServers.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupAPIServers.Controls.Add(this.butRemoveMiTokenAPIServer);
            this.groupAPIServers.Controls.Add(this.butValidateMiTokenAPIServer);
            this.groupAPIServers.Controls.Add(this.lstAPIServers);
            this.groupAPIServers.Controls.Add(this.txtMiTokenAPIServer);
            this.groupAPIServers.Controls.Add(this.label3);
            this.groupAPIServers.Location = new System.Drawing.Point(4, 12);
            this.groupAPIServers.Name = "groupAPIServers";
            this.groupAPIServers.Size = new System.Drawing.Size(413, 535);
            this.groupAPIServers.TabIndex = 24;
            this.groupAPIServers.TabStop = false;
            this.groupAPIServers.Text = "Mi-Token API Server (Step 3 of 7)";
            // 
            // butRemoveMiTokenAPIServer
            // 
            this.butRemoveMiTokenAPIServer.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.butRemoveMiTokenAPIServer.Location = new System.Drawing.Point(288, 495);
            this.butRemoveMiTokenAPIServer.Name = "butRemoveMiTokenAPIServer";
            this.butRemoveMiTokenAPIServer.Size = new System.Drawing.Size(113, 23);
            this.butRemoveMiTokenAPIServer.TabIndex = 24;
            this.butRemoveMiTokenAPIServer.Text = "Remove";
            this.butRemoveMiTokenAPIServer.UseVisualStyleBackColor = true;
            this.butRemoveMiTokenAPIServer.Click += new System.EventHandler(this.butRemoveMiTokenAPIServer_Click);
            // 
            // butValidateMiTokenAPIServer
            // 
            this.butValidateMiTokenAPIServer.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.butValidateMiTokenAPIServer.Location = new System.Drawing.Point(288, 25);
            this.butValidateMiTokenAPIServer.Name = "butValidateMiTokenAPIServer";
            this.butValidateMiTokenAPIServer.Size = new System.Drawing.Size(113, 23);
            this.butValidateMiTokenAPIServer.TabIndex = 23;
            this.butValidateMiTokenAPIServer.Text = "Validate and Add";
            this.butValidateMiTokenAPIServer.UseVisualStyleBackColor = true;
            this.butValidateMiTokenAPIServer.Click += new System.EventHandler(this.butValidateMiTokenAPIServer_Click);
            // 
            // lstAPIServers
            // 
            this.lstAPIServers.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstAPIServers.Font = new System.Drawing.Font("Courier New", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lstAPIServers.FormattingEnabled = true;
            this.lstAPIServers.ItemHeight = 15;
            this.lstAPIServers.Location = new System.Drawing.Point(14, 65);
            this.lstAPIServers.Name = "lstAPIServers";
            this.lstAPIServers.Size = new System.Drawing.Size(387, 424);
            this.lstAPIServers.TabIndex = 21;
            this.lstAPIServers.SelectedIndexChanged += new System.EventHandler(this.lstAPIServers_SelectedIndexChanged);
            // 
            // txtMiTokenAPIServer
            // 
            this.txtMiTokenAPIServer.Location = new System.Drawing.Point(70, 27);
            this.txtMiTokenAPIServer.Name = "txtMiTokenAPIServer";
            this.txtMiTokenAPIServer.Size = new System.Drawing.Size(212, 20);
            this.txtMiTokenAPIServer.TabIndex = 22;
            this.txtMiTokenAPIServer.TextChanged += new System.EventHandler(this.txtMiTokenAPIServer_TextChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(14, 30);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(48, 13);
            this.label3.TabIndex = 21;
            this.label3.Text = "Address:";
            // 
            // groupLocationCalibration
            // 
            this.groupLocationCalibration.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupLocationCalibration.Controls.Add(this.buttonContinue);
            this.groupLocationCalibration.Controls.Add(this.labLatestRSSI);
            this.groupLocationCalibration.Controls.Add(this.txtStageID);
            this.groupLocationCalibration.Controls.Add(this.labLockRSSI);
            this.groupLocationCalibration.Controls.Add(this.labUnlockRSSI);
            this.groupLocationCalibration.Controls.Add(this.progStageProgress);
            this.groupLocationCalibration.Controls.Add(this.txtAction);
            this.groupLocationCalibration.Location = new System.Drawing.Point(4, 12);
            this.groupLocationCalibration.Name = "groupLocationCalibration";
            this.groupLocationCalibration.Size = new System.Drawing.Size(413, 535);
            this.groupLocationCalibration.TabIndex = 25;
            this.groupLocationCalibration.TabStop = false;
            this.groupLocationCalibration.Text = "Location Calibration (Step 5 of 7)";
            // 
            // buttonContinue
            // 
            this.buttonContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonContinue.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F);
            this.buttonContinue.Location = new System.Drawing.Point(245, 215);
            this.buttonContinue.Name = "buttonContinue";
            this.buttonContinue.Size = new System.Drawing.Size(127, 46);
            this.buttonContinue.TabIndex = 5;
            this.buttonContinue.Text = "Continue";
            this.buttonContinue.UseVisualStyleBackColor = true;
            this.buttonContinue.Click += new System.EventHandler(this.buttonContinue_Click);
            // 
            // labLatestRSSI
            // 
            this.labLatestRSSI.AutoSize = true;
            this.labLatestRSSI.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F);
            this.labLatestRSSI.Location = new System.Drawing.Point(22, 257);
            this.labLatestRSSI.Name = "labLatestRSSI";
            this.labLatestRSSI.Size = new System.Drawing.Size(156, 25);
            this.labLatestRSSI.TabIndex = 9;
            this.labLatestRSSI.Text = "Latest Signal: -";
            // 
            // txtStageID
            // 
            this.txtStageID.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtStageID.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtStageID.Location = new System.Drawing.Point(89, 29);
            this.txtStageID.Name = "txtStageID";
            this.txtStageID.Size = new System.Drawing.Size(237, 26);
            this.txtStageID.TabIndex = 2;
            this.txtStageID.Text = "[Stage ID]";
            this.txtStageID.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // progStageProgress
            // 
            this.progStageProgress.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.progStageProgress.Location = new System.Drawing.Point(22, 160);
            this.progStageProgress.Name = "progStageProgress";
            this.progStageProgress.Size = new System.Drawing.Size(381, 23);
            this.progStageProgress.TabIndex = 6;
            // 
            // groupConfigSummary
            // 
            this.groupConfigSummary.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupConfigSummary.Controls.Add(this.butSaveSettings);
            this.groupConfigSummary.Controls.Add(this.butServiceStartStop);
            this.groupConfigSummary.Controls.Add(this.labPrimaryMiToken);
            this.groupConfigSummary.Controls.Add(this.labDeviceName);
            this.groupConfigSummary.Controls.Add(this.labCOMPort);
            this.groupConfigSummary.Controls.Add(this.labServiceStatus);
            this.groupConfigSummary.Controls.Add(this.butUpdateService);
            this.groupConfigSummary.Location = new System.Drawing.Point(4, 12);
            this.groupConfigSummary.Name = "groupConfigSummary";
            this.groupConfigSummary.Size = new System.Drawing.Size(413, 535);
            this.groupConfigSummary.TabIndex = 25;
            this.groupConfigSummary.TabStop = false;
            this.groupConfigSummary.Text = "Configuration Summary (Step 7 of 7)";
            // 
            // butSaveSettings
            // 
            this.butSaveSettings.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.butSaveSettings.Location = new System.Drawing.Point(299, 499);
            this.butSaveSettings.Name = "butSaveSettings";
            this.butSaveSettings.Size = new System.Drawing.Size(108, 24);
            this.butSaveSettings.TabIndex = 31;
            this.butSaveSettings.Text = "Save Settings";
            this.butSaveSettings.UseVisualStyleBackColor = true;
            this.butSaveSettings.Click += new System.EventHandler(this.butSaveSettings_Click);
            // 
            // butServiceStartStop
            // 
            this.butServiceStartStop.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.butServiceStartStop.Location = new System.Drawing.Point(299, 132);
            this.butServiceStartStop.Name = "butServiceStartStop";
            this.butServiceStartStop.Size = new System.Drawing.Size(109, 24);
            this.butServiceStartStop.TabIndex = 32;
            this.butServiceStartStop.Text = "Start Service";
            this.butServiceStartStop.UseVisualStyleBackColor = true;
            this.butServiceStartStop.Visible = false;
            this.butServiceStartStop.Click += new System.EventHandler(this.butServiceStartStop_Click);
            // 
            // labPrimaryMiToken
            // 
            this.labPrimaryMiToken.AutoSize = true;
            this.labPrimaryMiToken.Location = new System.Drawing.Point(14, 82);
            this.labPrimaryMiToken.Name = "labPrimaryMiToken";
            this.labPrimaryMiToken.Size = new System.Drawing.Size(143, 13);
            this.labPrimaryMiToken.TabIndex = 30;
            this.labPrimaryMiToken.Text = "Primary Mi-Token API Server";
            // 
            // labDeviceName
            // 
            this.labDeviceName.AutoSize = true;
            this.labDeviceName.Location = new System.Drawing.Point(14, 56);
            this.labDeviceName.Name = "labDeviceName";
            this.labDeviceName.Size = new System.Drawing.Size(72, 13);
            this.labDeviceName.TabIndex = 29;
            this.labDeviceName.Text = "Device Name";
            // 
            // labCOMPort
            // 
            this.labCOMPort.AutoSize = true;
            this.labCOMPort.Location = new System.Drawing.Point(14, 30);
            this.labCOMPort.Name = "labCOMPort";
            this.labCOMPort.Size = new System.Drawing.Size(53, 13);
            this.labCOMPort.TabIndex = 28;
            this.labCOMPort.Text = "COM Port";
            // 
            // labServiceStatus
            // 
            this.labServiceStatus.AutoSize = true;
            this.labServiceStatus.Location = new System.Drawing.Point(14, 108);
            this.labServiceStatus.Name = "labServiceStatus";
            this.labServiceStatus.Size = new System.Drawing.Size(76, 13);
            this.labServiceStatus.TabIndex = 27;
            this.labServiceStatus.Text = "Service Status";
            // 
            // butUpdateService
            // 
            this.butUpdateService.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.butUpdateService.Location = new System.Drawing.Point(299, 102);
            this.butUpdateService.Name = "butUpdateService";
            this.butUpdateService.Size = new System.Drawing.Size(109, 24);
            this.butUpdateService.TabIndex = 26;
            this.butUpdateService.Text = "Update Service";
            this.butUpdateService.UseVisualStyleBackColor = true;
            this.butUpdateService.Click += new System.EventHandler(this.butUpdateService_Click);
            // 
            // butDebug
            // 
            this.butDebug.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.butDebug.Location = new System.Drawing.Point(180, 558);
            this.butDebug.Name = "butDebug";
            this.butDebug.Size = new System.Drawing.Size(75, 35);
            this.butDebug.TabIndex = 26;
            this.butDebug.Text = "Debug";
            this.butDebug.UseVisualStyleBackColor = true;
            this.butDebug.Visible = false;
            this.butDebug.Click += new System.EventHandler(this.butDebug_Click);
            // 
            // groupBLELocalBond
            // 
            this.groupBLELocalBond.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBLELocalBond.Controls.Add(this.labelBLELocalBondDeviceSelected);
            this.groupBLELocalBond.Controls.Add(this.label7);
            this.groupBLELocalBond.Controls.Add(this.butBondToDevice);
            this.groupBLELocalBond.Controls.Add(this.buttonRemoveBonds);
            this.groupBLELocalBond.Controls.Add(this.txtBondUser);
            this.groupBLELocalBond.Controls.Add(this.label5);
            this.groupBLELocalBond.Location = new System.Drawing.Point(4, 12);
            this.groupBLELocalBond.Name = "groupBLELocalBond";
            this.groupBLELocalBond.Size = new System.Drawing.Size(416, 535);
            this.groupBLELocalBond.TabIndex = 26;
            this.groupBLELocalBond.TabStop = false;
            this.groupBLELocalBond.Text = "Local Bond Configuration (Step 6 of 7)";
            this.groupBLELocalBond.Visible = false;
            // 
            // labelBLELocalBondDeviceSelected
            // 
            this.labelBLELocalBondDeviceSelected.AutoSize = true;
            this.labelBLELocalBondDeviceSelected.Location = new System.Drawing.Point(114, 30);
            this.labelBLELocalBondDeviceSelected.Name = "labelBLELocalBondDeviceSelected";
            this.labelBLELocalBondDeviceSelected.Size = new System.Drawing.Size(106, 13);
            this.labelBLELocalBondDeviceSelected.TabIndex = 7;
            this.labelBLELocalBondDeviceSelected.Text = "XX:XX:XX:XX:XX:XX";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(14, 30);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(89, 13);
            this.label7.TabIndex = 6;
            this.label7.Text = "Device Selected:";
            // 
            // butBondToDevice
            // 
            this.butBondToDevice.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.butBondToDevice.Location = new System.Drawing.Point(281, 50);
            this.butBondToDevice.Name = "butBondToDevice";
            this.butBondToDevice.Size = new System.Drawing.Size(123, 23);
            this.butBondToDevice.TabIndex = 3;
            this.butBondToDevice.Text = "Create Local Bond";
            this.butBondToDevice.UseVisualStyleBackColor = true;
            this.butBondToDevice.Click += new System.EventHandler(this.butBondToDevice_Click);
            // 
            // buttonRemoveBonds
            // 
            this.buttonRemoveBonds.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonRemoveBonds.Location = new System.Drawing.Point(281, 79);
            this.buttonRemoveBonds.Name = "buttonRemoveBonds";
            this.buttonRemoveBonds.Size = new System.Drawing.Size(123, 23);
            this.buttonRemoveBonds.TabIndex = 2;
            this.buttonRemoveBonds.Text = "Remove All Bonds";
            this.buttonRemoveBonds.UseVisualStyleBackColor = true;
            this.buttonRemoveBonds.Click += new System.EventHandler(this.buttonRemoveBonds_Click);
            // 
            // txtBondUser
            // 
            this.txtBondUser.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtBondUser.Location = new System.Drawing.Point(117, 53);
            this.txtBondUser.Name = "txtBondUser";
            this.txtBondUser.Size = new System.Drawing.Size(154, 20);
            this.txtBondUser.TabIndex = 1;
            this.txtBondUser.TextChanged += new System.EventHandler(this.txtBondUser_TextChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(14, 56);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(92, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "Local Bond User :";
            // 
            // labLatestRSSI
            // 
            this.labLatestRSSI.AutoSize = true;
            this.labLatestRSSI.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F);
            this.labLatestRSSI.Location = new System.Drawing.Point(22, 257);
            this.labLatestRSSI.Name = "labLatestRSSI";
            this.labLatestRSSI.Size = new System.Drawing.Size(156, 25);
            this.labLatestRSSI.TabIndex = 9;
            this.labLatestRSSI.Text = "Latest Signal: -";
            // 
            // labUnlockRSSI
            // 
            this.labUnlockRSSI.AutoSize = true;
            this.labUnlockRSSI.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F);
            this.labUnlockRSSI.Location = new System.Drawing.Point(26, 195);
            this.labUnlockRSSI.Name = "labUnlockRSSI";
            this.labUnlockRSSI.Size = new System.Drawing.Size(150, 25);
            this.labUnlockRSSI.TabIndex = 7;
            this.labUnlockRSSI.Text = "Login Signal: -";
            // 
            // buttonContinue
            // 
            this.buttonContinue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonContinue.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F);
            this.buttonContinue.Location = new System.Drawing.Point(245, 215);
            this.buttonContinue.Name = "buttonContinue";
            this.buttonContinue.Size = new System.Drawing.Size(127, 46);
            this.buttonContinue.TabIndex = 5;
            this.buttonContinue.Text = "Continue";
            this.buttonContinue.UseVisualStyleBackColor = true;
            this.buttonContinue.Click += new System.EventHandler(this.buttonContinue_Click);
            // 
            // labLockRSSI
            // 
            this.labLockRSSI.AutoSize = true;
            this.labLockRSSI.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F);
            this.labLockRSSI.Location = new System.Drawing.Point(34, 226);
            this.labLockRSSI.Name = "labLockRSSI";
            this.labLockRSSI.Size = new System.Drawing.Size(143, 25);
            this.labLockRSSI.TabIndex = 8;
            this.labLockRSSI.Text = "Lock Signal: -";
            // 
            // txtAction
            // 
            this.txtAction.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtAction.BackColor = System.Drawing.SystemColors.Control;
            this.txtAction.Font = new System.Drawing.Font("Microsoft Sans Serif", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtAction.Location = new System.Drawing.Point(17, 72);
            this.txtAction.Name = "txtAction";
            this.txtAction.Size = new System.Drawing.Size(384, 82);
            this.txtAction.TabIndex = 4;
            this.txtAction.Text = "[Requested Action] (Line 1)\r\n(Line 2)\r\n(Line 3)\r\n";
            // 
            // BLE_Configuration
            // 
            // groupUserPreferences
            // 
            this.groupUserPreferences.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupUserPreferences.Controls.Add(this.radioButtonUserMulti);
            this.groupUserPreferences.Controls.Add(this.radioButtonUserSingle);
            this.groupUserPreferences.Controls.Add(this.label4);
            this.groupUserPreferences.Location = new System.Drawing.Point(4, 12);
            this.groupUserPreferences.Name = "groupUserPreferences";
            this.groupUserPreferences.Size = new System.Drawing.Size(416, 540);
            this.groupUserPreferences.TabIndex = 27;
            this.groupUserPreferences.TabStop = false;
            this.groupUserPreferences.Text = "Preferences (Step 2 of 7)";
            // 
            // radioButtonUserMulti
            // 
            this.radioButtonUserMulti.AutoSize = true;
            this.radioButtonUserMulti.Enabled = false;
            this.radioButtonUserMulti.Location = new System.Drawing.Point(157, 92);
            this.radioButtonUserMulti.Name = "radioButtonUserMulti";
            this.radioButtonUserMulti.Size = new System.Drawing.Size(47, 17);
            this.radioButtonUserMulti.TabIndex = 2;
            this.radioButtonUserMulti.TabStop = true;
            this.radioButtonUserMulti.Text = "Multi";
            this.radioButtonUserMulti.UseVisualStyleBackColor = true;
            this.radioButtonUserMulti.Click += new System.EventHandler(this.radioButtonUserMulti_Click);
            // 
            // radioButtonUserSingle
            // 
            this.radioButtonUserSingle.AutoSize = true;
            this.radioButtonUserSingle.Location = new System.Drawing.Point(157, 68);
            this.radioButtonUserSingle.Name = "radioButtonUserSingle";
            this.radioButtonUserSingle.Size = new System.Drawing.Size(54, 17);
            this.radioButtonUserSingle.TabIndex = 1;
            this.radioButtonUserSingle.TabStop = true;
            this.radioButtonUserSingle.Text = "Single";
            this.radioButtonUserSingle.UseVisualStyleBackColor = true;
            this.radioButtonUserSingle.Click += new System.EventHandler(this.radioButtonUserSingle_Click);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(154, 49);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(117, 13);
            this.label4.TabIndex = 0;
            this.label4.Text = "Select user preference:";
            // 
            // BLE_Configuration
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(424, 600);
            this.Controls.Add(this.groupUserPreferences);
            this.Controls.Add(this.butDebug);
            this.Controls.Add(this.butNext);
            this.Controls.Add(this.butBack);
            this.Controls.Add(this.groupCOMPort);
            this.Controls.Add(this.groupBLELocalBond);
            this.Controls.Add(this.groupLocationCalibration);
            this.Controls.Add(this.groupConfigSummary);
            this.Controls.Add(this.groupAPIServers);
            this.Controls.Add(this.groupDeviceFilter);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "BLE_Configuration";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "BLE Configuration Tool";
            this.TopMost = true;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.BLE_Configuration_FormClosing);
            this.Load += new System.EventHandler(this.BLE_Configuration_Load);
            this.groupCOMPort.ResumeLayout(false);
            this.groupCOMPort.PerformLayout();
            this.groupDeviceFilter.ResumeLayout(false);
            this.groupDeviceFilter.PerformLayout();
            this.groupAPIServers.ResumeLayout(false);
            this.groupAPIServers.PerformLayout();
            this.groupLocationCalibration.ResumeLayout(false);
            this.groupLocationCalibration.PerformLayout();
            this.groupConfigSummary.ResumeLayout(false);
            this.groupConfigSummary.PerformLayout();
            this.groupBLELocalBond.ResumeLayout(false);
            this.groupBLELocalBond.PerformLayout();
            this.groupUserPreferences.ResumeLayout(false);
            this.groupUserPreferences.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupCOMPort;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupDeviceFilter;
        private System.Windows.Forms.Button butTest;
        private System.Windows.Forms.Button butNext;
        private System.Windows.Forms.Button butBack;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.ListBox lstDevices;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox groupAPIServers;
        private System.Windows.Forms.Button butRemoveMiTokenAPIServer;
        private System.Windows.Forms.Button butValidateMiTokenAPIServer;
        private System.Windows.Forms.TextBox txtMiTokenAPIServer;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.GroupBox groupLocationCalibration;
        private System.Windows.Forms.GroupBox groupConfigSummary;
        public System.Windows.Forms.ComboBox comboCOM;
        public System.Windows.Forms.Button butRefreshCOM;
        private System.Windows.Forms.Button butUpdateService;
        private System.Windows.Forms.Button butSaveSettings;
        public System.Windows.Forms.Label labPrimaryMiToken;
        public System.Windows.Forms.Label labDeviceName;
        public System.Windows.Forms.Label labCOMPort;
        public System.Windows.Forms.Label labServiceStatus;
        public System.Windows.Forms.ListBox lstAPIServers;
        public System.Windows.Forms.TextBox txtDeviceName;
        public System.Windows.Forms.Button butServiceStartStop;
        private System.Windows.Forms.Button butDebug;
        private System.Windows.Forms.GroupBox groupBLELocalBond;
        private System.Windows.Forms.Button butBondToDevice;
        private System.Windows.Forms.Button buttonRemoveBonds;
        private System.Windows.Forms.TextBox txtBondUser;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ProgressBar progStageProgress;
        private System.Windows.Forms.Label txtStageID;
        private System.Windows.Forms.Label labelCOMStatus;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label labelBLELocalBondDeviceSelected;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button buttonContinue;
        private System.Windows.Forms.Label labLatestRSSI;
        private System.Windows.Forms.Label labLockRSSI;
        private System.Windows.Forms.Label labUnlockRSSI;
        private System.Windows.Forms.Label txtAction;
    }
        private System.Windows.Forms.GroupBox groupUserPreferences;
        private System.Windows.Forms.RadioButton radioButtonUserMulti;
        private System.Windows.Forms.RadioButton radioButtonUserSingle;
        private System.Windows.Forms.Label label4;
    }
}