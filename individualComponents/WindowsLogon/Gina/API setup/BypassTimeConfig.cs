using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace API_setup
{
    public partial class BypassTimeConfig : UserControl
    {
        public delegate void updateClickedHandle();
        public event updateClickedHandle updateClicked;

        private void m_updateClicked() { if (updateClicked != null) { updateClicked(); } }

        class timeBypass
        {
            private byte[] values;
            public timeBypass()
            {
                values = new byte[21];
            }
            private bool getByteBit(int day, int hour, out int by, out int bit)
            {
                //validate input
                by = 0;
                bit = 0;
                if ((day > 6) || (day < 0) || (hour > 23) || (hour < 0))
                    return false;

                by = (day * 3 + (hour / 8));
                bit = hour % 8;
                if (by >= 21)
                    return false;
                if (by < 0)
                    return false;
                return true;

            }
            public bool IsEnabledAtTime(int day, int hour)
            {
                int by, bit;
                if (getByteBit(day, hour, out by, out bit))
                {
                    if ((values[by] & (1 << bit)) != 0)
                        return true;
                    else
                        return false;
                }
                return false;
            }

            public void SetAtTime(int day, int hour, bool enabled)
            {
                int by, bit;
                if (getByteBit(day, hour, out by, out bit))
                {
                    if (enabled)
                    {
                        values[by] |= (byte)(1 << bit);
                    }
                    else
                    {
                        values[by] &= (byte)(~(1 << bit));
                    }
                }
            }

            public void DefaultSetting(bool enabled)
            {
                byte v = (byte)(enabled ? 0xFF : 0x00);
                for (int i = 0; i < values.Length; ++i)
                    values[i] = v;
            }

            public byte[] table()
            {
                return values;
            }
            public void setTable(byte[] table)
            {
                if (table.Length == 21)
                {
                    Array.Copy(table, values, 21);
                }
            }

        }
        timeBypass TB;

        public byte[] getTimeTable()
        {
            return TB.table();
        }
        public BypassTimeConfig()
        {
            InitializeComponent();
            TB = new timeBypass();
            TB.DefaultSetting(true);
            radioButton3.Checked = true;
            drawEnabledTimes();
        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

    

        void drawEnabledTimes()
        {
            int boxHeight = pictureBox1.Height / 7;
            int boxWidth = pictureBox1.Width / 24;
            Bitmap pb1 = new Bitmap(pictureBox1.Width, pictureBox1.Height);
            Graphics g = Graphics.FromImage(pb1);
            g.Clear(Color.White);
            for (int day = 0; day < 7; ++day)
            {
                for (int hour = 0; hour < 24; ++hour)
                {
                    bool enabled = TB.IsEnabledAtTime(day, hour);
                    if (enabled)
                    {
                        g.FillRectangle(Brushes.Blue, new Rectangle(hour * boxWidth, day * boxHeight, boxWidth, boxHeight));
                    }
                }
            }
            
            for (int i = 0; i < 25; ++i)
            {
                g.DrawLine(Pens.Black, new Point(i * boxWidth, 0), new Point(i * boxWidth, pictureBox1.Height));
                if (i % 3 == 0)
                {
                    g.DrawLine(Pens.Black, new Point(i * boxWidth - 1, 0), new Point(i * boxWidth - 1, pictureBox1.Height));
                    g.DrawLine(Pens.Black, new Point(i * boxWidth + 1, 0), new Point(i * boxWidth + 1, pictureBox1.Height));
                }
            }
            for (int i = 0; i < 8; ++i)
            {
                g.DrawLine(Pens.Black, new Point(0, i * boxHeight), new Point(pictureBox1.Width, i * boxHeight));
            }
            
            pictureBox1.Image = pb1;

        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            int boxHeight = pictureBox1.Height / 7;
            int boxWidth = pictureBox1.Width / 24;
            int hour, day;
            
            hour = e.X / boxWidth;
            day = e.Y / boxHeight;

            bool enabled = TB.IsEnabledAtTime(day, hour);
            setToEnabled = !enabled;
            setOnMouseMove = true;
            TB.SetAtTime(day, hour, !enabled);
            radioButton1.Checked = false;
            radioButton2.Checked = false;
            drawEnabledTimes();
        }
        bool setOnMouseMove = false;
        bool setToEnabled = false;
        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
        {
            int boxHeight = pictureBox1.Height / 7;
            int boxWidth = pictureBox1.Width / 24;
            int hour, day;

            hour = e.X / boxWidth;
            day = e.Y / boxHeight;
            if ((day < 0) || (day > 6) || (hour < 0) || (hour > 23))
                return;
            bool enabled = TB.IsEnabledAtTime(day, hour);
            string[] sarr = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
            if (day > 6)
                return;
            label1.Text = string.Format("{0} : {1}:00 - {1}:59 is {2}", sarr[day], hour, (enabled ? "ENABLED" : "DISABLED"));

            if (setOnMouseMove)
            {
                if (TB.IsEnabledAtTime(day, hour) != setToEnabled)
                {
                    TB.SetAtTime(day, hour, setToEnabled);
                    drawEnabledTimes();
                }
            }
        }

        private void pictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
            setOnMouseMove = false;
        }

        private void BypassEnabledTimes_Resize(object sender, EventArgs e)
        {
            Rectangle cr = this.ClientRectangle;
            int boxHeight = ((label1.Top)) / 7;
            int boxWidth = cr.Width / 24;
            pictureBox1.Width = 1 + (boxWidth * 24);
            pictureBox1.Height = 1 + (boxHeight * 7);
            drawEnabledTimes();
        }

        public void loadTime(byte[] times)
        {
            radioButton1.Checked = false;
            radioButton2.Checked = false;
            radioButton3.Checked = true;
            TB.setTable(times);
            
            drawEnabledTimes();
        }
        private void button1_Click(object sender, EventArgs e)
        {
            m_updateClicked();
        }   

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            
            if (radioButton1.Checked)
            {
                TB.DefaultSetting(true);
                drawEnabledTimes();
            }
            

        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton2.Checked)
            {
                TB.DefaultSetting(false);
                drawEnabledTimes();
            }
        }

        private void BypassTimeConfig_Load(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }
    }
}
