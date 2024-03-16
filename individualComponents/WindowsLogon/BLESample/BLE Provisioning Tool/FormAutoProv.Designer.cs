namespace BLE_Provisioning_Tool
{
    partial class FormAutoProv
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormAutoProv));
            this.labelMajor = new System.Windows.Forms.Label();
            this.labelMinor = new System.Windows.Forms.Label();
            this.labelUuid = new System.Windows.Forms.Label();
            this.buttonApply = new System.Windows.Forms.Button();
            this.textBoxUuid = new System.Windows.Forms.TextBox();
            this.textBoxMajor = new System.Windows.Forms.TextBox();
            this.textBoxMinor = new System.Windows.Forms.TextBox();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.labelAdvertInterval = new System.Windows.Forms.Label();
            this.textBoxAdvertInterval = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // labelMajor
            // 
            this.labelMajor.AutoSize = true;
            this.labelMajor.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelMajor.Location = new System.Drawing.Point(77, 41);
            this.labelMajor.Name = "labelMajor";
            this.labelMajor.Size = new System.Drawing.Size(62, 13);
            this.labelMajor.TabIndex = 0;
            this.labelMajor.Text = "Major (dec)";
            // 
            // labelMinor
            // 
            this.labelMinor.AutoSize = true;
            this.labelMinor.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelMinor.Location = new System.Drawing.Point(76, 67);
            this.labelMinor.Name = "labelMinor";
            this.labelMinor.Size = new System.Drawing.Size(61, 13);
            this.labelMinor.TabIndex = 1;
            this.labelMinor.Text = "Minor (dec)";
            // 
            // labelUuid
            // 
            this.labelUuid.AutoSize = true;
            this.labelUuid.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelUuid.Location = new System.Drawing.Point(77, 15);
            this.labelUuid.Name = "labelUuid";
            this.labelUuid.Size = new System.Drawing.Size(61, 13);
            this.labelUuid.TabIndex = 2;
            this.labelUuid.Text = "UUID (hex)";
            // 
            // buttonApply
            // 
            this.buttonApply.Location = new System.Drawing.Point(141, 119);
            this.buttonApply.Name = "buttonApply";
            this.buttonApply.Size = new System.Drawing.Size(75, 23);
            this.buttonApply.TabIndex = 5;
            this.buttonApply.Text = "Apply";
            this.buttonApply.UseVisualStyleBackColor = true;
            this.buttonApply.Click += new System.EventHandler(this.buttonApply_Click);
            // 
            // textBoxUuid
            // 
            this.textBoxUuid.Location = new System.Drawing.Point(143, 12);
            this.textBoxUuid.Name = "textBoxUuid";
            this.textBoxUuid.Size = new System.Drawing.Size(245, 21);
            this.textBoxUuid.TabIndex = 1;
            this.textBoxUuid.Validating += new System.ComponentModel.CancelEventHandler(this.textBoxUuid_Validating);
            this.textBoxUuid.Validated += new System.EventHandler(this.textBoxUuid_Validated);
            // 
            // textBoxMajor
            // 
            this.textBoxMajor.Location = new System.Drawing.Point(143, 38);
            this.textBoxMajor.Name = "textBoxMajor";
            this.textBoxMajor.Size = new System.Drawing.Size(100, 21);
            this.textBoxMajor.TabIndex = 2;
            this.textBoxMajor.Validating += new System.ComponentModel.CancelEventHandler(this.textBoxMajor_Validating);
            this.textBoxMajor.Validated += new System.EventHandler(this.textBoxMajor_Validated);
            // 
            // textBoxMinor
            // 
            this.textBoxMinor.Location = new System.Drawing.Point(143, 64);
            this.textBoxMinor.Name = "textBoxMinor";
            this.textBoxMinor.Size = new System.Drawing.Size(100, 21);
            this.textBoxMinor.TabIndex = 3;
            this.textBoxMinor.Validating += new System.ComponentModel.CancelEventHandler(this.textBoxMinor_Validating);
            this.textBoxMinor.Validated += new System.EventHandler(this.textBoxMinor_Validated);
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(222, 119);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 6;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // labelAdvertInterval
            // 
            this.labelAdvertInterval.AutoSize = true;
            this.labelAdvertInterval.Location = new System.Drawing.Point(12, 95);
            this.labelAdvertInterval.Name = "labelAdvertInterval";
            this.labelAdvertInterval.Size = new System.Drawing.Size(126, 13);
            this.labelAdvertInterval.TabIndex = 6;
            this.labelAdvertInterval.Text = "Advertising Interval (ms)";
            // 
            // textBoxAdvertInterval
            // 
            this.textBoxAdvertInterval.Location = new System.Drawing.Point(143, 92);
            this.textBoxAdvertInterval.Name = "textBoxAdvertInterval";
            this.textBoxAdvertInterval.Size = new System.Drawing.Size(100, 21);
            this.textBoxAdvertInterval.TabIndex = 4;
            this.textBoxAdvertInterval.Validating += new System.ComponentModel.CancelEventHandler(this.textBoxAdvertInterval_Validating);
            this.textBoxAdvertInterval.Validated += new System.EventHandler(this.textBoxAdvertInterval_Validated);
            // 
            // FormAutoProv
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(408, 149);
            this.Controls.Add(this.textBoxAdvertInterval);
            this.Controls.Add(this.labelAdvertInterval);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.textBoxMinor);
            this.Controls.Add(this.textBoxMajor);
            this.Controls.Add(this.textBoxUuid);
            this.Controls.Add(this.buttonApply);
            this.Controls.Add(this.labelUuid);
            this.Controls.Add(this.labelMinor);
            this.Controls.Add(this.labelMajor);
            this.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "FormAutoProv";
            this.ShowInTaskbar = false;
            this.Text = "Auto-Provisioning";
            this.TopMost = true;
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelMajor;
        private System.Windows.Forms.Label labelMinor;
        private System.Windows.Forms.Label labelUuid;
        private System.Windows.Forms.Button buttonApply;
        private System.Windows.Forms.TextBox textBoxUuid;
        private System.Windows.Forms.TextBox textBoxMajor;
        private System.Windows.Forms.TextBox textBoxMinor;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label labelAdvertInterval;
        private System.Windows.Forms.TextBox textBoxAdvertInterval;
    }
}