namespace API_setup
{
    partial class add_bypass
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(add_bypass));
            this.UsernameLabel = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.bypassCodeBox = new System.Windows.Forms.TextBox();
            this.OK = new System.Windows.Forms.Button();
            this.CancelButton = new System.Windows.Forms.Button();
            this.butSelectUser = new System.Windows.Forms.Button();
            this.checkAllUsers = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // UsernameLabel
            // 
            this.UsernameLabel.AutoSize = true;
            this.UsernameLabel.Location = new System.Drawing.Point(124, 9);
            this.UsernameLabel.Name = "UsernameLabel";
            this.UsernameLabel.Size = new System.Drawing.Size(72, 13);
            this.UsernameLabel.TabIndex = 0;
            this.UsernameLabel.Text = "Select A User";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(46, 39);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(69, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Bypass Code";
            // 
            // bypassCodeBox
            // 
            this.bypassCodeBox.Location = new System.Drawing.Point(121, 36);
            this.bypassCodeBox.Name = "bypassCodeBox";
            this.bypassCodeBox.Size = new System.Drawing.Size(152, 20);
            this.bypassCodeBox.TabIndex = 2;
            // 
            // OK
            // 
            this.OK.Location = new System.Drawing.Point(294, 86);
            this.OK.Name = "OK";
            this.OK.Size = new System.Drawing.Size(75, 23);
            this.OK.TabIndex = 4;
            this.OK.Text = "OK";
            this.OK.UseVisualStyleBackColor = true;
            this.OK.Click += new System.EventHandler(this.OK_Click);
            // 
            // CancelButton
            // 
            this.CancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.CancelButton.Location = new System.Drawing.Point(375, 86);
            this.CancelButton.Name = "CancelButton";
            this.CancelButton.Size = new System.Drawing.Size(75, 23);
            this.CancelButton.TabIndex = 5;
            this.CancelButton.Text = "Cancel";
            this.CancelButton.UseVisualStyleBackColor = true;
            this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);
            // 
            // butSelectUser
            // 
            this.butSelectUser.Location = new System.Drawing.Point(299, 4);
            this.butSelectUser.Name = "butSelectUser";
            this.butSelectUser.Size = new System.Drawing.Size(123, 23);
            this.butSelectUser.TabIndex = 1;
            this.butSelectUser.Text = "Select User / Group";
            this.butSelectUser.UseVisualStyleBackColor = true;
            this.butSelectUser.Click += new System.EventHandler(this.button1_Click);
            // 
            // checkAllUsers
            // 
            this.checkAllUsers.AutoSize = true;
            this.checkAllUsers.Location = new System.Drawing.Point(299, 36);
            this.checkAllUsers.Name = "checkAllUsers";
            this.checkAllUsers.Size = new System.Drawing.Size(67, 17);
            this.checkAllUsers.TabIndex = 3;
            this.checkAllUsers.Text = "All Users";
            this.checkAllUsers.UseVisualStyleBackColor = true;
            this.checkAllUsers.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(106, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Current User / Group";
            // 
            // add_bypass
            // 
            this.AcceptButton = this.OK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(455, 116);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.checkAllUsers);
            this.Controls.Add(this.butSelectUser);
            this.Controls.Add(this.CancelButton);
            this.Controls.Add(this.OK);
            this.Controls.Add(this.bypassCodeBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.UsernameLabel);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "add_bypass";
            this.ShowInTaskbar = false;
            this.Text = "New Bypass...";
            this.TopMost = true;
            this.Load += new System.EventHandler(this.add_bypass_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label UsernameLabel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox bypassCodeBox;
        private System.Windows.Forms.Button OK;
        private System.Windows.Forms.Button CancelButton;
        private System.Windows.Forms.Button butSelectUser;
        private System.Windows.Forms.CheckBox checkAllUsers;
        private System.Windows.Forms.Label label2;
    }
}