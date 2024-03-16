using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Security.Cryptography;
using System.IO;



namespace CachedOTPServer
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        CacheOTPServerThread server = null;
        private void button1_Click(object sender, EventArgs e)
        {
            server = new CacheOTPServerThread(txtCustID.Text, 30000, txtServerID.Text);
            server.DebugInfo += new CacheOTPServerThread.DebugInfoHandle(server_DebugInfo);
            server.Start();
        }

        private delegate void dDebugInfo(object sender, string info);

        void server_DebugInfo(object sender, string info)
        {
            if (InvokeRequired)
            {
                BeginInvoke(new dDebugInfo(server_DebugInfo), sender, info);
                return;
            }

            listBox1.Items.Add(info);
            listBox1.SelectedIndex = listBox1.Items.Count - 1;
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        bool forceShutdown = false;
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            
            if (!server.SafeToShutdown())
            {
                if (!forceShutdown)
                {
                    forceShutdown = true;
                    server.Stop();
                    this.Text = "Waiting for server to close...";
                    System.Threading.Thread.Sleep(10000);
                    if (!server.SafeToShutdown())
                        server.Abort();
                }
                else if (forceShutdown)
                {
                    server.Abort();
                }

            }
        }


        void writeInt32(FileStream stream, int value)
        {
            byte[] data = new byte[4];
            for (int i = 0; i < 4; ++i)
            {
                data[i] = (byte)(value & 0xFF);
                value >>= 8;
            }

            stream.Write(data, 0, 4);
        }

        void writeByteArr(FileStream fstream, byte[] data)
        {
            writeInt32(fstream, data.Length);
            fstream.Write(data, 0, data.Length);
        }

        void WriteRSAParams(FileStream fstream, RSAParameters p, bool priv)
        {
            writeByteArr(fstream, p.Exponent);
            writeByteArr(fstream, p.Modulus);
            if (priv)
            {
                writeByteArr(fstream, p.D);
                writeByteArr(fstream, p.DP);
                writeByteArr(fstream, p.DQ);
                writeByteArr(fstream, p.InverseQ);
                writeByteArr(fstream, p.P);
                writeByteArr(fstream, p.Q);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            /*
            using (RSACryptoServiceProvider RSA = new RSACryptoServiceProvider())
            {
                RSAParameters RSAParams = RSA.ExportParameters(true);

                FileStream fStream = new FileStream("PrivKeys.key", FileMode.Create);
                WriteRSAParams(fStream, RSAParams, true);
                fStream.Close();

                fStream = new FileStream("PubKeys.key", FileMode.Create);
                WriteRSAParams(fStream, RSAParams, false);
                fStream.Close();
            }*/
        }

    }
}
