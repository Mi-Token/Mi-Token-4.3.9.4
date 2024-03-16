using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Pipes;
using System.Threading;

namespace MemLeakServer
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }



        AutoResetEvent ARE = new AutoResetEvent(true);
        private void Form1_Load(object sender, EventArgs e)
        {

        }
        class MemData
        {
            public int allocSize;
            public int fileLine;
            public int blockUseID;
            public int sectionID;
            public bool refreshed;
            public string filepath;
        };

        Dictionary<long, MemData> memMap = new Dictionary<long, MemData>();
        Dictionary<int, int> memAllocSections = new Dictionary<int, int>();
        Dictionary<int, int> oldMemAllocSections = new Dictionary<int, int>();

        public int memAllocated = 0;
        void startRefresh()
        {
            foreach (MemData md in memMap.Values)
            {
                md.refreshed = false;
            }
        }

        void endRefresh()
        {
            //lock for this entire function
            ARE.WaitOne();

            List<long> remList = new List<long>();
            foreach (long l in memMap.Keys)
            {
                if (!memMap[l].refreshed)
                {
                    remList.Add(l);
                }
            }
            foreach (long l in remList)
            {
                memAllocated -= (memMap[l].allocSize);
                memAllocSections[memMap[l].sectionID] -= memMap[l].allocSize;
                memMap.Remove(l);
            }

            ARE.Set();
        }


        NamedPipeServerStream NPSS;
        Thread pipeThread;
        private void button1_Click(object sender, EventArgs e)
        {
            button1.Enabled = false;
            //NPSS = new NamedPipeServerStream("\\\\.\\pipe\\MTMemDebug", PipeDirection.InOut, 20, PipeTransmissionMode.Byte, PipeOptions.WriteThrough, 8192, 8192);
            NPSS = new NamedPipeServerStream("MTMemDebug", System.IO.Pipes.PipeDirection.InOut, 4);
            pipeThread = new Thread(new ThreadStart(threadFunction));
            pipeThread.Start();
            timer1.Enabled = true;
            button2.Enabled = true;
        }

        int currentSectionID = -1;
        private delegate void d_debugPrint(string message);
        private void debugPrint(string message)
        {
            if (InvokeRequired)
            {
                Invoke(new d_debugPrint(debugPrint), message);
                return;
            }

            //listBox1.Items.Add(message);
            //listBox1.SelectedIndex = listBox1.Items.Count - 1;
        }

        private delegate void d_memAllocDebug(string message, string message2);
        private void memAllocDebug(string message, string message2 = "")
        {
            if (InvokeRequired)
            {
                Invoke(new d_memAllocDebug(memAllocDebug), message);
                return;
            }
            if (message == "CLEARALL")
            {
                listBox2.Items.Clear();
            }
            else
            {
                if (message != "")
                {
                    listBox2.Items.Add(message);
                    listBox2.SelectedIndex = listBox2.Items.Count - 1;
                }
            }

            if (message2 == "CLEARALL")
            {
                listBox1.Items.Clear();
            }
            else
            {
                if (message2 != "")
                {
                    listBox1.Items.Add(message2);
                }
            }
        }

        private string getHeaderStr(ref byte[] data)
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < 4; ++i)
                sb.Append((char)data[i]);

            return sb.ToString();
        }

        private byte[] removeBytes(byte[] src, int bytesToRemove)
        {
            if(bytesToRemove >= src.Length)
                return new byte[0];

            byte[] ret = new byte[src.Length - bytesToRemove];
            Array.Copy(src, bytesToRemove, ret, 0, ret.Length);

            return ret;
        }

        private Int32 getInt32FromBytes(byte[] src, int location)
        {
            Int32 r = 0;
            for (int i = 0; i < 4; ++i)
            {
                r <<= 8;
                r += (src[location + (3 - i)]);
            }
            return r;
        }

        private void addMemData(byte[] buffer)
        {
            
            Int32 ptrSize = getInt32FromBytes(buffer, 8);
            long ptr = 0;
            int offset = 12;
            if(ptrSize == 4)
            {
                ptr = (long)getInt32FromBytes(buffer, offset);
            }
            else if (ptrSize == 8)
            {
                offset += 4; //padding
                ptr = BitConverter.ToInt64(buffer, offset);
            }
            else
            {
                throw new Exception("Unexpected pointer size of " + ptrSize);
            }
            offset += ptrSize;

            ARE.WaitOne();

            MemData md = new MemData();
            md.allocSize = getInt32FromBytes(buffer, offset);
            offset += 4;
            md.fileLine = getInt32FromBytes(buffer, offset);
            offset += 4;
            md.blockUseID = getInt32FromBytes(buffer, offset);
            offset += 4;
            byte[] maxName = new byte[100];
            Array.Copy(buffer, offset, maxName, 0, Math.Min(100, buffer.Length - offset));
            string s = Encoding.ASCII.GetString(maxName);
            if (s.Contains('\0'))
            {
                s = s.Substring(0, s.IndexOf('\0'));
            }

            md.refreshed = true;
            md.sectionID = currentSectionID;
            md.filepath = s;
            if(memMap.ContainsKey(ptr))
            {
                memAllocated -= memMap[ptr].allocSize;
                memAllocSections[memMap[ptr].sectionID] -= memMap[ptr].allocSize;
                md.sectionID = memMap[ptr].sectionID;
            }
            memMap[ptr] = md;
            memAllocated += md.allocSize;
            if (!memAllocSections.ContainsKey(md.sectionID))
            {
                memAllocSections.Add(md.sectionID, 0);
            }
            memAllocSections[md.sectionID] += md.allocSize;

            ARE.Set();
        }

        private void threadFunction()
        {
            debugPrint("Starting thread function...");
            while (true)
            {
                debugPrint("Waiting for client pipe");
                NPSS.ReadMode = PipeTransmissionMode.Byte;
                NPSS.WaitForConnection();
                debugPrint("Client connected");
                debugPrint("Starting main loop");
                byte[] curBuffer = new byte[0];
                while (true)
                {
                    byte[] buffer = new byte[8192]; //8k buffer
                    int bytesRead = NPSS.Read(buffer, 0, 8192);
                    if (bytesRead == 0)
                    {
                        debugPrint("Got EOF from pipe");
                        break;
                    }
                    //append the new bytes read onto the end of the bytes already in the buffer
                    byte[] tbuf = new byte[bytesRead + curBuffer.Length];
                    Array.Copy(curBuffer, tbuf, curBuffer.Length);
                    Array.Copy(buffer, 0, tbuf, curBuffer.Length, bytesRead);
                    curBuffer = tbuf;

                    if (curBuffer.Length < 4)
                        continue; //cannot do anything unless we have atleast 4 bytes

                    bool moreData = true;
                    while (moreData)
                    {
                        if (curBuffer.Length < 4)
                            break;
                        string headerStr = getHeaderStr(ref curBuffer);
                        switch (headerStr)
                        {
                            case "UPMD":
                                //update memory data
                                debugPrint("Received Update Memory Data Packet");
                                curBuffer = removeBytes(curBuffer, 4);
                                startRefresh();
                                break;
                            case "EUPD":
                                //end update memory data
                                debugPrint("Recevied End Update Memory Data Packet");
                                curBuffer = removeBytes(curBuffer, 4);
                                endRefresh();
                                break;
                            case "ULID":
                                //update LID
                                debugPrint("Received Update LID Packet");
                                if (curBuffer.Length < 8)
                                {
                                    debugPrint("Incomplete ULID Packet... waiting for more data");
                                    moreData = false;
                                    break;
                                }
                                Int32 nInt = getInt32FromBytes(curBuffer, 4);
                                currentSectionID = nInt;
                                curBuffer = removeBytes(curBuffer, 8);
                                break;
                            case "MEMS":
                                //Memory Section
                                debugPrint("Received Memory Section Packet");
                                if (curBuffer.Length < 8)
                                {
                                    debugPrint("Incomplete MEMS Packet... waiting for more data");
                                    moreData = true;
                                    break;
                                }
                                Int32 packetLen = getInt32FromBytes(curBuffer, 4);
                                if (curBuffer.Length < packetLen)
                                {
                                    debugPrint("Incomplete MEMS Packet... waiting for more data");
                                    moreData = true;
                                    break;
                                }
                                addMemData(curBuffer);
                                curBuffer = removeBytes(curBuffer, packetLen);
                                break;
                            default:
                                //unknown header string
                                debugPrint("Unknown header string : " + headerStr);
                                break;
                        }
                    }
                }
                NPSS.Close();
                //restart the memMap dictionary, and say no memory is allocated
                memMap = new Dictionary<long, MemData>();
                memAllocated = 0;
                NPSS = new NamedPipeServerStream("MTMemDebug", System.IO.Pipes.PipeDirection.InOut, 4);
            }

        }

        private delegate void d_updateMemInfo(int hID, int alloc, int offset);
        private void updateMemInfo(int hID, int alloc, int offset)
        {
            if (InvokeRequired)
            {
                Invoke(new d_updateMemInfo(updateMemInfo), hID, alloc, offset);
                return;
            }

            string s = string.Format("[{3}] : {0} ({1}{2})", alloc, (offset >= 0 ? "+" : "-"), offset, hID);
            labMemAlloc.Text = s;
        }

        int HID = 0;
        int lastAlloc = 0;
        private void timer1_Tick(object sender, EventArgs e)
        {
            HID++;
            int ma = memAllocated;
            int mo = memAllocated - lastAlloc;
            updateMemInfo(HID, ma, mo);
            lastAlloc = ma;

            ARE.WaitOne();

            //hacky way to get the clear all without needing a new delegate
            memAllocDebug("CLEARALL", "CLEARALL");

            foreach (int i in memAllocSections.Keys)
            {
                ma = memAllocSections[i];
                mo = 0;
                if (oldMemAllocSections.ContainsKey(i))
                {
                    mo = oldMemAllocSections[i];
                }
                else
                {
                    oldMemAllocSections.Add(i, 0);
                }
                mo = ma - mo;
                string s = string.Format("{0} : {1} ({2}{3})", i, ma, (mo >= 0 ? "+" : "-"), mo);
                memAllocDebug(s);


                oldMemAllocSections[i] = ma;
            }
            foreach (long l in memMap.Keys)
            {
                MemData md = memMap[l];

                //memAllocDebug("", md.filepath);
            }
            
            

            ARE.Set();
            


        }

        private void listBox2_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {

            pipeThread.Abort();
            NPSS.Close();
            button1.Enabled = true;
            button2.Enabled = false;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            Environment.Exit(0);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            Environment.Exit(0);
        }
    }
}
