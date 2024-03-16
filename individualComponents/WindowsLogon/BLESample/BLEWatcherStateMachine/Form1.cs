#define USE_DOUBLE_LISTBOX
#define USE_DOUBLE_TREEVIEW

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Threading;

namespace BLEWatcherStateMachine
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
#if USE_DOUBLE_LISTBOX
            bufferedLB1 = new DoubleBufferedListbox();
            bufferedLB1.Location = listBox1.Location;
            bufferedLB1.Size = listBox1.Size;
            this.Controls.Add(bufferedLB1);
            bufferedLB1.Visible = true;
            bufferedLB1.Anchor = listBox1.Anchor;
            listBox1.Visible = false;
#endif

#if USE_DOUBLE_TREEVIEW
            bufferedTV1 = new DoubleBufferedTreeview();
            bufferedTV1.Location = treeView1.Location;
            bufferedTV1.Size = treeView1.Size;
            this.Controls.Add(bufferedTV1);
            bufferedTV1.Visible = true;
            bufferedTV1.Anchor = treeView1.Anchor;
            treeView1.Visible = false;
#endif
        }

        StateMachine machine = null;

        private void SetDoubleBuffering(Control control, bool value)
        {
            System.Reflection.PropertyInfo controlProperty = typeof(System.Windows.Forms.Control).GetProperty("DoubleBuffered", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
            controlProperty.SetValue(control, value, null);
        }

        DoubleBufferedListbox bufferedLB1;
        DoubleBufferedTreeview bufferedTV1;

        private void Form1_Load(object sender, EventArgs e)
        {
            this.SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint | ControlStyles.AllPaintingInWmPaint, true);
            this.DoubleBuffered = true;
            SetDoubleBuffering(listBox1, true);
            SetDoubleBuffering(treeView1, true);
            if (!Program.delayStart)
            {
                Thread T = new Thread(new ThreadStart(() =>
                    {
                        if (Thread.CurrentThread.Name == null)
                        {
                            Thread.CurrentThread.Name = "Delayed Form1 Load Thread";
                        }

                        Thread.Sleep(100);
                        Invoke((MethodInvoker)delegate
                        {
                            button2_Click(null, new EventArgs());
                        });
                    }));
                T.Start();
            }
        }



        void machine_debugMessage(string strSender, string message)
        {
            if (InvokeRequired)
            {
                BeginInvoke((MethodInvoker)delegate
                {
                    machine_debugMessage(strSender, message);
                });
                return;
            }
#if USE_DOUBLE_LISTBOX
            bufferedLB1.BeginUpdate();
            bufferedLB1.Items.Add(string.Format("{0} : {1}", strSender, message));
            bufferedLB1.SelectedIndex = bufferedLB1.Items.Count - 1;
            bufferedLB1.EndUpdate();
#else
            listBox1.BeginUpdate();
            listBox1.Items.Add(string.Format("{0} : {1}", strSender, message));
            //listBox1.SelectedIndex = listBox1.Items.Count - 1;
            listBox1.EndUpdate();
#endif
        }

        private void button1_Click(object sender, EventArgs e)
        {
            machine.V2_1_MiTokenBLE_GotMessageCallback(null, new byte[] { (byte)StateMachine.DATA_FLAG.CP_ONLINE });
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Thread T = new Thread(new ThreadStart(() =>
            {
                if (Thread.CurrentThread.Name == null)
                {
                    Thread.CurrentThread.Name = "Machine State Updater";
                }

                Thread.Sleep(1000);
                machine = new StateMachine();
                machine.debugMessage += machine_debugMessage;
                machine.Initialize();
                machine.StartMachine();


                int j = 0;
                while (machine.myData.running)
                {
                    j++;
                    byte[] rtime = new byte[1];
                    machine.passwordHandler.crng.GetBytes(rtime);
                    Thread.Sleep(100);
                    TreeNode newNode;
                    machine.populateNode(out newNode);
                    Invoke((MethodInvoker)delegate
                    {
#if USE_DOUBLE_TREEVIEW
                        bufferedTV1.BeginUpdate();
                        bufferedTV1.Nodes.Clear();
                        bufferedTV1.Nodes.Add(newNode);
                        bufferedTV1.ExpandAll();
                        bufferedTV1.EndUpdate();
#else
                        treeView1.Nodes.Clear();
                        treeView1.Nodes.Add(newNode);
                        treeView1.ExpandAll();
#endif
                    });

                }
            }));
            T.Start();
        }

        private byte[] strToByteArr(string str)
        {
            byte[] encBytes = Encoding.ASCII.GetBytes(str);
            byte len = (byte)encBytes.Length;

            byte[] output = new byte[len + 1];
            output[0] = len;
            Array.Copy(encBytes, 0, output, 1, len);

            return output;
        }


        private void button3_Click(object sender, EventArgs e)
        {
            ByteBuilder bb = new ByteBuilder();
            bb.Add((byte)StateMachine.DATA_FLAG.STORE_PASSWORD);
            bb.Add(strToByteArr(txtUsername.Text));
            bb.Add(strToByteArr(txtPassword.Text));

            machine.V2_1_MiTokenBLE_GotMessageCallback(null, bb.ToArray);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            machine.myData.ResetToWaitForWakeupState = true;
            machine.myData.ForceStateUpdate.Set();
        }

        private void button5_Click(object sender, EventArgs e)
        {
            BLEDebug debugForm = new BLEDebug(machine.myData.ble);
            debugForm.Show();
        }

        private void button6_Click(object sender, EventArgs e)
        {
            machine.StopMachine();
        }

        private void button7_Click(object sender, EventArgs e)
        {
            machine.myData._PreEmptive._allowPreEmptiveLogins = true;
        }

    }
}
