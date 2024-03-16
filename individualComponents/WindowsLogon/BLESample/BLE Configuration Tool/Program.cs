using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace BLE_Configuration_Tool
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>

        public static bool debugMode = false;
        public static string portID = "";
        [STAThread]
        static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            //Application.Run(new Form1());

            for (int i = 0; i < args.Length; ++i)
            {
                if (args[i] == "-DEBUGMODE")
                {
                    debugMode = true;
                }
                if (args[i] == "-PORT")
                {
                    if(args.Length > (i+1))
                    {
                        portID = args[i+1];
                        i++;
                    }
                }
            }

            if(debugMode)
            {
                //We are running in debug mode, so start DebugStart
                Application.Run(new DebugStart());
            }
            else
            {
                //We are running in normal mode, so start BLE_Configuration
                Application.Run(new BLE_Configuration());
            }
        }
    }
}
