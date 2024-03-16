namespace API_setup
{
    partial class Security_options
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Security_options));
            this.checkCredInSafeMode = new System.Windows.Forms.CheckBox();
            this.ButSave = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            this.checkTraceLogging = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtDefaultDomain = new System.Windows.Forms.TextBox();
            this.checkUseHTTP = new System.Windows.Forms.CheckBox();
            this.checkDynamicPassword = new System.Windows.Forms.CheckBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.checkCacheDumps = new System.Windows.Forms.CheckBox();
            this.checkSensitiveLogging = new System.Windows.Forms.CheckBox();
            this.checkVerboseLogging = new System.Windows.Forms.CheckBox();
            this.hideLastUsername = new System.Windows.Forms.CheckBox();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // checkCredInSafeMode
            // 
            this.checkCredInSafeMode.AutoSize = true;
            this.checkCredInSafeMode.Location = new System.Drawing.Point(12, 72);
            this.checkCredInSafeMode.Name = "checkCredInSafeMode";
            this.checkCredInSafeMode.Size = new System.Drawing.Size(298, 17);
            this.checkCredInSafeMode.TabIndex = 1;
            this.checkCredInSafeMode.Text = "Force credential providers in safe mode: (Windows 7 only)";
            this.checkCredInSafeMode.UseVisualStyleBackColor = true;
            this.checkCredInSafeMode.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // ButSave
            // 
            this.ButSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.ButSave.Location = new System.Drawing.Point(287, 263);
            this.ButSave.Name = "ButSave";
            this.ButSave.Size = new System.Drawing.Size(75, 23);
            this.ButSave.TabIndex = 4;
            this.ButSave.Text = "Save";
            this.ButSave.UseVisualStyleBackColor = true;
            this.ButSave.Click += new System.EventHandler(this.button1_Click);
            // 
            // button4
            // 
            this.button4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.button4.Location = new System.Drawing.Point(206, 263);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(75, 23);
            this.button4.TabIndex = 4;
            this.button4.Text = "Cancel";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.button4_Click);
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
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(10, 18);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(80, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Default Domain";
            // 
            // txtDefaultDomain
            // 
            this.txtDefaultDomain.Location = new System.Drawing.Point(96, 12);
            this.txtDefaultDomain.Name = "txtDefaultDomain";
            this.txtDefaultDomain.Size = new System.Drawing.Size(266, 20);
            this.txtDefaultDomain.TabIndex = 8;
            // 
            // checkUseHTTP
            // 
            this.checkUseHTTP.AutoSize = true;
            this.checkUseHTTP.Location = new System.Drawing.Point(12, 95);
            this.checkUseHTTP.Name = "checkUseHTTP";
            this.checkUseHTTP.Size = new System.Drawing.Size(289, 17);
            this.checkUseHTTP.TabIndex = 9;
            this.checkUseHTTP.Text = "Use HTTP for API calls (Required for Win 2003 servers)";
            this.checkUseHTTP.UseVisualStyleBackColor = true;
            this.checkUseHTTP.CheckedChanged += new System.EventHandler(this.checkUseHTTP_CheckedChanged);
            // 
            // checkDynamicPassword
            // 
            this.checkDynamicPassword.AutoSize = true;
            this.checkDynamicPassword.Location = new System.Drawing.Point(12, 240);
            this.checkDynamicPassword.Name = "checkDynamicPassword";
            this.checkDynamicPassword.Size = new System.Drawing.Size(157, 17);
            this.checkDynamicPassword.TabIndex = 10;
            this.checkDynamicPassword.Text = "Enable Dynamic Passwords";
            this.checkDynamicPassword.UseVisualStyleBackColor = true;
            this.checkDynamicPassword.CheckedChanged += new System.EventHandler(this.checkBox3_CheckedChanged);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.checkCacheDumps);
            this.groupBox2.Controls.Add(this.checkSensitiveLogging);
            this.groupBox2.Controls.Add(this.checkVerboseLogging);
            this.groupBox2.Controls.Add(this.checkTraceLogging);
            this.groupBox2.Location = new System.Drawing.Point(13, 118);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(179, 116);
            this.groupBox2.TabIndex = 11;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Debug Logging";
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
            // hideLastUsername
            // 
            this.hideLastUsername.AutoSize = true;
            this.hideLastUsername.Location = new System.Drawing.Point(12, 49);
            this.hideLastUsername.Name = "hideLastUsername";
            this.hideLastUsername.Size = new System.Drawing.Size(164, 17);
            this.hideLastUsername.TabIndex = 12;
            this.hideLastUsername.Text = "Hide last logged in Username";
            this.hideLastUsername.UseVisualStyleBackColor = true;
            this.hideLastUsername.CheckedChanged += new System.EventHandler(this.hideLastUsername_CheckChanged);
            // 
            // Security_options
            // 
            this.AcceptButton = this.ButSave;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(374, 309);
            this.Controls.Add(this.hideLastUsername);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.checkDynamicPassword);
            this.Controls.Add(this.checkUseHTTP);
            this.Controls.Add(this.txtDefaultDomain);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.button4);
            this.Controls.Add(this.ButSave);
            this.Controls.Add(this.checkCredInSafeMode);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Security_options";
            this.Text = "Security Options";
            this.Load += new System.EventHandler(this.Security_options_Load);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox checkCredInSafeMode;
        private System.Windows.Forms.Button ButSave;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.CheckBox checkTraceLogging;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtDefaultDomain;
        private System.Windows.Forms.CheckBox checkUseHTTP;
        private System.Windows.Forms.CheckBox checkDynamicPassword;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.CheckBox checkCacheDumps;
        private System.Windows.Forms.CheckBox checkSensitiveLogging;
        private System.Windows.Forms.CheckBox checkVerboseLogging;
        private System.Windows.Forms.CheckBox hideLastUsername;

    }
}