using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace OTP_Cache_Config
{
    static class Program
    {
        static public bool serverMode, clientMode;


        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            
            clientMode = serverMode = false;
            foreach (string s in args)
            {
                if (s.ToLower().Equals("-clientmode"))
                {
                    clientMode = true;
                }
                if (s.ToLower().Equals("-servermode"))
                {
                    serverMode = true;
                }
            }

            if (clientMode && serverMode)
            {
                //both client and server mode enabled - disable them both
                clientMode = serverMode = false;
            }

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
