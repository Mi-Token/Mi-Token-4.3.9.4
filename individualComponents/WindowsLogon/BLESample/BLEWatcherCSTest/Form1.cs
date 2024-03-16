using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using System.Threading;


namespace BLEWatcherCSTest
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        public void writeToList(string str)
        {
            Invoke((MethodInvoker)delegate()
            {
                listBox1.Items.Add(str);
                listBox1.SelectedIndex = listBox1.Items.Count - 1;
            });
        }

        
        Thread watcherThread = null;
        private void button1_Click(object sender, EventArgs e)
        {
            WatcherCode watcher = new WatcherCode();

            if (watcher.connectToCOM())
            {
                listBox1.Items.Add("DLL Initialized");
                listBox1.Items.Add("Waiting for COM Access...");

                (watcherThread = new Thread(new ThreadStart(() =>
                    {
                        while (watcher.hasAccessToCOMPort(true) == false)
                        {
                            Invoke((MethodInvoker)delegate() 
                            {
                                listBox1.Items.Add("Holder unknown");
                            });
                            Thread.Sleep(1000);
                        }

                        Invoke((MethodInvoker)delegate()
                        {
                            listBox1.Items.Add("Access Granted");
                        });

                        watcher.startPoller();



                        while (true)
                        {
                            Invoke((MethodInvoker)delegate()
                            {
                                listBox1.Items.Add("Waiting for address to watch");
                            });

                            watcher.waitForWatcherPowerup();

                            Invoke((MethodInvoker)delegate()
                            {
                                listBox1.Items.Add("Watching an address");
                            });

                            watcher.mainWatching();

                            break;
                        }
                    }))).Start();
            }
            //Steps to get this up and running are:
            /*
             * Initialize the DLL (or keep trying every few seconds if we fail)
             * Create a callback for NP to tell us of client messages
             * Create filter object
             * 
             * 
             * 
             * 
             * 
             * 
             */
            
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (watcherThread != null)
            {
                watcherThread.Abort();
                BLE_API_DLL.DLLWrapper.V2.Finalize();
            }
        }
    }
}
