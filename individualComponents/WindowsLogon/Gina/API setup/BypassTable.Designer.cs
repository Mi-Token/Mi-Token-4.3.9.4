namespace API_setup
{
    partial class BypassTable
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
            this.OKButton = new System.Windows.Forms.Button();
            this.removeButton = new System.Windows.Forms.Button();
            this.AddButton = new System.Windows.Forms.Button();
            this.BypassCode = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Username = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.bypassGridView = new System.Windows.Forms.DataGridView();
            this.butBypassEnabledTimes = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.bypassGridView)).BeginInit();
            this.SuspendLayout();
            // 
            // OKButton
            // 
            this.OKButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.OKButton.Location = new System.Drawing.Point(366, 314);
            this.OKButton.Name = "OKButton";
            this.OKButton.Size = new System.Drawing.Size(75, 23);
            this.OKButton.TabIndex = 9;
            this.OKButton.Text = "OK";
            this.OKButton.UseVisualStyleBackColor = true;
            this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
            // 
            // removeButton
            // 
            this.removeButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.removeButton.Location = new System.Drawing.Point(84, 314);
            this.removeButton.Name = "removeButton";
            this.removeButton.Size = new System.Drawing.Size(159, 23);
            this.removeButton.TabIndex = 8;
            this.removeButton.Text = "Remove all user bypass codes";
            this.removeButton.UseVisualStyleBackColor = true;
            this.removeButton.Click += new System.EventHandler(this.removeButton_Click);
            // 
            // AddButton
            // 
            this.AddButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.AddButton.Location = new System.Drawing.Point(3, 314);
            this.AddButton.Name = "AddButton";
            this.AddButton.Size = new System.Drawing.Size(75, 23);
            this.AddButton.TabIndex = 7;
            this.AddButton.Text = "Add...";
            this.AddButton.UseVisualStyleBackColor = true;
            this.AddButton.Click += new System.EventHandler(this.AddButton_Click);
            // 
            // BypassCode
            // 
            this.BypassCode.HeaderText = "Bypass Code";
            this.BypassCode.Name = "BypassCode";
            this.BypassCode.ReadOnly = true;
            // 
            // Username
            // 
            this.Username.HeaderText = "User";
            this.Username.Name = "Username";
            this.Username.ReadOnly = true;
            // 
            // bypassGridView
            // 
            this.bypassGridView.AllowUserToAddRows = false;
            this.bypassGridView.AllowUserToDeleteRows = false;
            this.bypassGridView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.bypassGridView.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.bypassGridView.BackgroundColor = System.Drawing.SystemColors.ControlLight;
            this.bypassGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.bypassGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Username,
            this.BypassCode});
            this.bypassGridView.Location = new System.Drawing.Point(3, 3);
            this.bypassGridView.Name = "bypassGridView";
            this.bypassGridView.ReadOnly = true;
            this.bypassGridView.RowHeadersVisible = false;
            this.bypassGridView.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.bypassGridView.ShowRowErrors = false;
            this.bypassGridView.Size = new System.Drawing.Size(438, 305);
            this.bypassGridView.TabIndex = 6;
            this.bypassGridView.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.bypassGridView_CellContentClick_1);
            // 
            // butBypassEnabledTimes
            // 
            this.butBypassEnabledTimes.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.butBypassEnabledTimes.Enabled = false;
            this.butBypassEnabledTimes.Location = new System.Drawing.Point(249, 314);
            this.butBypassEnabledTimes.Name = "butBypassEnabledTimes";
            this.butBypassEnabledTimes.Size = new System.Drawing.Size(111, 23);
            this.butBypassEnabledTimes.TabIndex = 10;
            this.butBypassEnabledTimes.Text = "Enabled Times";
            this.butBypassEnabledTimes.UseVisualStyleBackColor = true;
            this.butBypassEnabledTimes.Click += new System.EventHandler(this.butBypassEnabledTimes_Click);
            // 
            // BypassTable
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.butBypassEnabledTimes);
            this.Controls.Add(this.OKButton);
            this.Controls.Add(this.removeButton);
            this.Controls.Add(this.AddButton);
            this.Controls.Add(this.bypassGridView);
            this.Name = "BypassTable";
            this.Size = new System.Drawing.Size(444, 345);
            this.Load += new System.EventHandler(this.BypassTable_Load);
            ((System.ComponentModel.ISupportInitialize)(this.bypassGridView)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button OKButton;
        private System.Windows.Forms.Button removeButton;
        private System.Windows.Forms.Button AddButton;
        private System.Windows.Forms.DataGridViewTextBoxColumn BypassCode;
        private System.Windows.Forms.DataGridViewTextBoxColumn Username;
        private System.Windows.Forms.DataGridView bypassGridView;
        private System.Windows.Forms.Button butBypassEnabledTimes;
    }
}
