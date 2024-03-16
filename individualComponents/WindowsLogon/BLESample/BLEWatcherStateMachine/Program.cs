using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BLEWatcherStateMachine
{
    static class Program
    {
        public static bool delayStart;
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            delayStart = false;
            foreach (string s in args)
            {
                if (s.ToLower().Equals("delaystart"))
                {
                    delayStart = true;
                }
            }
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
