using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace API_Autoconfig
{
    static class Program
    {
        public static bool sufficientprivleges = true;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            foreach (string s in Environment.GetCommandLineArgs())
            {
                //debug string to start with form2 instead of form1.
                if (s == "-debug")
                {
                    Application.Run(new Form2());
                    return;
                }
            }

            //Form2 is now the default form to use.
            Application.Run(new Form2());
        }
    }
}
