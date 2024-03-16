namespace BLE_Provisioning_Tool
{
    partial class UserControlBleDevice
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.buttonPairCreate = new System.Windows.Forms.Button();
            this.buttonPairRemove = new System.Windows.Forms.Button();
            this.groupBoxBleDev = new System.Windows.Forms.GroupBox();
            this.buttonRefresh = new System.Windows.Forms.Button();
            this.labelButtonProcessing = new System.Windows.Forms.Label();
            this.labelProvStatus = new System.Windows.Forms.Label();
            this.buttonUpdate = new System.Windows.Forms.Button();
            this.labelLastSeenHdr = new System.Windows.Forms.Label();
            this.labelTimeOfLastPollSeen = new System.Windows.Forms.Label();
            this.labelAddressHdr = new System.Windows.Forms.Label();
            this.labelAddress = new System.Windows.Forms.Label();
            this.buttonPing = new System.Windows.Forms.Button();
            this.labelMinor = new System.Windows.Forms.Label();
            this.labelMajor = new System.Windows.Forms.Label();
            this.labelDevUuid = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.progressBarRssi = new System.Windows.Forms.ProgressBar();
            this.buttonConfigure = new System.Windows.Forms.Button();
            this.groupBoxBleDev.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonPairCreate
            // 
            this.buttonPairCreate.Enabled = false;
            this.buttonPairCreate.Location = new System.Drawing.Point(375, 16);
            this.buttonPairCreate.Name = "buttonPairCreate";
            this.buttonPairCreate.Size = new System.Drawing.Size(65, 23);
            this.buttonPairCreate.TabIndex = 0;
            this.buttonPairCreate.Text = "Provision";
            this.buttonPairCreate.UseVisualStyleBackColor = true;
            this.buttonPairCreate.Click += new System.EventHandler(this.buttonPairCreate_Click);
            // 
            // buttonPairRemove
            // 
            this.buttonPairRemove.Enabled = false;
            this.buttonPairRemove.Location = new System.Drawing.Point(511, 45);
            this.buttonPairRemove.Name = "buttonPairRemove";
            this.buttonPairRemove.Size = new System.Drawing.Size(65, 23);
            this.buttonPairRemove.TabIndex = 1;
            this.buttonPairRemove.Text = "Reset";
            this.buttonPairRemove.UseVisualStyleBackColor = true;
            this.buttonPairRemove.Click += new System.EventHandler(this.buttonPairRemove_Click);
            // 
            // groupBoxBleDev
            // 
            this.groupBoxBleDev.BackColor = System.Drawing.SystemColors.Control;
            this.groupBoxBleDev.Controls.Add(this.buttonRefresh);
            this.groupBoxBleDev.Controls.Add(this.labelButtonProcessing);
            this.groupBoxBleDev.Controls.Add(this.labelProvStatus);
            this.groupBoxBleDev.Controls.Add(this.buttonUpdate);
            this.groupBoxBleDev.Controls.Add(this.labelLastSeenHdr);
            this.groupBoxBleDev.Controls.Add(this.labelTimeOfLastPollSeen);
            this.groupBoxBleDev.Controls.Add(this.labelAddressHdr);
            this.groupBoxBleDev.Controls.Add(this.labelAddress);
            this.groupBoxBleDev.Controls.Add(this.buttonPing);
            this.groupBoxBleDev.Controls.Add(this.labelMinor);
            this.groupBoxBleDev.Controls.Add(this.labelMajor);
            this.groupBoxBleDev.Controls.Add(this.labelDevUuid);
            this.groupBoxBleDev.Controls.Add(this.label5);
            this.groupBoxBleDev.Controls.Add(this.label4);
            this.groupBoxBleDev.Controls.Add(this.label3);
            this.groupBoxBleDev.Controls.Add(this.label1);
            this.groupBoxBleDev.Controls.Add(this.progressBarRssi);
            this.groupBoxBleDev.Controls.Add(this.buttonConfigure);
            this.groupBoxBleDev.Controls.Add(this.buttonPairCreate);
            this.groupBoxBleDev.Controls.Add(this.buttonPairRemove);
            this.groupBoxBleDev.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBoxBleDev.Location = new System.Drawing.Point(3, 3);
            this.groupBoxBleDev.Name = "groupBoxBleDev";
            this.groupBoxBleDev.Size = new System.Drawing.Size(582, 116);
            this.groupBoxBleDev.TabIndex = 3;
            this.groupBoxBleDev.TabStop = false;
            this.groupBoxBleDev.Text = "DEVICE NAME";
            // 
            // buttonRefresh
            // 
            this.buttonRefresh.Location = new System.Drawing.Point(444, 24);
            this.buttonRefresh.Name = "buttonRefresh";
            this.buttonRefresh.Size = new System.Drawing.Size(75, 37);
            this.buttonRefresh.TabIndex = 23;
            this.buttonRefresh.Text = "Refresh Services";
            this.buttonRefresh.UseVisualStyleBackColor = true;
            this.buttonRefresh.Click += new System.EventHandler(this.buttonRefresh_Click);
            // 
            // labelButtonProcessing
            // 
            this.labelButtonProcessing.Location = new System.Drawing.Point(375, 17);
            this.labelButtonProcessing.Name = "labelButtonProcessing";
            this.labelButtonProcessing.Size = new System.Drawing.Size(200, 51);
            this.labelButtonProcessing.TabIndex = 21;
            this.labelButtonProcessing.Text = "Discovering Services...";
            this.labelButtonProcessing.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // labelProvStatus
            // 
            this.labelProvStatus.AutoSize = true;
            this.labelProvStatus.Location = new System.Drawing.Point(210, 89);
            this.labelProvStatus.Name = "labelProvStatus";
            this.labelProvStatus.Size = new System.Drawing.Size(78, 13);
            this.labelProvStatus.TabIndex = 22;
            this.labelProvStatus.Text = "<Provisioned>";
            // 
            // buttonUpdate
            // 
            this.buttonUpdate.Enabled = false;
            this.buttonUpdate.Location = new System.Drawing.Point(444, 45);
            this.buttonUpdate.Name = "buttonUpdate";
            this.buttonUpdate.Size = new System.Drawing.Size(65, 23);
            this.buttonUpdate.TabIndex = 20;
            this.buttonUpdate.Text = "Update";
            this.buttonUpdate.UseVisualStyleBackColor = true;
            this.buttonUpdate.Click += new System.EventHandler(this.buttonUpdate_Click);
            // 
            // labelLastSeenHdr
            // 
            this.labelLastSeenHdr.AutoSize = true;
            this.labelLastSeenHdr.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelLastSeenHdr.Location = new System.Drawing.Point(455, 87);
            this.labelLastSeenHdr.Name = "labelLastSeenHdr";
            this.labelLastSeenHdr.Size = new System.Drawing.Size(58, 13);
            this.labelLastSeenHdr.TabIndex = 18;
            this.labelLastSeenHdr.Text = "Last Seen:";
            // 
            // labelTimeOfLastPollSeen
            // 
            this.labelTimeOfLastPollSeen.AutoSize = true;
            this.labelTimeOfLastPollSeen.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelTimeOfLastPollSeen.Location = new System.Drawing.Point(514, 87);
            this.labelTimeOfLastPollSeen.Name = "labelTimeOfLastPollSeen";
            this.labelTimeOfLastPollSeen.Size = new System.Drawing.Size(61, 13);
            this.labelTimeOfLastPollSeen.TabIndex = 16;
            this.labelTimeOfLastPollSeen.Text = "1 min+ ago";
            // 
            // labelAddressHdr
            // 
            this.labelAddressHdr.AutoSize = true;
            this.labelAddressHdr.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelAddressHdr.Location = new System.Drawing.Point(13, 23);
            this.labelAddressHdr.Name = "labelAddressHdr";
            this.labelAddressHdr.Size = new System.Drawing.Size(50, 13);
            this.labelAddressHdr.TabIndex = 15;
            this.labelAddressHdr.Text = "Address:";
            // 
            // labelAddress
            // 
            this.labelAddress.AutoSize = true;
            this.labelAddress.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelAddress.Location = new System.Drawing.Point(75, 23);
            this.labelAddress.Name = "labelAddress";
            this.labelAddress.Size = new System.Drawing.Size(99, 13);
            this.labelAddress.TabIndex = 4;
            this.labelAddress.Text = "XX:XX:XX:XX:XX:XX";
            // 
            // buttonPing
            // 
            this.buttonPing.Enabled = false;
            this.buttonPing.Location = new System.Drawing.Point(511, 16);
            this.buttonPing.Name = "buttonPing";
            this.buttonPing.Size = new System.Drawing.Size(65, 23);
            this.buttonPing.TabIndex = 14;
            this.buttonPing.Text = "Find";
            this.buttonPing.UseVisualStyleBackColor = true;
            this.buttonPing.Click += new System.EventHandler(this.buttonPing_Click);
            // 
            // labelMinor
            // 
            this.labelMinor.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelMinor.Location = new System.Drawing.Point(75, 89);
            this.labelMinor.Name = "labelMinor";
            this.labelMinor.Size = new System.Drawing.Size(50, 13);
            this.labelMinor.TabIndex = 13;
            this.labelMinor.Text = "65535";
            // 
            // labelMajor
            // 
            this.labelMajor.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelMajor.Location = new System.Drawing.Point(75, 67);
            this.labelMajor.Name = "labelMajor";
            this.labelMajor.Size = new System.Drawing.Size(50, 13);
            this.labelMajor.TabIndex = 12;
            this.labelMajor.Text = "65535";
            // 
            // labelDevUuid
            // 
            this.labelDevUuid.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelDevUuid.Location = new System.Drawing.Point(75, 45);
            this.labelDevUuid.Name = "labelDevUuid";
            this.labelDevUuid.Size = new System.Drawing.Size(253, 13);
            this.labelDevUuid.TabIndex = 11;
            this.labelDevUuid.Text = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(27, 89);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(37, 13);
            this.label5.TabIndex = 10;
            this.label5.Text = "Minor:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(27, 67);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(38, 13);
            this.label4.TabIndex = 9;
            this.label4.Text = "Major:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(27, 45);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(36, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "UUID:";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(328, 87);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(39, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Signal:";
            // 
            // progressBarRssi
            // 
            this.progressBarRssi.Location = new System.Drawing.Point(369, 82);
            this.progressBarRssi.Name = "progressBarRssi";
            this.progressBarRssi.Size = new System.Drawing.Size(77, 21);
            this.progressBarRssi.TabIndex = 5;
            // 
            // buttonConfigure
            // 
            this.buttonConfigure.Enabled = false;
            this.buttonConfigure.Location = new System.Drawing.Point(444, 16);
            this.buttonConfigure.Name = "buttonConfigure";
            this.buttonConfigure.Size = new System.Drawing.Size(65, 23);
            this.buttonConfigure.TabIndex = 3;
            this.buttonConfigure.Text = "Settings";
            this.buttonConfigure.UseVisualStyleBackColor = true;
            this.buttonConfigure.Click += new System.EventHandler(this.buttonConfigure_Click);
            // 
            // UserControlBleDevice
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.groupBoxBleDev);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "UserControlBleDevice";
            this.Size = new System.Drawing.Size(588, 126);
            this.groupBoxBleDev.ResumeLayout(false);
            this.groupBoxBleDev.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button buttonPairCreate;
        private System.Windows.Forms.Button buttonPairRemove;
        private System.Windows.Forms.GroupBox groupBoxBleDev;
        private System.Windows.Forms.ProgressBar progressBarRssi;
        private System.Windows.Forms.Button buttonConfigure;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label labelMinor;
        private System.Windows.Forms.Label labelMajor;
        private System.Windows.Forms.Label labelDevUuid;
        private System.Windows.Forms.Label labelAddress;
        private System.Windows.Forms.Button buttonPing;
        private System.Windows.Forms.Label labelAddressHdr;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label labelTimeOfLastPollSeen;
        private System.Windows.Forms.Label labelLastSeenHdr;
        private System.Windows.Forms.Button buttonUpdate;
        private System.Windows.Forms.Label labelButtonProcessing;
        private System.Windows.Forms.Label labelProvStatus;
        private System.Windows.Forms.Button buttonRefresh;
    }
}
