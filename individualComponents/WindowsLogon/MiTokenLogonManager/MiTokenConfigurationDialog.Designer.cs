namespace MiTokenWindowsLogon
{
    partial class MiTokenConfigurationDialog
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
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MiTokenConfigurationDialog));
            this.groupBoxRadiusServer = new System.Windows.Forms.GroupBox();
            this.dataGridViewRADIUSServers = new System.Windows.Forms.DataGridView();
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBoxRadiusServer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewRADIUSServers)).BeginInit();
            this.tabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBoxRadiusServer
            // 
            this.groupBoxRadiusServer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBoxRadiusServer.Controls.Add(this.dataGridViewRADIUSServers);
            this.groupBoxRadiusServer.Location = new System.Drawing.Point(11, 5);
            this.groupBoxRadiusServer.Margin = new System.Windows.Forms.Padding(2);
            this.groupBoxRadiusServer.Name = "groupBoxRadiusServer";
            this.groupBoxRadiusServer.Padding = new System.Windows.Forms.Padding(2);
            this.groupBoxRadiusServer.Size = new System.Drawing.Size(596, 271);
            this.groupBoxRadiusServer.TabIndex = 0;
            this.groupBoxRadiusServer.TabStop = false;
            this.groupBoxRadiusServer.Text = "RADIUS Servers";
            // 
            // dataGridViewRADIUSServers
            // 
            this.dataGridViewRADIUSServers.AllowUserToAddRows = false;
            this.dataGridViewRADIUSServers.AllowUserToDeleteRows = false;
            this.dataGridViewRADIUSServers.AllowUserToOrderColumns = true;
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
            this.dataGridViewRADIUSServers.Dock = System.Windows.Forms.DockStyle.Fill;
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
            this.dataGridViewRADIUSServers.RowTemplate.Height = 24;
            this.dataGridViewRADIUSServers.Size = new System.Drawing.Size(592, 254);
            this.dataGridViewRADIUSServers.TabIndex = 0;
            this.dataGridViewRADIUSServers.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridViewRADIUSServers_CellContentClick);
            // 
            // tabControl
            // 
            this.tabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl.Controls.Add(this.tabPage1);
            this.tabControl.Location = new System.Drawing.Point(9, 10);
            this.tabControl.Margin = new System.Windows.Forms.Padding(2);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(626, 316);
            this.tabControl.TabIndex = 1;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.groupBoxRadiusServer);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Margin = new System.Windows.Forms.Padding(2);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(2);
            this.tabPage1.Size = new System.Drawing.Size(618, 290);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Authentication";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // MiTokenConfigurationDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(644, 335);
            this.Controls.Add(this.tabControl);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(2);
            this.MinimumSize = new System.Drawing.Size(653, 373);
            this.Name = "MiTokenConfigurationDialog";
            this.Text = "Mi-Token Radius Server Configuration Tool";
            this.Load += new System.EventHandler(this.MiTokenConfigurationDialog_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MiTokenConfigurationDialog_FormClosing);
            this.groupBoxRadiusServer.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewRADIUSServers)).EndInit();
            this.tabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBoxRadiusServer;
        private System.Windows.Forms.DataGridView dataGridViewRADIUSServers;
        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPage1;
    }
}

