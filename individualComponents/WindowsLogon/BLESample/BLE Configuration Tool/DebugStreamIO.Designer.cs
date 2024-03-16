namespace BLE_Configuration_Tool
{
    partial class DebugStreamIO
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
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.labInputTime = new System.Windows.Forms.Label();
            this.labInputSize = new System.Windows.Forms.Label();
            this.labInputTotal = new System.Windows.Forms.Label();
            this.labOutputTotal = new System.Windows.Forms.Label();
            this.labOutputSize = new System.Windows.Forms.Label();
            this.labOutputTime = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // listBox1
            // 
            this.listBox1.FormattingEnabled = true;
            this.listBox1.Location = new System.Drawing.Point(12, 65);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(427, 238);
            this.listBox1.TabIndex = 0;
            this.listBox1.SelectedIndexChanged += new System.EventHandler(this.listBox1_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(67, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Input Stream";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 36);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Output Stream";
            // 
            // labInputTime
            // 
            this.labInputTime.AutoSize = true;
            this.labInputTime.Location = new System.Drawing.Point(121, 9);
            this.labInputTime.Name = "labInputTime";
            this.labInputTime.Size = new System.Drawing.Size(86, 13);
            this.labInputTime.TabIndex = 3;
            this.labInputTime.Text = "Last @ 00:00:00";
            // 
            // labInputSize
            // 
            this.labInputSize.AutoSize = true;
            this.labInputSize.Location = new System.Drawing.Point(230, 9);
            this.labInputSize.Name = "labInputSize";
            this.labInputSize.Size = new System.Drawing.Size(72, 13);
            this.labInputSize.TabIndex = 4;
            this.labInputSize.Text = "Last Size : 0B";
            // 
            // labInputTotal
            // 
            this.labInputTotal.AutoSize = true;
            this.labInputTotal.Location = new System.Drawing.Point(329, 9);
            this.labInputTotal.Name = "labInputTotal";
            this.labInputTotal.Size = new System.Drawing.Size(76, 13);
            this.labInputTotal.TabIndex = 5;
            this.labInputTotal.Text = "Total Size : 0B";
            // 
            // labOutputTotal
            // 
            this.labOutputTotal.AutoSize = true;
            this.labOutputTotal.Location = new System.Drawing.Point(329, 36);
            this.labOutputTotal.Name = "labOutputTotal";
            this.labOutputTotal.Size = new System.Drawing.Size(76, 13);
            this.labOutputTotal.TabIndex = 8;
            this.labOutputTotal.Text = "Total Size : 0B";
            // 
            // labOutputSize
            // 
            this.labOutputSize.AutoSize = true;
            this.labOutputSize.Location = new System.Drawing.Point(230, 36);
            this.labOutputSize.Name = "labOutputSize";
            this.labOutputSize.Size = new System.Drawing.Size(72, 13);
            this.labOutputSize.TabIndex = 7;
            this.labOutputSize.Text = "Last Size : 0B";
            // 
            // labOutputTime
            // 
            this.labOutputTime.AutoSize = true;
            this.labOutputTime.Location = new System.Drawing.Point(121, 36);
            this.labOutputTime.Name = "labOutputTime";
            this.labOutputTime.Size = new System.Drawing.Size(86, 13);
            this.labOutputTime.TabIndex = 6;
            this.labOutputTime.Text = "Last @ 00:00:00";
            // 
            // DebugStreamIO
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(451, 312);
            this.Controls.Add(this.labOutputTotal);
            this.Controls.Add(this.labOutputSize);
            this.Controls.Add(this.labOutputTime);
            this.Controls.Add(this.labInputTotal);
            this.Controls.Add(this.labInputSize);
            this.Controls.Add(this.labInputTime);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.listBox1);
            this.Name = "DebugStreamIO";
            this.Text = "DebugStreamIO";
            this.Load += new System.EventHandler(this.DebugStreamIO_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label labInputTime;
        private System.Windows.Forms.Label labInputSize;
        private System.Windows.Forms.Label labInputTotal;
        private System.Windows.Forms.Label labOutputTotal;
        private System.Windows.Forms.Label labOutputSize;
        private System.Windows.Forms.Label labOutputTime;
    }
}