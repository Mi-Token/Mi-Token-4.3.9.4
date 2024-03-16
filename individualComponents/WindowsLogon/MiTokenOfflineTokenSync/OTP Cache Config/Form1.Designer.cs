namespace OTP_Cache_Config
{
    partial class Form1
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
            this.checkEnableCaching = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtPollPeriod = new System.Windows.Forms.TextBox();
            this.txtServerAddress = new System.Windows.Forms.TextBox();
            this.txtCustomerName = new System.Windows.Forms.TextBox();
            this.groupClientSettings = new System.Windows.Forms.GroupBox();
            this.txtHOTPCount = new System.Windows.Forms.TextBox();
            this.txtTOTPCount = new System.Windows.Forms.TextBox();
            this.txtClientRefresh = new System.Windows.Forms.TextBox();
            this.labHOTPCount = new System.Windows.Forms.Label();
            this.labTOTPCount = new System.Windows.Forms.Label();
            this.labClientRefresh = new System.Windows.Forms.Label();
            this.checkClientMachine = new System.Windows.Forms.CheckBox();
            this.labPollPeriod = new System.Windows.Forms.Label();
            this.labServerAddress = new System.Windows.Forms.Label();
            this.labCustomerName = new System.Windows.Forms.Label();
            this.butSave = new System.Windows.Forms.Button();
            this.butExport = new System.Windows.Forms.Button();
            this.butInstallService = new System.Windows.Forms.Button();
            this.labClientMachine = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.groupClientSettings.SuspendLayout();
            this.SuspendLayout();
            // 
            // checkEnableCaching
            // 
            this.checkEnableCaching.AutoSize = true;
            this.checkEnableCaching.Location = new System.Drawing.Point(6, 19);
            this.checkEnableCaching.Name = "checkEnableCaching";
            this.checkEnableCaching.Size = new System.Drawing.Size(126, 17);
            this.checkEnableCaching.TabIndex = 7;
            this.checkEnableCaching.Text = "Enable OTP Caching";
            this.checkEnableCaching.UseVisualStyleBackColor = true;
            this.checkEnableCaching.CheckedChanged += new System.EventHandler(this.checkEnableCaching_CheckedChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.txtPollPeriod);
            this.groupBox1.Controls.Add(this.txtServerAddress);
            this.groupBox1.Controls.Add(this.txtCustomerName);
            this.groupBox1.Controls.Add(this.groupClientSettings);
            this.groupBox1.Controls.Add(this.labPollPeriod);
            this.groupBox1.Controls.Add(this.labServerAddress);
            this.groupBox1.Controls.Add(this.labCustomerName);
            this.groupBox1.Controls.Add(this.checkEnableCaching);
            this.groupBox1.Location = new System.Drawing.Point(12, 8);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(349, 246);
            this.groupBox1.TabIndex = 9;
            this.groupBox1.TabStop = false;
            this.groupBox1.Enter += new System.EventHandler(this.groupBox1_Enter);
            // 
            // txtPollPeriod
            // 
            this.txtPollPeriod.Location = new System.Drawing.Point(133, 93);
            this.txtPollPeriod.Name = "txtPollPeriod";
            this.txtPollPeriod.Size = new System.Drawing.Size(204, 20);
            this.txtPollPeriod.TabIndex = 15;
            this.txtPollPeriod.Text = "30";
            // 
            // txtServerAddress
            // 
            this.txtServerAddress.Location = new System.Drawing.Point(133, 67);
            this.txtServerAddress.Name = "txtServerAddress";
            this.txtServerAddress.Size = new System.Drawing.Size(204, 20);
            this.txtServerAddress.TabIndex = 14;
            this.txtServerAddress.Text = "mobile.mi-token.com/v5cache";
            // 
            // txtCustomerName
            // 
            this.txtCustomerName.Location = new System.Drawing.Point(133, 41);
            this.txtCustomerName.Name = "txtCustomerName";
            this.txtCustomerName.Size = new System.Drawing.Size(204, 20);
            this.txtCustomerName.TabIndex = 13;
            // 
            // groupClientSettings
            // 
            this.groupClientSettings.Controls.Add(this.labClientMachine);
            this.groupClientSettings.Controls.Add(this.txtHOTPCount);
            this.groupClientSettings.Controls.Add(this.txtTOTPCount);
            this.groupClientSettings.Controls.Add(this.txtClientRefresh);
            this.groupClientSettings.Controls.Add(this.labHOTPCount);
            this.groupClientSettings.Controls.Add(this.labTOTPCount);
            this.groupClientSettings.Controls.Add(this.labClientRefresh);
            this.groupClientSettings.Controls.Add(this.checkClientMachine);
            this.groupClientSettings.Location = new System.Drawing.Point(6, 119);
            this.groupClientSettings.Name = "groupClientSettings";
            this.groupClientSettings.Size = new System.Drawing.Size(337, 123);
            this.groupClientSettings.TabIndex = 12;
            this.groupClientSettings.TabStop = false;
            this.groupClientSettings.Text = "Client Settings";
            // 
            // txtHOTPCount
            // 
            this.txtHOTPCount.Location = new System.Drawing.Point(156, 88);
            this.txtHOTPCount.Name = "txtHOTPCount";
            this.txtHOTPCount.Size = new System.Drawing.Size(175, 20);
            this.txtHOTPCount.TabIndex = 17;
            this.txtHOTPCount.Text = "0";
            // 
            // txtTOTPCount
            // 
            this.txtTOTPCount.Location = new System.Drawing.Point(156, 62);
            this.txtTOTPCount.Name = "txtTOTPCount";
            this.txtTOTPCount.Size = new System.Drawing.Size(175, 20);
            this.txtTOTPCount.TabIndex = 16;
            this.txtTOTPCount.Text = "0";
            // 
            // txtClientRefresh
            // 
            this.txtClientRefresh.Location = new System.Drawing.Point(156, 36);
            this.txtClientRefresh.Name = "txtClientRefresh";
            this.txtClientRefresh.Size = new System.Drawing.Size(175, 20);
            this.txtClientRefresh.TabIndex = 15;
            this.txtClientRefresh.Text = "4";
            // 
            // labHOTPCount
            // 
            this.labHOTPCount.AutoSize = true;
            this.labHOTPCount.Location = new System.Drawing.Point(9, 91);
            this.labHOTPCount.Name = "labHOTPCount";
            this.labHOTPCount.Size = new System.Drawing.Size(140, 13);
            this.labHOTPCount.TabIndex = 14;
            this.labHOTPCount.Text = "Token Cache Count (Event)";
            this.labHOTPCount.Click += new System.EventHandler(this.labHOTPCount_Click);
            // 
            // labTOTPCount
            // 
            this.labTOTPCount.AutoSize = true;
            this.labTOTPCount.Location = new System.Drawing.Point(15, 65);
            this.labTOTPCount.Name = "labTOTPCount";
            this.labTOTPCount.Size = new System.Drawing.Size(135, 13);
            this.labTOTPCount.TabIndex = 13;
            this.labTOTPCount.Text = "Token Cache Count (Time)";
            // 
            // labClientRefresh
            // 
            this.labClientRefresh.AutoSize = true;
            this.labClientRefresh.Location = new System.Drawing.Point(40, 39);
            this.labClientRefresh.Name = "labClientRefresh";
            this.labClientRefresh.Size = new System.Drawing.Size(110, 13);
            this.labClientRefresh.TabIndex = 11;
            this.labClientRefresh.Text = "Client Refresh (Hours)";
            // 
            // checkClientMachine
            // 
            this.checkClientMachine.AutoSize = true;
            this.checkClientMachine.Location = new System.Drawing.Point(15, 19);
            this.checkClientMachine.Name = "checkClientMachine";
            this.checkClientMachine.Size = new System.Drawing.Size(96, 17);
            this.checkClientMachine.TabIndex = 9;
            this.checkClientMachine.Text = "Client Machine";
            this.checkClientMachine.UseVisualStyleBackColor = true;
            this.checkClientMachine.Visible = false;
            this.checkClientMachine.CheckedChanged += new System.EventHandler(this.checkClientMachine_CheckedChanged);
            // 
            // labPollPeriod
            // 
            this.labPollPeriod.AutoSize = true;
            this.labPollPeriod.Location = new System.Drawing.Point(44, 96);
            this.labPollPeriod.Name = "labPollPeriod";
            this.labPollPeriod.Size = new System.Drawing.Size(83, 13);
            this.labPollPeriod.TabIndex = 10;
            this.labPollPeriod.Text = "Poll Period (Min)";
            // 
            // labServerAddress
            // 
            this.labServerAddress.AutoSize = true;
            this.labServerAddress.Location = new System.Drawing.Point(44, 70);
            this.labServerAddress.Name = "labServerAddress";
            this.labServerAddress.Size = new System.Drawing.Size(79, 13);
            this.labServerAddress.TabIndex = 9;
            this.labServerAddress.Text = "Server Address";
            // 
            // labCustomerName
            // 
            this.labCustomerName.AutoSize = true;
            this.labCustomerName.Location = new System.Drawing.Point(41, 44);
            this.labCustomerName.Name = "labCustomerName";
            this.labCustomerName.Size = new System.Drawing.Size(82, 13);
            this.labCustomerName.TabIndex = 8;
            this.labCustomerName.Text = "Customer Name";
            // 
            // butSave
            // 
            this.butSave.Location = new System.Drawing.Point(286, 260);
            this.butSave.Name = "butSave";
            this.butSave.Size = new System.Drawing.Size(75, 23);
            this.butSave.TabIndex = 10;
            this.butSave.Text = "Done";
            this.butSave.UseVisualStyleBackColor = true;
            this.butSave.Click += new System.EventHandler(this.butSave_Click);
            // 
            // butExport
            // 
            this.butExport.Enabled = false;
            this.butExport.Location = new System.Drawing.Point(205, 260);
            this.butExport.Name = "butExport";
            this.butExport.Size = new System.Drawing.Size(75, 23);
            this.butExport.TabIndex = 11;
            this.butExport.Text = "Export";
            this.butExport.UseVisualStyleBackColor = true;
            this.butExport.Visible = false;
            this.butExport.Click += new System.EventHandler(this.butExport_Click);
            // 
            // butInstallService
            // 
            this.butInstallService.Enabled = false;
            this.butInstallService.Location = new System.Drawing.Point(12, 260);
            this.butInstallService.Name = "butInstallService";
            this.butInstallService.Size = new System.Drawing.Size(89, 23);
            this.butInstallService.TabIndex = 12;
            this.butInstallService.Text = "Install Service";
            this.butInstallService.UseVisualStyleBackColor = true;
            this.butInstallService.Visible = false;
            this.butInstallService.Click += new System.EventHandler(this.butInstallService_Click);
            // 
            // labClientMachine
            // 
            this.labClientMachine.AutoSize = true;
            this.labClientMachine.Location = new System.Drawing.Point(12, 19);
            this.labClientMachine.Name = "labClientMachine";
            this.labClientMachine.Size = new System.Drawing.Size(138, 13);
            this.labClientMachine.TabIndex = 18;
            this.labClientMachine.Text = "Client Service not detected.";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(373, 287);
            this.Controls.Add(this.butInstallService);
            this.Controls.Add(this.butExport);
            this.Controls.Add(this.butSave);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Name = "Form1";
            this.Text = "Mi-Token OTP Caching Configuration";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupClientSettings.ResumeLayout(false);
            this.groupClientSettings.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.CheckBox checkEnableCaching;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox txtPollPeriod;
        private System.Windows.Forms.TextBox txtServerAddress;
        private System.Windows.Forms.TextBox txtCustomerName;
        private System.Windows.Forms.GroupBox groupClientSettings;
        private System.Windows.Forms.TextBox txtHOTPCount;
        private System.Windows.Forms.TextBox txtTOTPCount;
        private System.Windows.Forms.TextBox txtClientRefresh;
        private System.Windows.Forms.Label labHOTPCount;
        private System.Windows.Forms.Label labTOTPCount;
        private System.Windows.Forms.Label labClientRefresh;
        private System.Windows.Forms.CheckBox checkClientMachine;
        private System.Windows.Forms.Label labPollPeriod;
        private System.Windows.Forms.Label labServerAddress;
        private System.Windows.Forms.Label labCustomerName;
        private System.Windows.Forms.Button butSave;
        private System.Windows.Forms.Button butExport;
        private System.Windows.Forms.Button butInstallService;
        private System.Windows.Forms.Label labClientMachine;
    }
}

