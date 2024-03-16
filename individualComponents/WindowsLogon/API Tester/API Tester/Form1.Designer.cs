namespace API_Tester
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.btnTestAPI = new System.Windows.Forms.Button();
            this.txtboxUsername = new System.Windows.Forms.TextBox();
            this.txtboxOneTimePass = new System.Windows.Forms.TextBox();
            this.lblUsername = new System.Windows.Forms.Label();
            this.lblOneTimePass = new System.Windows.Forms.Label();
            this.lblAPIServer = new System.Windows.Forms.Label();
            this.txtboxAPIServer = new System.Windows.Forms.TextBox();
            this.txtboxAuthResult = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.txtTimeout = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // btnTestAPI
            // 
            this.btnTestAPI.Location = new System.Drawing.Point(12, 75);
            this.btnTestAPI.Name = "btnTestAPI";
            this.btnTestAPI.Size = new System.Drawing.Size(75, 23);
            this.btnTestAPI.TabIndex = 4;
            this.btnTestAPI.Text = "Test";
            this.btnTestAPI.UseVisualStyleBackColor = true;
            this.btnTestAPI.Click += new System.EventHandler(this.button1_Click);
            // 
            // txtboxUsername
            // 
            this.txtboxUsername.Location = new System.Drawing.Point(120, 12);
            this.txtboxUsername.Name = "txtboxUsername";
            this.txtboxUsername.Size = new System.Drawing.Size(100, 20);
            this.txtboxUsername.TabIndex = 0;
            this.txtboxUsername.Text = "Administrator";
            // 
            // txtboxOneTimePass
            // 
            this.txtboxOneTimePass.Location = new System.Drawing.Point(120, 37);
            this.txtboxOneTimePass.Name = "txtboxOneTimePass";
            this.txtboxOneTimePass.Size = new System.Drawing.Size(100, 20);
            this.txtboxOneTimePass.TabIndex = 2;
            // 
            // lblUsername
            // 
            this.lblUsername.AutoSize = true;
            this.lblUsername.Location = new System.Drawing.Point(12, 15);
            this.lblUsername.Name = "lblUsername";
            this.lblUsername.Size = new System.Drawing.Size(55, 13);
            this.lblUsername.TabIndex = 6;
            this.lblUsername.Text = "Username";
            // 
            // lblOneTimePass
            // 
            this.lblOneTimePass.AutoSize = true;
            this.lblOneTimePass.Location = new System.Drawing.Point(12, 40);
            this.lblOneTimePass.Name = "lblOneTimePass";
            this.lblOneTimePass.Size = new System.Drawing.Size(97, 13);
            this.lblOneTimePass.TabIndex = 8;
            this.lblOneTimePass.Text = "One-time password";
            // 
            // lblAPIServer
            // 
            this.lblAPIServer.AutoSize = true;
            this.lblAPIServer.Location = new System.Drawing.Point(226, 15);
            this.lblAPIServer.Name = "lblAPIServer";
            this.lblAPIServer.Size = new System.Drawing.Size(58, 13);
            this.lblAPIServer.TabIndex = 7;
            this.lblAPIServer.Text = "API Server";
            // 
            // txtboxAPIServer
            // 
            this.txtboxAPIServer.Location = new System.Drawing.Point(297, 12);
            this.txtboxAPIServer.Name = "txtboxAPIServer";
            this.txtboxAPIServer.Size = new System.Drawing.Size(93, 20);
            this.txtboxAPIServer.TabIndex = 1;
            // 
            // txtboxAuthResult
            // 
            this.txtboxAuthResult.Location = new System.Drawing.Point(12, 104);
            this.txtboxAuthResult.Multiline = true;
            this.txtboxAuthResult.Name = "txtboxAuthResult";
            this.txtboxAuthResult.ReadOnly = true;
            this.txtboxAuthResult.Size = new System.Drawing.Size(387, 326);
            this.txtboxAuthResult.TabIndex = 5;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(227, 36);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(64, 37);
            this.label1.TabIndex = 10;
            this.label1.Text = "Connection Timeout";
            // 
            // txtTimeout
            // 
            this.txtTimeout.Location = new System.Drawing.Point(297, 38);
            this.txtTimeout.Name = "txtTimeout";
            this.txtTimeout.Size = new System.Drawing.Size(93, 20);
            this.txtTimeout.TabIndex = 9;
            this.txtTimeout.Text = "2000";
            // 
            // Form1
            // 
            this.AcceptButton = this.btnTestAPI;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(411, 442);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtTimeout);
            this.Controls.Add(this.txtboxAuthResult);
            this.Controls.Add(this.lblAPIServer);
            this.Controls.Add(this.txtboxAPIServer);
            this.Controls.Add(this.lblOneTimePass);
            this.Controls.Add(this.lblUsername);
            this.Controls.Add(this.txtboxOneTimePass);
            this.Controls.Add(this.txtboxUsername);
            this.Controls.Add(this.btnTestAPI);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.Text = "Mi-Token API Tester";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnTestAPI;
        private System.Windows.Forms.TextBox txtboxUsername;
        private System.Windows.Forms.TextBox txtboxOneTimePass;
        private System.Windows.Forms.Label lblUsername;
        private System.Windows.Forms.Label lblOneTimePass;
        private System.Windows.Forms.Label lblAPIServer;
        private System.Windows.Forms.TextBox txtboxAPIServer;
        private System.Windows.Forms.TextBox txtboxAuthResult;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtTimeout;
    }
}

