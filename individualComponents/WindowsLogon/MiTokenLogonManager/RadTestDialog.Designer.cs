namespace MiTokenWindowsLogon
{
    partial class RadTestDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RadTestDialog));
            this.lblServerSharedSecret = new System.Windows.Forms.Label();
            this.btnCreateNewServer = new System.Windows.Forms.Button();
            this.txtSharedServerSecret = new System.Windows.Forms.TextBox();
            this.lblNewRADIUSPort = new System.Windows.Forms.Label();
            this.grpBoxServerDescription = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.lblPassword = new System.Windows.Forms.Label();
            this.btnTestRADIUSServer = new System.Windows.Forms.Button();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.lblOTP = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.txtUserName = new System.Windows.Forms.TextBox();
            this.txtOTP = new System.Windows.Forms.TextBox();
            this.lblUserName = new System.Windows.Forms.Label();
            this.richTextBoxLog = new System.Windows.Forms.RichTextBox();
            this.lblLog = new System.Windows.Forms.Label();
            this.btnCancel = new System.Windows.Forms.Button();
            this.tableLayoutPanel1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tableLayoutPanel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // lblServerSharedSecret
            // 
            this.lblServerSharedSecret.AutoSize = true;
            this.lblServerSharedSecret.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblServerSharedSecret.Location = new System.Drawing.Point(4, 20);
            this.lblServerSharedSecret.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.lblServerSharedSecret.Name = "lblServerSharedSecret";
            this.lblServerSharedSecret.Size = new System.Drawing.Size(78, 80);
            this.lblServerSharedSecret.TabIndex = 1;
            this.lblServerSharedSecret.Text = "Shared Secret:";
            this.lblServerSharedSecret.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // btnCreateNewServer
            // 
            this.btnCreateNewServer.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.btnCreateNewServer.Location = new System.Drawing.Point(161, 120);
            this.btnCreateNewServer.Margin = new System.Windows.Forms.Padding(4);
            this.btnCreateNewServer.Name = "btnCreateNewServer";
            this.btnCreateNewServer.Size = new System.Drawing.Size(100, 30);
            this.btnCreateNewServer.TabIndex = 6;
            this.btnCreateNewServer.Text = "Add";
            this.btnCreateNewServer.UseVisualStyleBackColor = true;
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
            // grpBoxServerDescription
            // 
            this.grpBoxServerDescription.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.grpBoxServerDescription.Location = new System.Drawing.Point(0, 0);
            this.grpBoxServerDescription.Name = "grpBoxServerDescription";
            this.grpBoxServerDescription.Size = new System.Drawing.Size(200, 100);
            this.grpBoxServerDescription.TabIndex = 0;
            this.grpBoxServerDescription.TabStop = false;
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.lblServerSharedSecret, 0, 1);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(200, 100);
            this.tableLayoutPanel1.TabIndex = 0;
            // 
            // lblPassword
            // 
            this.lblPassword.AutoSize = true;
            this.lblPassword.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblPassword.Location = new System.Drawing.Point(3, 36);
            this.lblPassword.Name = "lblPassword";
            this.lblPassword.Size = new System.Drawing.Size(105, 32);
            this.lblPassword.TabIndex = 1;
            this.lblPassword.Text = "Password:";
            this.lblPassword.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // btnTestRADIUSServer
            // 
            this.btnTestRADIUSServer.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.tableLayoutPanel2.SetColumnSpan(this.btnTestRADIUSServer, 2);
            this.btnTestRADIUSServer.Location = new System.Drawing.Point(203, 104);
            this.btnTestRADIUSServer.Name = "btnTestRADIUSServer";
            this.btnTestRADIUSServer.Size = new System.Drawing.Size(75, 24);
            this.btnTestRADIUSServer.TabIndex = 6;
            this.btnTestRADIUSServer.Text = "Authenticate";
            this.btnTestRADIUSServer.UseVisualStyleBackColor = true;
            this.btnTestRADIUSServer.Click += new System.EventHandler(this.btnTestRADIUSServer_Click);
            // 
            // txtPassword
            // 
            this.txtPassword.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.txtPassword.Location = new System.Drawing.Point(114, 42);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(365, 20);
            this.txtPassword.TabIndex = 4;
            // 
            // lblOTP
            // 
            this.lblOTP.AutoSize = true;
            this.lblOTP.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblOTP.Location = new System.Drawing.Point(3, 68);
            this.lblOTP.Name = "lblOTP";
            this.lblOTP.Size = new System.Drawing.Size(105, 32);
            this.lblOTP.TabIndex = 2;
            this.lblOTP.Text = "One Time Password:";
            this.lblOTP.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.tableLayoutPanel2);
            this.groupBox1.Location = new System.Drawing.Point(10, 11);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(488, 151);
            this.groupBox1.TabIndex = 4;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Test RADIUS server";
            // 
            // tableLayoutPanel2
            // 
            this.tableLayoutPanel2.ColumnCount = 2;
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel2.Controls.Add(this.lblPassword, 0, 1);
            this.tableLayoutPanel2.Controls.Add(this.btnTestRADIUSServer, 0, 3);
            this.tableLayoutPanel2.Controls.Add(this.txtPassword, 1, 1);
            this.tableLayoutPanel2.Controls.Add(this.txtUserName, 1, 0);
            this.tableLayoutPanel2.Controls.Add(this.lblOTP, 0, 2);
            this.tableLayoutPanel2.Controls.Add(this.txtOTP, 1, 2);
            this.tableLayoutPanel2.Controls.Add(this.lblUserName, 0, 0);
            this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 16);
            this.tableLayoutPanel2.Name = "tableLayoutPanel2";
            this.tableLayoutPanel2.Padding = new System.Windows.Forms.Padding(0, 4, 0, 0);
            this.tableLayoutPanel2.RowCount = 4;
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel2.Size = new System.Drawing.Size(482, 132);
            this.tableLayoutPanel2.TabIndex = 7;
            // 
            // txtUserName
            // 
            this.txtUserName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.txtUserName.Location = new System.Drawing.Point(114, 10);
            this.txtUserName.Name = "txtUserName";
            this.txtUserName.Size = new System.Drawing.Size(365, 20);
            this.txtUserName.TabIndex = 3;
            // 
            // txtOTP
            // 
            this.txtOTP.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.txtOTP.Location = new System.Drawing.Point(114, 74);
            this.txtOTP.Name = "txtOTP";
            this.txtOTP.PasswordChar = '*';
            this.txtOTP.Size = new System.Drawing.Size(365, 20);
            this.txtOTP.TabIndex = 5;
            // 
            // lblUserName
            // 
            this.lblUserName.AutoSize = true;
            this.lblUserName.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblUserName.Location = new System.Drawing.Point(3, 4);
            this.lblUserName.Name = "lblUserName";
            this.lblUserName.Size = new System.Drawing.Size(105, 32);
            this.lblUserName.TabIndex = 0;
            this.lblUserName.Text = "User Name:";
            this.lblUserName.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // richTextBoxLog
            // 
            this.richTextBoxLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxLog.Location = new System.Drawing.Point(9, 191);
            this.richTextBoxLog.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.richTextBoxLog.Name = "richTextBoxLog";
            this.richTextBoxLog.ReadOnly = true;
            this.richTextBoxLog.Size = new System.Drawing.Size(489, 236);
            this.richTextBoxLog.TabIndex = 5;
            this.richTextBoxLog.Text = "";
            // 
            // lblLog
            // 
            this.lblLog.AutoSize = true;
            this.lblLog.Location = new System.Drawing.Point(10, 175);
            this.lblLog.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.lblLog.Name = "lblLog";
            this.lblLog.Size = new System.Drawing.Size(28, 13);
            this.lblLog.TabIndex = 6;
            this.lblLog.Text = "Log:";
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.btnCancel.Location = new System.Drawing.Point(216, 430);
            this.btnCancel.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 24);
            this.btnCancel.TabIndex = 7;
            this.btnCancel.Text = "Continue";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // RadTestDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(508, 465);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.lblLog);
            this.Controls.Add(this.richTextBoxLog);
            this.Controls.Add(this.groupBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.MinimumSize = new System.Drawing.Size(518, 503);
            this.Name = "RadTestDialog";
            this.Text = "Mi-Token RADIUS Test";
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.tableLayoutPanel2.ResumeLayout(false);
            this.tableLayoutPanel2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblServerSharedSecret;
        private System.Windows.Forms.Button btnCreateNewServer;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.TextBox txtSharedServerSecret;
        private System.Windows.Forms.Label lblNewRADIUSPort;
        private System.Windows.Forms.GroupBox grpBoxServerDescription;
        private System.Windows.Forms.Label lblPassword;
        private System.Windows.Forms.Button btnTestRADIUSServer;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.TextBox txtUserName;
        private System.Windows.Forms.Label lblOTP;
        private System.Windows.Forms.TextBox txtOTP;
        private System.Windows.Forms.Label lblUserName;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RichTextBox richTextBoxLog;
        private System.Windows.Forms.Label lblLog;
        private System.Windows.Forms.Button btnCancel;
    }
}