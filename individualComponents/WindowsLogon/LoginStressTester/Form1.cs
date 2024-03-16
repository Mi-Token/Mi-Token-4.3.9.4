using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;


namespace LoginStressTester
{
    public partial class Form1 : Form
    {
        StressController controller = new StressController();

        public Form1()
        {
            InitializeComponent();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            
            OpenFileDialog OFD = new OpenFileDialog();
            if (OFD.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                FileStream fs = new FileStream(OFD.FileName, FileMode.Open);
                byte[] bd = new byte[fs.Length];
                fs.Read(bd, 0, (int)fs.Length);
                fs.Close();
                string s = Encoding.ASCII.GetString(bd);
                string[] lines = s.Split(new string[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
                foreach (string line in lines)
                {
                    //The file format for temp tokens is as follows
                    //temp,[username],[token code]
                    string[] parts = line.Split(new string[] { "," }, StringSplitOptions.None);
                    if (parts[0].ToLower() == "temp")
                    {
                        if (parts.Length == 3)
                        {
                            controller.addTempTokenUser(parts[1], parts[2]);
                        }
                    }
                }
                //all users have been setup, finalize it.
                controller.finalizeUsers();
            }
        }

        private void butRunTests_Click(object sender, EventArgs e)
        {
            controller.setServerAddress(txtAddress.Text);
            DateTime start = DateTime.Now;

            List<DateTime> results = controller.RunStressTest(Convert.ToInt32(txtThreads.Text), Convert.ToInt32(txtRequest.Text), Convert.ToInt32(txtHOTP.Text), Convert.ToInt32(txtTOTP.Text));

            int secondsOff = 0;
            int curHits = 0;
            foreach (DateTime test in results)
            {
                bool repeat = true;

                while (repeat)
                {
                    if (((int)((test - start).TotalSeconds) < secondsOff))
                    {
                        curHits++;
                        repeat = false;
                    }
                    else
                    {
                        lstResults.Items.Add(string.Format("Second {0} : Hits {1}", secondsOff, curHits));
                        curHits = 0;
                        secondsOff++;
                    }
                }
            }
            lstResults.Items.Add(string.Format("Second {0} : Hits {1}", secondsOff, curHits));
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //Not yet done
        }
    }
}
