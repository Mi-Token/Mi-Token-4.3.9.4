namespace MiTokenWindowsLogon
{
    partial class ServerRegistrationDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ServerRegistrationDialog));
            this.grpBoxServerDescription = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.lblServerSharedSecret = new System.Windows.Forms.Label();
            this.btnCreateNewServer = new System.Windows.Forms.Button();
            this.txtSharedServerSecret = new System.Windows.Forms.TextBox();
            this.txtIPAddress = new System.Windows.Forms.TextBox();
            this.lblNewRADIUSPort = new System.Windows.Forms.Label();
            this.txtRADIUSPort = new System.Windows.Forms.TextBox();
            this.lblIPAddress = new System.Windows.Forms.Label();
            this.grpBoxServerDescription.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // grpBoxServerDescription
            // 
            this.grpBoxServerDescription.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.grpBoxServerDescription.Controls.Add(this.tableLayoutPanel1);
            this.grpBoxServerDescription.Location = new System.Drawing.Point(13, 13);
            this.grpBoxServerDescription.Margin = new System.Windows.Forms.Padding(4);
            this.grpBoxServerDescription.Name = "grpBoxServerDescription";
            this.grpBoxServerDescription.Padding = new System.Windows.Forms.Padding(4);
            this.grpBoxServerDescription.Size = new System.Drawing.Size(431, 177);
            this.grpBoxServerDescription.TabIndex = 3;
            this.grpBoxServerDescription.TabStop = false;
            this.grpBoxServerDescription.Text = "Add a new RADIUS server";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.lblServerSharedSecret, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.btnCreateNewServer, 0, 3);
            this.tableLayoutPanel1.Controls.Add(this.txtSharedServerSecret, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.txtIPAddress, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.lblNewRADIUSPort, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.txtRADIUSPort, 1, 2);
            this.tableLayoutPanel1.Controls.Add(this.lblIPAddress, 0, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(4, 19);
            this.tableLayoutPanel1.Margin = new System.Windows.Forms.Padding(4);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
            this.tableLayoutPanel1.RowCount = 4;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.Size = new System.Drawing.Size(423, 154);
            this.tableLayoutPanel1.TabIndex = 7;
            // 
            // lblServerSharedSecret
            // 
            this.lblServerSharedSecret.AutoSize = true;
            this.lblServerSharedSecret.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblServerSharedSecret.Location = new System.Drawing.Point(4, 42);
            this.lblServerSharedSecret.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lblServerSharedSecret.Name = "lblServerSharedSecret";
            this.lblServerSharedSecret.Size = new System.Drawing.Size(103, 37);
            this.lblServerSharedSecret.TabIndex = 1;
            this.lblServerSharedSecret.Text = "Shared Secret:";
            this.lblServerSharedSecret.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // btnCreateNewServer
            // 
            this.btnCreateNewServer.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.tableLayoutPanel1.SetColumnSpan(this.btnCreateNewServer, 2);
            this.btnCreateNewServer.Location = new System.Drawing.Point(161, 120);
            this.btnCreateNewServer.Margin = new System.Windows.Forms.Padding(4);
            this.btnCreateNewServer.Name = "btnCreateNewServer";
            this.btnCreateNewServer.Size = new System.Drawing.Size(100, 30);
            this.btnCreateNewServer.TabIndex = 6;
            this.btnCreateNewServer.Text = "Add";
            this.btnCreateNewServer.UseVisualStyleBackColor = true;
            this.btnCreateNewServer.Click += new System.EventHandler(this.btnCreateNewServer_Click);
            // 
            // txtSharedServerSecret
            // 
            this.txtSharedServerSecret.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.txtSharedServerSecret.Location = new System.Drawing.Point(115, 49);
            this.txtSharedServerSecret.Margin = new System.Windows.Forms.Padding(4);
            this.txtSharedServerSecret.Multiline = true;
            this.txtSharedServerSecret.Name = "txtSharedServerSecret";
            this.txtSharedServerSecret.Size = new System.Drawing.Size(304, 22);
            this.txtSharedServerSecret.TabIndex = 4;
            // 
            // txtIPAddress
            // 
            this.txtIPAddress.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.txtIPAddress.Location = new System.Drawing.Point(115, 12);
            this.txtIPAddress.Margin = new System.Windows.Forms.Padding(4);
            this.txtIPAddress.Name = "txtIPAddress";
            this.txtIPAddress.Size = new System.Drawing.Size(304, 22);
            this.txtIPAddress.TabIndex = 3;
            // 
            // lblNewRADIUSPort
            // 
            this.lblNewRADIUSPort.AutoSize = true;
            this.lblNewRADIUSPort.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblNewRADIUSPort.Location = new System.Drawing.Point(4, 79);
            this.lblNewRADIUSPort.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lblNewRADIUSPort.Name = "lblNewRADIUSPort";
            this.lblNewRADIUSPort.Size = new System.Drawing.Size(103, 37);
            this.lblNewRADIUSPort.TabIndex = 2;
            this.lblNewRADIUSPort.Text = "RADIUS Port:";
            this.lblNewRADIUSPort.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txtRADIUSPort
            // 
            this.txtRADIUSPort.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.txtRADIUSPort.Location = new System.Drawing.Point(115, 86);
            this.txtRADIUSPort.Margin = new System.Windows.Forms.Padding(4);
            this.txtRADIUSPort.Name = "txtRADIUSPort";
            this.txtRADIUSPort.Size = new System.Drawing.Size(304, 22);
            this.txtRADIUSPort.TabIndex = 5;
            this.txtRADIUSPort.Text = "1645";
            // 
            // lblIPAddress
            // 
            this.lblIPAddress.AutoSize = true;
            this.lblIPAddress.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblIPAddress.Location = new System.Drawing.Point(4, 5);
            this.lblIPAddress.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lblIPAddress.Name = "lblIPAddress";
            this.lblIPAddress.Size = new System.Drawing.Size(103, 37);
            this.lblIPAddress.TabIndex = 0;
            this.lblIPAddress.Text = "IP address:";
            this.lblIPAddress.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // ServerRegistrationDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(457, 203);
            this.Controls.Add(this.grpBoxServerDescription);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.MaximizeBox = false;
            this.Name = "ServerRegistrationDialog";
            this.Text = "Mi-Token RADIUS server properties";
            this.Load += new System.EventHandler(this.ServerRegistrationDialog_Load);
            this.grpBoxServerDescription.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox grpBoxServerDescription;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Label lblIPAddress;
        private System.Windows.Forms.Label lblServerSharedSecret;
        private System.Windows.Forms.TextBox txtSharedServerSecret;
        private System.Windows.Forms.TextBox txtIPAddress;
        private System.Windows.Forms.Label lblNewRADIUSPort;
        private System.Windows.Forms.TextBox txtRADIUSPort;
        private System.Windows.Forms.Button btnCreateNewServer;


    }
}