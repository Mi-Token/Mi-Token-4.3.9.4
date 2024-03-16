namespace API_Autoconfig
{
    partial class Form2
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
            this.butAddToAutosetup = new System.Windows.Forms.Button();
            this.butLoadAutoconfig = new System.Windows.Forms.Button();
            this.butCreateACF = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // butAddToAutosetup
            // 
            this.butAddToAutosetup.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.butAddToAutosetup.Location = new System.Drawing.Point(359, 346);
            this.butAddToAutosetup.Name = "butAddToAutosetup";
            this.butAddToAutosetup.Size = new System.Drawing.Size(138, 32);
            this.butAddToAutosetup.TabIndex = 15;
            this.butAddToAutosetup.Text = "Add Config File to Installer";
            this.butAddToAutosetup.UseVisualStyleBackColor = true;
            this.butAddToAutosetup.Click += new System.EventHandler(this.butAddToAutosetup_Click);
            // 
            // butLoadAutoconfig
            // 
            this.butLoadAutoconfig.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.butLoadAutoconfig.Location = new System.Drawing.Point(71, 346);
            this.butLoadAutoconfig.Name = "butLoadAutoconfig";
            this.butLoadAutoconfig.Size = new System.Drawing.Size(138, 32);
            this.butLoadAutoconfig.TabIndex = 14;
            this.butLoadAutoconfig.Text = "Load Autoconfig File";
            this.butLoadAutoconfig.UseVisualStyleBackColor = true;
            this.butLoadAutoconfig.Click += new System.EventHandler(this.button2_Click);
            // 
            // butCreateACF
            // 
            this.butCreateACF.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.butCreateACF.Location = new System.Drawing.Point(215, 347);
            this.butCreateACF.Name = "butCreateACF";
            this.butCreateACF.Size = new System.Drawing.Size(138, 32);
            this.butCreateACF.TabIndex = 13;
            this.butCreateACF.Text = "Create Autoconfig File";
            this.butCreateACF.UseVisualStyleBackColor = true;
            this.butCreateACF.Click += new System.EventHandler(this.butCreateACF_Click);
            // 
            // Form2
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(509, 390);
            this.Controls.Add(this.butAddToAutosetup);
            this.Controls.Add(this.butLoadAutoconfig);
            this.Controls.Add(this.butCreateACF);
            this.Name = "Form2";
            this.Text = "Auto-configuration Utility";
            this.Load += new System.EventHandler(this.Form2_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button butAddToAutosetup;
        private System.Windows.Forms.Button butLoadAutoconfig;
        private System.Windows.Forms.Button butCreateACF;

    }
}