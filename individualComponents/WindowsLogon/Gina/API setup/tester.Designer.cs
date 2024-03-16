namespace API_setup
{
    partial class tester
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(tester));
            this.txtboxAPIServer = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.txtboxUsername = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtboxOneTimePass = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.txtboxAuthResult = new System.Windows.Forms.TextBox();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // txtboxAPIServer
            // 
            this.txtboxAPIServer.Location = new System.Drawing.Point(76, 12);
            this.txtboxAPIServer.Name = "txtboxAPIServer";
            this.txtboxAPIServer.Size = new System.Drawing.Size(141, 20);
            this.txtboxAPIServer.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(58, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "API Server";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 41);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(55, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Username";
            // 
            // txtboxUsername
            // 
            this.txtboxUsername.Location = new System.Drawing.Point(76, 38);
            this.txtboxUsername.Name = "txtboxUsername";
            this.txtboxUsername.Size = new System.Drawing.Size(141, 20);
            this.txtboxUsername.TabIndex = 2;
            this.txtboxUsername.Text = "Administrator";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 67);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(29, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "OTP";
            // 
            // txtboxOneTimePass
            // 
            this.txtboxOneTimePass.Location = new System.Drawing.Point(76, 64);
            this.txtboxOneTimePass.Name = "txtboxOneTimePass";
            this.txtboxOneTimePass.Size = new System.Drawing.Size(141, 20);
            this.txtboxOneTimePass.TabIndex = 4;
            this.txtboxOneTimePass.TextChanged += new System.EventHandler(this.txtboxOneTimePass_TextChanged);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(223, 62);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(77, 23);
            this.button1.TabIndex = 6;
            this.button1.Text = "Test";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // txtboxAuthResult
            // 
            this.txtboxAuthResult.Location = new System.Drawing.Point(12, 91);
            this.txtboxAuthResult.Multiline = true;
            this.txtboxAuthResult.Name = "txtboxAuthResult";
            this.txtboxAuthResult.ReadOnly = true;
            this.txtboxAuthResult.Size = new System.Drawing.Size(288, 335);
            this.txtboxAuthResult.TabIndex = 7;
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Location = new System.Drawing.Point(223, 15);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(77, 17);
            this.checkBox1.TabIndex = 8;
            this.checkBox1.Text = "Use HTTP";
            this.checkBox1.UseVisualStyleBackColor = true;
            // 
            // tester
            // 
            this.AcceptButton = this.button1;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(312, 438);
            this.Controls.Add(this.checkBox1);
            this.Controls.Add(this.txtboxAuthResult);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.txtboxOneTimePass);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtboxUsername);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.txtboxAPIServer);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "tester";
            this.Text = "API Tester";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox txtboxAuthResult;
        public System.Windows.Forms.TextBox txtboxAPIServer;
        public System.Windows.Forms.TextBox txtboxUsername;
        public System.Windows.Forms.TextBox txtboxOneTimePass;
        private System.Windows.Forms.CheckBox checkBox1;
    }
}