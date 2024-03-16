namespace API_Autoconfig
{
    partial class ConfigControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBoxRadiusServer = new System.Windows.Forms.GroupBox();
            this.button3 = new System.Windows.Forms.Button();
            this.btnDeleteRadius = new System.Windows.Forms.Button();
            this.btnEditRadius = new System.Windows.Forms.Button();
            this.btnAddRadius = new System.Windows.Forms.Button();
            this.dataGridViewRADIUSServers = new System.Windows.Forms.DataGridView();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.groupBypass = new System.Windows.Forms.GroupBox();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.radFilterAll = new System.Windows.Forms.RadioButton();
            this.radFilterMiToken = new System.Windows.Forms.RadioButton();
            this.radFilterDefault = new System.Windows.Forms.RadioButton();
            this.radFilterNothing = new System.Windows.Forms.RadioButton();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.checkXPOTP = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.radConBoth = new System.Windows.Forms.RadioButton();
            this.radConRemote = new System.Windows.Forms.RadioButton();
            this.radConLocal = new System.Windows.Forms.RadioButton();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.checkCacheDumps = new System.Windows.Forms.CheckBox();
            this.checkSensitiveLogging = new System.Windows.Forms.CheckBox();
            this.checkVerboseLogging = new System.Windows.Forms.CheckBox();
            this.checkTraceLogging = new System.Windows.Forms.CheckBox();
            this.checkDynamicPassword = new System.Windows.Forms.CheckBox();
            this.checkUseHTTP = new System.Windows.Forms.CheckBox();
            this.txtDefaultDomain = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.checkCredInSafeMode = new System.Windows.Forms.CheckBox();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.groupTokenCacheEnabled = new System.Windows.Forms.GroupBox();
            this.groupBox7 = new System.Windows.Forms.GroupBox();
            this.labStatusText = new System.Windows.Forms.Label();
            this.labStatus = new System.Windows.Forms.Label();
            this.txtClientRefresh = new System.Windows.Forms.TextBox();
            this.labClientRefresh = new System.Windows.Forms.Label();
            this.txtPollPeriod = new System.Windows.Forms.TextBox();
            this.txtServerAddress = new System.Windows.Forms.TextBox();
            this.txtCustomerName = new System.Windows.Forms.TextBox();
            this.labPollPeriod = new System.Windows.Forms.Label();
            this.labServerAddress = new System.Windows.Forms.Label();
            this.labCustomerName = new System.Windows.Forms.Label();
            this.txtHOTPCount = new System.Windows.Forms.TextBox();
            this.txtTOTPCount = new System.Windows.Forms.TextBox();
            this.labHOTPCount = new System.Windows.Forms.Label();
            this.labTOTPCount = new System.Windows.Forms.Label();
            this.checkEnableCaching = new System.Windows.Forms.CheckBox();
            this.groupTokenCacheDisable = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.checkHideLastUsername = new System.Windows.Forms.CheckBox();
            this.tabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBoxRadiusServer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewRADIUSServers)).BeginInit();
            this.tabPage2.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.tabPage3.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.groupTokenCacheEnabled.SuspendLayout();
            this.groupBox7.SuspendLayout();
            this.groupTokenCacheDisable.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl
            // 
            this.tabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl.Controls.Add(this.tabPage1);
            this.tabControl.Controls.Add(this.tabPage2);
            this.tabControl.Controls.Add(this.tabPage4);
            this.tabControl.Controls.Add(this.tabPage3);
            this.tabControl.Controls.Add(this.tabPage5);
            this.tabControl.Location = new System.Drawing.Point(0, 0);
            this.tabControl.Margin = new System.Windows.Forms.Padding(2);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(600, 551);
            this.tabControl.TabIndex = 6;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.groupBoxRadiusServer);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Margin = new System.Windows.Forms.Padding(2);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(2);
            this.tabPage1.Size = new System.Drawing.Size(592, 525);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Authentication";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // groupBoxRadiusServer
            // 
            this.groupBoxRadiusServer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBoxRadiusServer.Controls.Add(this.button3);
            this.groupBoxRadiusServer.Controls.Add(this.btnDeleteRadius);
            this.groupBoxRadiusServer.Controls.Add(this.btnEditRadius);
            this.groupBoxRadiusServer.Controls.Add(this.btnAddRadius);
            this.groupBoxRadiusServer.Controls.Add(this.dataGridViewRADIUSServers);
            this.groupBoxRadiusServer.Location = new System.Drawing.Point(3, 3);
            this.groupBoxRadiusServer.Margin = new System.Windows.Forms.Padding(2);
            this.groupBoxRadiusServer.Name = "groupBoxRadiusServer";
            this.groupBoxRadiusServer.Padding = new System.Windows.Forms.Padding(2);
            this.groupBoxRadiusServer.Size = new System.Drawing.Size(585, 516);
            this.groupBoxRadiusServer.TabIndex = 0;
            this.groupBoxRadiusServer.TabStop = false;
            this.groupBoxRadiusServer.Text = "API Servers";
            this.groupBoxRadiusServer.Enter += new System.EventHandler(this.groupBoxRadiusServer_Enter);
            // 
            // button3
            // 
            this.button3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.button3.Location = new System.Drawing.Point(200, 490);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(59, 21);
            this.button3.TabIndex = 8;
            this.button3.Text = "Test";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // btnDeleteRadius
            // 
            this.btnDeleteRadius.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnDeleteRadius.Location = new System.Drawing.Point(135, 490);
            this.btnDeleteRadius.Name = "btnDeleteRadius";
            this.btnDeleteRadius.Size = new System.Drawing.Size(59, 21);
            this.btnDeleteRadius.TabIndex = 7;
            this.btnDeleteRadius.Text = "Delete";
            this.btnDeleteRadius.UseVisualStyleBackColor = true;
            this.btnDeleteRadius.Click += new System.EventHandler(this.btnDeleteRadius_Click);
            // 
            // btnEditRadius
            // 
            this.btnEditRadius.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnEditRadius.Location = new System.Drawing.Point(70, 490);
            this.btnEditRadius.Name = "btnEditRadius";
            this.btnEditRadius.Size = new System.Drawing.Size(59, 21);
            this.btnEditRadius.TabIndex = 6;
            this.btnEditRadius.Text = "Edit";
            this.btnEditRadius.UseVisualStyleBackColor = true;
            this.btnEditRadius.Click += new System.EventHandler(this.btnEditRadius_Click);
            // 
            // btnAddRadius
            // 
            this.btnAddRadius.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnAddRadius.Location = new System.Drawing.Point(5, 490);
            this.btnAddRadius.Name = "btnAddRadius";
            this.btnAddRadius.Size = new System.Drawing.Size(59, 21);
            this.btnAddRadius.TabIndex = 4;
            this.btnAddRadius.Text = "Add";
            this.btnAddRadius.UseVisualStyleBackColor = true;
            this.btnAddRadius.Click += new System.EventHandler(this.btnAddRadius_Click);
            // 
            // dataGridViewRADIUSServers
            // 
            this.dataGridViewRADIUSServers.AllowUserToAddRows = false;
            this.dataGridViewRADIUSServers.AllowUserToDeleteRows = false;
            this.dataGridViewRADIUSServers.AllowUserToOrderColumns = true;
            this.dataGridViewRADIUSServers.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewRADIUSServers.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            dataGridViewCellStyle1.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle1.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle1.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle1.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle1.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle1.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewRADIUSServers.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle1;
            this.dataGridViewRADIUSServers.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle2.BackColor = System.Drawing.SystemColors.Window;
            dataGridViewCellStyle2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle2.ForeColor = System.Drawing.SystemColors.ControlText;
            dataGridViewCellStyle2.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle2.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle2.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridViewRADIUSServers.DefaultCellStyle = dataGridViewCellStyle2;
            this.dataGridViewRADIUSServers.Location = new System.Drawing.Point(2, 15);
            this.dataGridViewRADIUSServers.Margin = new System.Windows.Forms.Padding(2);
            this.dataGridViewRADIUSServers.MultiSelect = false;
            this.dataGridViewRADIUSServers.Name = "dataGridViewRADIUSServers";
            this.dataGridViewRADIUSServers.ReadOnly = true;
            dataGridViewCellStyle3.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle3.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle3.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle3.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle3.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle3.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridViewRADIUSServers.RowHeadersDefaultCellStyle = dataGridViewCellStyle3;
            this.dataGridViewRADIUSServers.RowHeadersVisible = false;
            this.dataGridViewRADIUSServers.RowTemplate.Height = 24;
            this.dataGridViewRADIUSServers.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewRADIUSServers.Size = new System.Drawing.Size(579, 470);
            this.dataGridViewRADIUSServers.TabIndex = 0;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.groupBypass);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Size = new System.Drawing.Size(592, 525);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Bypass";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // groupBypass
            // 
            this.groupBypass.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBypass.Location = new System.Drawing.Point(3, 3);
            this.groupBypass.Name = "groupBypass";
            this.groupBypass.Size = new System.Drawing.Size(578, 519);
            this.groupBypass.TabIndex = 0;
            this.groupBypass.TabStop = false;
            this.groupBypass.Text = "Bypass";
            this.groupBypass.Enter += new System.EventHandler(this.groupBypass_Enter);
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.groupBox4);
            this.tabPage4.Controls.Add(this.groupBox3);
            this.tabPage4.Controls.Add(this.groupBox2);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Size = new System.Drawing.Size(592, 525);
            this.tabPage4.TabIndex = 3;
            this.tabPage4.Text = "Filter Modes";
            this.tabPage4.UseVisualStyleBackColor = true;
            this.tabPage4.Click += new System.EventHandler(this.tabPage4_Click);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.groupBox5);
            this.groupBox4.Controls.Add(this.groupBox1);
            this.groupBox4.Location = new System.Drawing.Point(120, 3);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(465, 151);
            this.groupBox4.TabIndex = 8;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Filter Settings";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.radFilterAll);
            this.groupBox5.Controls.Add(this.radFilterMiToken);
            this.groupBox5.Controls.Add(this.radFilterDefault);
            this.groupBox5.Controls.Add(this.radFilterNothing);
            this.groupBox5.Location = new System.Drawing.Point(6, 19);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(453, 126);
            this.groupBox5.TabIndex = 8;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Windows Vista / 7 / 2008";
            // 
            // radFilterAll
            // 
            this.radFilterAll.AutoSize = true;
            this.radFilterAll.Location = new System.Drawing.Point(14, 88);
            this.radFilterAll.Name = "radFilterAll";
            this.radFilterAll.Size = new System.Drawing.Size(252, 17);
            this.radFilterAll.TabIndex = 7;
            this.radFilterAll.TabStop = true;
            this.radFilterAll.Text = "Disable all but Mi-Token 2 Factor Authentication";
            this.radFilterAll.UseVisualStyleBackColor = true;
            this.radFilterAll.CheckedChanged += new System.EventHandler(this.radFilter_CheckedChanged);
            // 
            // radFilterMiToken
            // 
            this.radFilterMiToken.AutoSize = true;
            this.radFilterMiToken.Location = new System.Drawing.Point(14, 65);
            this.radFilterMiToken.Name = "radFilterMiToken";
            this.radFilterMiToken.Size = new System.Drawing.Size(221, 17);
            this.radFilterMiToken.TabIndex = 6;
            this.radFilterMiToken.TabStop = true;
            this.radFilterMiToken.Text = "Disable Mi-Token 2 Factor Authentication";
            this.radFilterMiToken.UseVisualStyleBackColor = true;
            this.radFilterMiToken.CheckedChanged += new System.EventHandler(this.radFilter_CheckedChanged);
            // 
            // radFilterDefault
            // 
            this.radFilterDefault.AutoSize = true;
            this.radFilterDefault.Location = new System.Drawing.Point(14, 42);
            this.radFilterDefault.Name = "radFilterDefault";
            this.radFilterDefault.Size = new System.Drawing.Size(177, 17);
            this.radFilterDefault.TabIndex = 5;
            this.radFilterDefault.TabStop = true;
            this.radFilterDefault.Text = "Disable Default Windows Logon";
            this.radFilterDefault.UseVisualStyleBackColor = true;
            this.radFilterDefault.CheckedChanged += new System.EventHandler(this.radFilter_CheckedChanged);
            // 
            // radFilterNothing
            // 
            this.radFilterNothing.AutoSize = true;
            this.radFilterNothing.Location = new System.Drawing.Point(14, 19);
            this.radFilterNothing.Name = "radFilterNothing";
            this.radFilterNothing.Size = new System.Drawing.Size(100, 17);
            this.radFilterNothing.TabIndex = 4;
            this.radFilterNothing.TabStop = true;
            this.radFilterNothing.Text = "Disable Nothing";
            this.radFilterNothing.UseVisualStyleBackColor = true;
            this.radFilterNothing.CheckedChanged += new System.EventHandler(this.radFilter_CheckedChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.checkXPOTP);
            this.groupBox1.Location = new System.Drawing.Point(314, 19);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(143, 126);
            this.groupBox1.TabIndex = 7;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Windows XP";
            this.groupBox1.Visible = false;
            // 
            // checkXPOTP
            // 
            this.checkXPOTP.AutoSize = true;
            this.checkXPOTP.Location = new System.Drawing.Point(6, 19);
            this.checkXPOTP.Name = "checkXPOTP";
            this.checkXPOTP.Size = new System.Drawing.Size(136, 17);
            this.checkXPOTP.TabIndex = 7;
            this.checkXPOTP.Text = "Require Mi-Token OTP";
            this.checkXPOTP.ThreeState = true;
            this.checkXPOTP.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.radConBoth);
            this.groupBox3.Controls.Add(this.radConRemote);
            this.groupBox3.Controls.Add(this.radConLocal);
            this.groupBox3.Location = new System.Drawing.Point(3, 3);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(111, 151);
            this.groupBox3.TabIndex = 7;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Connection Type";
            // 
            // radConBoth
            // 
            this.radConBoth.AutoSize = true;
            this.radConBoth.Location = new System.Drawing.Point(6, 65);
            this.radConBoth.Name = "radConBoth";
            this.radConBoth.Size = new System.Drawing.Size(47, 17);
            this.radConBoth.TabIndex = 6;
            this.radConBoth.TabStop = true;
            this.radConBoth.Text = "Both";
            this.radConBoth.UseVisualStyleBackColor = true;
            this.radConBoth.CheckedChanged += new System.EventHandler(this.radConBoth_CheckedChanged);
            // 
            // radConRemote
            // 
            this.radConRemote.AutoSize = true;
            this.radConRemote.Location = new System.Drawing.Point(6, 42);
            this.radConRemote.Name = "radConRemote";
            this.radConRemote.Size = new System.Drawing.Size(62, 17);
            this.radConRemote.TabIndex = 5;
            this.radConRemote.TabStop = true;
            this.radConRemote.Text = "Remote";
            this.radConRemote.UseVisualStyleBackColor = true;
            this.radConRemote.CheckedChanged += new System.EventHandler(this.radConRemote_CheckedChanged);
            // 
            // radConLocal
            // 
            this.radConLocal.AutoSize = true;
            this.radConLocal.Location = new System.Drawing.Point(6, 19);
            this.radConLocal.Name = "radConLocal";
            this.radConLocal.Size = new System.Drawing.Size(51, 17);
            this.radConLocal.TabIndex = 4;
            this.radConLocal.TabStop = true;
            this.radConLocal.Text = "Local";
            this.radConLocal.UseVisualStyleBackColor = true;
            this.radConLocal.CheckedChanged += new System.EventHandler(this.radConLocal_CheckedChanged);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.pictureBox1);
            this.groupBox2.Location = new System.Drawing.Point(3, 160);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(582, 359);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Windows Vista / 7 / 2008";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Location = new System.Drawing.Point(7, 20);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(566, 330);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.checkHideLastUsername);
            this.tabPage3.Controls.Add(this.groupBox6);
            this.tabPage3.Controls.Add(this.checkDynamicPassword);
            this.tabPage3.Controls.Add(this.checkUseHTTP);
            this.tabPage3.Controls.Add(this.txtDefaultDomain);
            this.tabPage3.Controls.Add(this.label3);
            this.tabPage3.Controls.Add(this.checkCredInSafeMode);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(592, 525);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Security";
            this.tabPage3.UseVisualStyleBackColor = true;
            this.tabPage3.Click += new System.EventHandler(this.tabPage3_Click);
            // 
            // groupBox6
            // 
            this.groupBox6.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox6.Controls.Add(this.checkCacheDumps);
            this.groupBox6.Controls.Add(this.checkSensitiveLogging);
            this.groupBox6.Controls.Add(this.checkVerboseLogging);
            this.groupBox6.Controls.Add(this.checkTraceLogging);
            this.groupBox6.Location = new System.Drawing.Point(6, 106);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(583, 116);
            this.groupBox6.TabIndex = 17;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Debug Logging";
            // 
            // checkCacheDumps
            // 
            this.checkCacheDumps.AutoSize = true;
            this.checkCacheDumps.Location = new System.Drawing.Point(12, 88);
            this.checkCacheDumps.Name = "checkCacheDumps";
            this.checkCacheDumps.Size = new System.Drawing.Size(129, 17);
            this.checkCacheDumps.TabIndex = 9;
            this.checkCacheDumps.Text = "Enable Cache Dumps";
            this.checkCacheDumps.UseVisualStyleBackColor = true;
            this.checkCacheDumps.CheckedChanged += new System.EventHandler(this.checkCacheDumps_CheckedChanged);
            // 
            // checkSensitiveLogging
            // 
            this.checkSensitiveLogging.AutoSize = true;
            this.checkSensitiveLogging.Location = new System.Drawing.Point(12, 65);
            this.checkSensitiveLogging.Name = "checkSensitiveLogging";
            this.checkSensitiveLogging.Size = new System.Drawing.Size(146, 17);
            this.checkSensitiveLogging.TabIndex = 8;
            this.checkSensitiveLogging.Text = "Enable Sensitive Logging";
            this.checkSensitiveLogging.UseVisualStyleBackColor = true;
            this.checkSensitiveLogging.CheckedChanged += new System.EventHandler(this.checkSensitiveLogging_CheckedChanged);
            // 
            // checkVerboseLogging
            // 
            this.checkVerboseLogging.AutoSize = true;
            this.checkVerboseLogging.Location = new System.Drawing.Point(12, 42);
            this.checkVerboseLogging.Name = "checkVerboseLogging";
            this.checkVerboseLogging.Size = new System.Drawing.Size(142, 17);
            this.checkVerboseLogging.TabIndex = 7;
            this.checkVerboseLogging.Text = "Enable Verbose Logging";
            this.checkVerboseLogging.UseVisualStyleBackColor = true;
            this.checkVerboseLogging.CheckedChanged += new System.EventHandler(this.checkVerboseLogging_CheckedChanged);
            // 
            // checkTraceLogging
            // 
            this.checkTraceLogging.AutoSize = true;
            this.checkTraceLogging.Location = new System.Drawing.Point(12, 19);
            this.checkTraceLogging.Name = "checkTraceLogging";
            this.checkTraceLogging.Size = new System.Drawing.Size(135, 17);
            this.checkTraceLogging.TabIndex = 6;
            this.checkTraceLogging.Text = "Enable Debug Logging";
            this.checkTraceLogging.UseVisualStyleBackColor = true;
            this.checkTraceLogging.CheckedChanged += new System.EventHandler(this.checkTraceLogging_CheckedChanged);
            // 
            // checkDynamicPassword
            // 
            this.checkDynamicPassword.AutoSize = true;
            this.checkDynamicPassword.Location = new System.Drawing.Point(5, 228);
            this.checkDynamicPassword.Name = "checkDynamicPassword";
            this.checkDynamicPassword.Size = new System.Drawing.Size(157, 17);
            this.checkDynamicPassword.TabIndex = 16;
            this.checkDynamicPassword.Text = "Enable Dynamic Passwords";
            this.checkDynamicPassword.UseVisualStyleBackColor = true;
            this.checkDynamicPassword.CheckedChanged += new System.EventHandler(this.checkDynamicPassword_CheckedChanged);
            // 
            // checkUseHTTP
            // 
            this.checkUseHTTP.AutoSize = true;
            this.checkUseHTTP.Location = new System.Drawing.Point(5, 83);
            this.checkUseHTTP.Name = "checkUseHTTP";
            this.checkUseHTTP.Size = new System.Drawing.Size(289, 17);
            this.checkUseHTTP.TabIndex = 15;
            this.checkUseHTTP.Text = "Use HTTP for API calls (Required for Win 2003 servers)";
            this.checkUseHTTP.UseVisualStyleBackColor = true;
            this.checkUseHTTP.CheckedChanged += new System.EventHandler(this.checkUseHTTP_CheckedChanged);
            // 
            // txtDefaultDomain
            // 
            this.txtDefaultDomain.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtDefaultDomain.Location = new System.Drawing.Point(89, 4);
            this.txtDefaultDomain.Name = "txtDefaultDomain";
            this.txtDefaultDomain.Size = new System.Drawing.Size(500, 20);
            this.txtDefaultDomain.TabIndex = 14;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 10);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(80, 13);
            this.label3.TabIndex = 13;
            this.label3.Text = "Default Domain";
            // 
            // checkCredInSafeMode
            // 
            this.checkCredInSafeMode.AutoSize = true;
            this.checkCredInSafeMode.Location = new System.Drawing.Point(5, 60);
            this.checkCredInSafeMode.Name = "checkCredInSafeMode";
            this.checkCredInSafeMode.Size = new System.Drawing.Size(298, 17);
            this.checkCredInSafeMode.TabIndex = 12;
            this.checkCredInSafeMode.Text = "Force credential providers in safe mode: (Windows 7 only)";
            this.checkCredInSafeMode.UseVisualStyleBackColor = true;
            this.checkCredInSafeMode.CheckedChanged += new System.EventHandler(this.checkCredInSafeMode_CheckedChanged);
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.groupTokenCacheEnabled);
            this.tabPage5.Controls.Add(this.groupTokenCacheDisable);
            this.tabPage5.Location = new System.Drawing.Point(4, 22);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Size = new System.Drawing.Size(592, 525);
            this.tabPage5.TabIndex = 4;
            this.tabPage5.Text = "Token Caching";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // groupTokenCacheEnabled
            // 
            this.groupTokenCacheEnabled.Controls.Add(this.groupBox7);
            this.groupTokenCacheEnabled.Controls.Add(this.txtHOTPCount);
            this.groupTokenCacheEnabled.Controls.Add(this.txtTOTPCount);
            this.groupTokenCacheEnabled.Controls.Add(this.labHOTPCount);
            this.groupTokenCacheEnabled.Controls.Add(this.labTOTPCount);
            this.groupTokenCacheEnabled.Controls.Add(this.checkEnableCaching);
            this.groupTokenCacheEnabled.Location = new System.Drawing.Point(0, 0);
            this.groupTokenCacheEnabled.Name = "groupTokenCacheEnabled";
            this.groupTokenCacheEnabled.Size = new System.Drawing.Size(586, 519);
            this.groupTokenCacheEnabled.TabIndex = 30;
            this.groupTokenCacheEnabled.TabStop = false;
            this.groupTokenCacheEnabled.Enter += new System.EventHandler(this.groupTokenCacheEnabled_Enter);
            // 
            // groupBox7
            // 
            this.groupBox7.Controls.Add(this.labStatusText);
            this.groupBox7.Controls.Add(this.labStatus);
            this.groupBox7.Controls.Add(this.txtClientRefresh);
            this.groupBox7.Controls.Add(this.labClientRefresh);
            this.groupBox7.Controls.Add(this.txtPollPeriod);
            this.groupBox7.Controls.Add(this.txtServerAddress);
            this.groupBox7.Controls.Add(this.txtCustomerName);
            this.groupBox7.Controls.Add(this.labPollPeriod);
            this.groupBox7.Controls.Add(this.labServerAddress);
            this.groupBox7.Controls.Add(this.labCustomerName);
            this.groupBox7.Location = new System.Drawing.Point(6, 118);
            this.groupBox7.Name = "groupBox7";
            this.groupBox7.Size = new System.Drawing.Size(574, 145);
            this.groupBox7.TabIndex = 43;
            this.groupBox7.TabStop = false;
            this.groupBox7.Text = "Online Cache Syncing";
            this.groupBox7.Visible = false;
            // 
            // labStatusText
            // 
            this.labStatusText.AutoSize = true;
            this.labStatusText.Location = new System.Drawing.Point(113, 16);
            this.labStatusText.Name = "labStatusText";
            this.labStatusText.Size = new System.Drawing.Size(225, 13);
            this.labStatusText.TabIndex = 50;
            this.labStatusText.Text = "Caution : Cache Syncing Service Not Installed";
            // 
            // labStatus
            // 
            this.labStatus.AutoSize = true;
            this.labStatus.Location = new System.Drawing.Point(66, 16);
            this.labStatus.Name = "labStatus";
            this.labStatus.Size = new System.Drawing.Size(40, 13);
            this.labStatus.TabIndex = 49;
            this.labStatus.Text = "Status ";
            // 
            // txtClientRefresh
            // 
            this.txtClientRefresh.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtClientRefresh.Location = new System.Drawing.Point(116, 116);
            this.txtClientRefresh.Name = "txtClientRefresh";
            this.txtClientRefresh.Size = new System.Drawing.Size(55, 20);
            this.txtClientRefresh.TabIndex = 48;
            this.txtClientRefresh.Text = "4";
            this.txtClientRefresh.TextChanged += new System.EventHandler(this.txtClientRefresh_TextChanged);
            // 
            // labClientRefresh
            // 
            this.labClientRefresh.AutoSize = true;
            this.labClientRefresh.Location = new System.Drawing.Point(0, 119);
            this.labClientRefresh.Name = "labClientRefresh";
            this.labClientRefresh.Size = new System.Drawing.Size(110, 13);
            this.labClientRefresh.TabIndex = 47;
            this.labClientRefresh.Text = "Client Refresh (Hours)";
            // 
            // txtPollPeriod
            // 
            this.txtPollPeriod.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtPollPeriod.Location = new System.Drawing.Point(116, 90);
            this.txtPollPeriod.Name = "txtPollPeriod";
            this.txtPollPeriod.Size = new System.Drawing.Size(55, 20);
            this.txtPollPeriod.TabIndex = 46;
            this.txtPollPeriod.Text = "30";
            this.txtPollPeriod.TextChanged += new System.EventHandler(this.txtPollPeriod_TextChanged);
            // 
            // txtServerAddress
            // 
            this.txtServerAddress.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtServerAddress.Location = new System.Drawing.Point(116, 64);
            this.txtServerAddress.Name = "txtServerAddress";
            this.txtServerAddress.Size = new System.Drawing.Size(234, 20);
            this.txtServerAddress.TabIndex = 45;
            this.txtServerAddress.Text = "mobile.mi-token.com/v5cache";
            this.txtServerAddress.TextChanged += new System.EventHandler(this.txtServerAddress_TextChanged);
            // 
            // txtCustomerName
            // 
            this.txtCustomerName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtCustomerName.Location = new System.Drawing.Point(116, 38);
            this.txtCustomerName.Name = "txtCustomerName";
            this.txtCustomerName.Size = new System.Drawing.Size(234, 20);
            this.txtCustomerName.TabIndex = 44;
            this.txtCustomerName.TextChanged += new System.EventHandler(this.txtCustomerName_TextChanged);
            // 
            // labPollPeriod
            // 
            this.labPollPeriod.AutoSize = true;
            this.labPollPeriod.Location = new System.Drawing.Point(27, 93);
            this.labPollPeriod.Name = "labPollPeriod";
            this.labPollPeriod.Size = new System.Drawing.Size(83, 13);
            this.labPollPeriod.TabIndex = 43;
            this.labPollPeriod.Text = "Poll Period (Min)";
            // 
            // labServerAddress
            // 
            this.labServerAddress.AutoSize = true;
            this.labServerAddress.Location = new System.Drawing.Point(27, 67);
            this.labServerAddress.Name = "labServerAddress";
            this.labServerAddress.Size = new System.Drawing.Size(79, 13);
            this.labServerAddress.TabIndex = 42;
            this.labServerAddress.Text = "Server Address";
            // 
            // labCustomerName
            // 
            this.labCustomerName.AutoSize = true;
            this.labCustomerName.Location = new System.Drawing.Point(24, 41);
            this.labCustomerName.Name = "labCustomerName";
            this.labCustomerName.Size = new System.Drawing.Size(82, 13);
            this.labCustomerName.TabIndex = 41;
            this.labCustomerName.Text = "Customer Name";
            // 
            // txtHOTPCount
            // 
            this.txtHOTPCount.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtHOTPCount.Location = new System.Drawing.Point(150, 82);
            this.txtHOTPCount.Name = "txtHOTPCount";
            this.txtHOTPCount.Size = new System.Drawing.Size(70, 20);
            this.txtHOTPCount.TabIndex = 42;
            this.txtHOTPCount.Text = "0";
            this.txtHOTPCount.TextChanged += new System.EventHandler(this.txtHOTPCount_TextChanged);
            // 
            // txtTOTPCount
            // 
            this.txtTOTPCount.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtTOTPCount.Location = new System.Drawing.Point(150, 53);
            this.txtTOTPCount.Name = "txtTOTPCount";
            this.txtTOTPCount.Size = new System.Drawing.Size(70, 20);
            this.txtTOTPCount.TabIndex = 41;
            this.txtTOTPCount.Text = "0";
            this.txtTOTPCount.TextChanged += new System.EventHandler(this.txtTOTPCount_TextChanged);
            // 
            // labHOTPCount
            // 
            this.labHOTPCount.AutoSize = true;
            this.labHOTPCount.Location = new System.Drawing.Point(3, 85);
            this.labHOTPCount.Name = "labHOTPCount";
            this.labHOTPCount.Size = new System.Drawing.Size(140, 13);
            this.labHOTPCount.TabIndex = 39;
            this.labHOTPCount.Text = "Token Cache Count (Event)";
            // 
            // labTOTPCount
            // 
            this.labTOTPCount.AutoSize = true;
            this.labTOTPCount.Location = new System.Drawing.Point(9, 56);
            this.labTOTPCount.Name = "labTOTPCount";
            this.labTOTPCount.Size = new System.Drawing.Size(135, 13);
            this.labTOTPCount.TabIndex = 38;
            this.labTOTPCount.Text = "Token Cache Count (Time)";
            // 
            // checkEnableCaching
            // 
            this.checkEnableCaching.AutoSize = true;
            this.checkEnableCaching.Location = new System.Drawing.Point(20, 30);
            this.checkEnableCaching.Name = "checkEnableCaching";
            this.checkEnableCaching.Size = new System.Drawing.Size(126, 17);
            this.checkEnableCaching.TabIndex = 30;
            this.checkEnableCaching.Text = "Enable OTP Caching";
            this.checkEnableCaching.UseVisualStyleBackColor = true;
            this.checkEnableCaching.CheckedChanged += new System.EventHandler(this.checkEnableCaching_CheckedChanged);
            // 
            // groupTokenCacheDisable
            // 
            this.groupTokenCacheDisable.Controls.Add(this.label1);
            this.groupTokenCacheDisable.Location = new System.Drawing.Point(0, 0);
            this.groupTokenCacheDisable.Name = "groupTokenCacheDisable";
            this.groupTokenCacheDisable.Size = new System.Drawing.Size(586, 519);
            this.groupTokenCacheDisable.TabIndex = 43;
            this.groupTokenCacheDisable.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(202, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Setting not available in Auto-Config Mode";
            // 
            // checkHideLastUsername
            // 
            this.checkHideLastUsername.AutoSize = true;
            this.checkHideLastUsername.Location = new System.Drawing.Point(5, 37);
            this.checkHideLastUsername.Name = "checkHideLastUsername";
            this.checkHideLastUsername.Size = new System.Drawing.Size(164, 17);
            this.checkHideLastUsername.TabIndex = 18;
            this.checkHideLastUsername.Text = "Hide last logged in Username";
            this.checkHideLastUsername.UseVisualStyleBackColor = true;
            this.checkHideLastUsername.CheckedChanged += new System.EventHandler(this.checkHideLastUsername_Checked);
            // 
            // ConfigControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tabControl);
            this.Name = "ConfigControl";
            this.Size = new System.Drawing.Size(602, 551);
            this.Load += new System.EventHandler(this.ConfigControl_Load);
            this.tabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.groupBoxRadiusServer.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewRADIUSServers)).EndInit();
            this.tabPage2.ResumeLayout(false);
            this.tabPage4.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.tabPage5.ResumeLayout(false);
            this.groupTokenCacheEnabled.ResumeLayout(false);
            this.groupTokenCacheEnabled.PerformLayout();
            this.groupBox7.ResumeLayout(false);
            this.groupBox7.PerformLayout();
            this.groupTokenCacheDisable.ResumeLayout(false);
            this.groupTokenCacheDisable.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.GroupBox groupBoxRadiusServer;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button btnDeleteRadius;
        private System.Windows.Forms.Button btnEditRadius;
        private System.Windows.Forms.Button btnAddRadius;
        private System.Windows.Forms.DataGridView dataGridViewRADIUSServers;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.GroupBox groupBypass;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.RadioButton radFilterAll;
        private System.Windows.Forms.RadioButton radFilterMiToken;
        private System.Windows.Forms.RadioButton radFilterDefault;
        private System.Windows.Forms.RadioButton radFilterNothing;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.CheckBox checkXPOTP;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.RadioButton radConBoth;
        private System.Windows.Forms.RadioButton radConRemote;
        private System.Windows.Forms.RadioButton radConLocal;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.CheckBox checkCacheDumps;
        private System.Windows.Forms.CheckBox checkSensitiveLogging;
        private System.Windows.Forms.CheckBox checkVerboseLogging;
        private System.Windows.Forms.CheckBox checkTraceLogging;
        private System.Windows.Forms.CheckBox checkDynamicPassword;
        private System.Windows.Forms.CheckBox checkUseHTTP;
        private System.Windows.Forms.TextBox txtDefaultDomain;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox checkCredInSafeMode;
        private System.Windows.Forms.GroupBox groupTokenCacheEnabled;
        private System.Windows.Forms.TextBox txtHOTPCount;
        private System.Windows.Forms.TextBox txtTOTPCount;
        private System.Windows.Forms.Label labHOTPCount;
        private System.Windows.Forms.Label labTOTPCount;
        private System.Windows.Forms.CheckBox checkEnableCaching;
        private System.Windows.Forms.GroupBox groupTokenCacheDisable;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox7;
        private System.Windows.Forms.Label labStatusText;
        private System.Windows.Forms.Label labStatus;
        private System.Windows.Forms.TextBox txtClientRefresh;
        private System.Windows.Forms.Label labClientRefresh;
        private System.Windows.Forms.TextBox txtPollPeriod;
        private System.Windows.Forms.TextBox txtServerAddress;
        private System.Windows.Forms.TextBox txtCustomerName;
        private System.Windows.Forms.Label labPollPeriod;
        private System.Windows.Forms.Label labServerAddress;
        private System.Windows.Forms.Label labCustomerName;
        private System.Windows.Forms.CheckBox checkHideLastUsername;

    }
}
