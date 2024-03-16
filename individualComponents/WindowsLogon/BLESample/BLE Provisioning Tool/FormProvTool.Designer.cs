namespace BLE_Provisioning_Tool
{
    partial class FormProvTool
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormProvTool));
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.toolStripProgressBar = new System.Windows.Forms.ToolStripProgressBar();
            this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.provisioningToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.extendedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.refreshToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.settingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.serialPortToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.filterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.noneToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.allMiTokenToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.miTokenBluToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.miTokenBeaconToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.miTokenBootToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.wizardPages = new BLE_Provisioning_Tool.WizardPages();
            this.tabPageDeviceSelection = new System.Windows.Forms.TabPage();
            this.flowLayoutPanelBleDevices = new System.Windows.Forms.FlowLayoutPanel();
            this.tabPageCfgSelection = new System.Windows.Forms.TabPage();
            this.ucConfigurationData = new BLE_Provisioning_Tool.UserControlConfigurationData();
            this.bulkOADToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.wizardPages.SuspendLayout();
            this.tabPageDeviceSelection.SuspendLayout();
            this.tabPageCfgSelection.SuspendLayout();
            this.SuspendLayout();
            // 
            // statusStrip
            // 
            this.statusStrip.Font = new System.Drawing.Font("Tahoma", 8.25F);
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripProgressBar,
            this.toolStripStatusLabel});
            this.statusStrip.Location = new System.Drawing.Point(0, 573);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.statusStrip.Size = new System.Drawing.Size(603, 22);
            this.statusStrip.TabIndex = 7;
            this.statusStrip.Text = "statusStrip1";
            // 
            // toolStripProgressBar
            // 
            this.toolStripProgressBar.Name = "toolStripProgressBar";
            this.toolStripProgressBar.Size = new System.Drawing.Size(100, 16);
            // 
            // toolStripStatusLabel
            // 
            this.toolStripStatusLabel.AutoSize = false;
            this.toolStripStatusLabel.Name = "toolStripStatusLabel";
            this.toolStripStatusLabel.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.toolStripStatusLabel.Size = new System.Drawing.Size(464, 17);
            this.toolStripStatusLabel.Text = "Idle";
            this.toolStripStatusLabel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Font = new System.Drawing.Font("Tahoma", 8.25F);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.settingsToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(603, 24);
            this.menuStrip1.TabIndex = 12;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exportToolStripMenuItem,
            this.refreshToolStripMenuItem,
            this.bulkOADToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // exportToolStripMenuItem
            // 
            this.exportToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.provisioningToolStripMenuItem,
            this.extendedToolStripMenuItem});
            this.exportToolStripMenuItem.Name = "exportToolStripMenuItem";
            this.exportToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.exportToolStripMenuItem.Text = "Save Bonds";
            // 
            // provisioningToolStripMenuItem
            // 
            this.provisioningToolStripMenuItem.Name = "provisioningToolStripMenuItem";
            this.provisioningToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.provisioningToolStripMenuItem.Text = "Basic";
            this.provisioningToolStripMenuItem.Click += new System.EventHandler(this.provisioningToolStripMenuItem_Click);
            // 
            // extendedToolStripMenuItem
            // 
            this.extendedToolStripMenuItem.Name = "extendedToolStripMenuItem";
            this.extendedToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.extendedToolStripMenuItem.Text = "Extended";
            this.extendedToolStripMenuItem.Click += new System.EventHandler(this.extendedToolStripMenuItem_Click);
            // 
            // refreshToolStripMenuItem
            // 
            this.refreshToolStripMenuItem.Name = "refreshToolStripMenuItem";
            this.refreshToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.refreshToolStripMenuItem.Text = "Refresh";
            this.refreshToolStripMenuItem.Click += new System.EventHandler(this.refreshToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // settingsToolStripMenuItem
            // 
            this.settingsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.serialPortToolStripMenuItem,
            this.filterToolStripMenuItem});
            this.settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
            this.settingsToolStripMenuItem.Size = new System.Drawing.Size(58, 20);
            this.settingsToolStripMenuItem.Text = "Settings";
            // 
            // serialPortToolStripMenuItem
            // 
            this.serialPortToolStripMenuItem.Name = "serialPortToolStripMenuItem";
            this.serialPortToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.serialPortToolStripMenuItem.Text = "Serial Port";
            this.serialPortToolStripMenuItem.Click += new System.EventHandler(this.serialPortToolStripMenuItem_Click);
            // 
            // filterToolStripMenuItem
            // 
            this.filterToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.noneToolStripMenuItem,
            this.allMiTokenToolStripMenuItem,
            this.miTokenBluToolStripMenuItem,
            this.miTokenBeaconToolStripMenuItem,
            this.miTokenBootToolStripMenuItem});
            this.filterToolStripMenuItem.Name = "filterToolStripMenuItem";
            this.filterToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.filterToolStripMenuItem.Text = "Filter Device Name";
            // 
            // noneToolStripMenuItem
            // 
            this.noneToolStripMenuItem.CheckOnClick = true;
            this.noneToolStripMenuItem.Name = "noneToolStripMenuItem";
            this.noneToolStripMenuItem.Size = new System.Drawing.Size(204, 22);
            this.noneToolStripMenuItem.Text = "None (*)";
            this.noneToolStripMenuItem.Click += new System.EventHandler(this.noneToolStripMenuItem_Click);
            // 
            // allMiTokenToolStripMenuItem
            // 
            this.allMiTokenToolStripMenuItem.CheckOnClick = true;
            this.allMiTokenToolStripMenuItem.Name = "allMiTokenToolStripMenuItem";
            this.allMiTokenToolStripMenuItem.Size = new System.Drawing.Size(204, 22);
            this.allMiTokenToolStripMenuItem.Text = "All Mi-Token (MT-*)";
            this.allMiTokenToolStripMenuItem.Click += new System.EventHandler(this.allMiTokenToolStripMenuItem_Click);
            // 
            // miTokenBluToolStripMenuItem
            // 
            this.miTokenBluToolStripMenuItem.CheckOnClick = true;
            this.miTokenBluToolStripMenuItem.Name = "miTokenBluToolStripMenuItem";
            this.miTokenBluToolStripMenuItem.Size = new System.Drawing.Size(204, 22);
            this.miTokenBluToolStripMenuItem.Text = "Mi-Token Blu (MT-BLU)";
            this.miTokenBluToolStripMenuItem.Click += new System.EventHandler(this.miTokenBluToolStripMenuItem_Click);
            // 
            // miTokenBeaconToolStripMenuItem
            // 
            this.miTokenBeaconToolStripMenuItem.CheckOnClick = true;
            this.miTokenBeaconToolStripMenuItem.Name = "miTokenBeaconToolStripMenuItem";
            this.miTokenBeaconToolStripMenuItem.Size = new System.Drawing.Size(204, 22);
            this.miTokenBeaconToolStripMenuItem.Text = "Mi-Token Beacon (MT-BCN)";
            this.miTokenBeaconToolStripMenuItem.Click += new System.EventHandler(this.miTokenBeaconToolStripMenuItem_Click);
            // 
            // miTokenBootToolStripMenuItem
            // 
            this.miTokenBootToolStripMenuItem.Name = "miTokenBootToolStripMenuItem";
            this.miTokenBootToolStripMenuItem.Size = new System.Drawing.Size(204, 22);
            this.miTokenBootToolStripMenuItem.Text = "Mi-Token Boot (MT-BOOT)";
            this.miTokenBootToolStripMenuItem.Click += new System.EventHandler(this.miTokenBootToolStripMenuItem_Click);
            // 
            // wizardPages
            // 
            this.wizardPages.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.wizardPages.Controls.Add(this.tabPageDeviceSelection);
            this.wizardPages.Controls.Add(this.tabPageCfgSelection);
            this.wizardPages.Location = new System.Drawing.Point(0, 27);
            this.wizardPages.Name = "wizardPages";
            this.wizardPages.SelectedIndex = 0;
            this.wizardPages.Size = new System.Drawing.Size(603, 543);
            this.wizardPages.TabIndex = 11;
            this.wizardPages.TabStop = false;
            // 
            // tabPageDeviceSelection
            // 
            this.tabPageDeviceSelection.BackColor = System.Drawing.SystemColors.Control;
            this.tabPageDeviceSelection.Controls.Add(this.flowLayoutPanelBleDevices);
            this.tabPageDeviceSelection.Location = new System.Drawing.Point(4, 22);
            this.tabPageDeviceSelection.Name = "tabPageDeviceSelection";
            this.tabPageDeviceSelection.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageDeviceSelection.Size = new System.Drawing.Size(595, 517);
            this.tabPageDeviceSelection.TabIndex = 0;
            this.tabPageDeviceSelection.Text = "Device Selection";
            // 
            // flowLayoutPanelBleDevices
            // 
            this.flowLayoutPanelBleDevices.AutoScroll = true;
            this.flowLayoutPanelBleDevices.BackColor = System.Drawing.SystemColors.Control;
            this.flowLayoutPanelBleDevices.Dock = System.Windows.Forms.DockStyle.Fill;
            this.flowLayoutPanelBleDevices.Location = new System.Drawing.Point(3, 3);
            this.flowLayoutPanelBleDevices.Name = "flowLayoutPanelBleDevices";
            this.flowLayoutPanelBleDevices.Size = new System.Drawing.Size(589, 511);
            this.flowLayoutPanelBleDevices.TabIndex = 5;
            // 
            // tabPageCfgSelection
            // 
            this.tabPageCfgSelection.BackColor = System.Drawing.SystemColors.Control;
            this.tabPageCfgSelection.Controls.Add(this.ucConfigurationData);
            this.tabPageCfgSelection.Location = new System.Drawing.Point(4, 22);
            this.tabPageCfgSelection.Name = "tabPageCfgSelection";
            this.tabPageCfgSelection.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageCfgSelection.Size = new System.Drawing.Size(595, 517);
            this.tabPageCfgSelection.TabIndex = 2;
            this.tabPageCfgSelection.Text = "Configure Selection";
            // 
            // ucConfigurationData
            // 
            this.ucConfigurationData.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ucConfigurationData.BackColor = System.Drawing.SystemColors.Control;
            this.ucConfigurationData.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ucConfigurationData.Location = new System.Drawing.Point(6, 6);
            this.ucConfigurationData.Name = "ucConfigurationData";
            this.ucConfigurationData.Size = new System.Drawing.Size(595, 501);
            this.ucConfigurationData.TabIndex = 0;
            // 
            // bulkOADToolStripMenuItem
            // 
            this.bulkOADToolStripMenuItem.Name = "bulkOADToolStripMenuItem";
            this.bulkOADToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.bulkOADToolStripMenuItem.Text = "Bulk Update";
            this.bulkOADToolStripMenuItem.Click += new System.EventHandler(this.bulkOADToolStripMenuItem_Click);
            // 
            // FormProvTool
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(603, 595);
            this.Controls.Add(this.wizardPages);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.menuStrip1);
            this.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "FormProvTool";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Provisioning Tool";
            this.Activated += new System.EventHandler(this.FormProvTool_Activated);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormProvTool_FormClosing);
            this.Load += new System.EventHandler(this.FormProvTool_Load);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.FormProvTool_KeyDown);
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.wizardPages.ResumeLayout(false);
            this.tabPageDeviceSelection.ResumeLayout(false);
            this.tabPageCfgSelection.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripProgressBar toolStripProgressBar;
        private WizardPages wizardPages;
        private System.Windows.Forms.TabPage tabPageDeviceSelection;
        private System.Windows.Forms.TabPage tabPageCfgSelection;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanelBleDevices;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
        private UserControlConfigurationData ucConfigurationData;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem settingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem serialPortToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem provisioningToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem extendedToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem filterToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem noneToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem allMiTokenToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem miTokenBluToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem miTokenBeaconToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem miTokenBootToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem refreshToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem bulkOADToolStripMenuItem;
    }
}

