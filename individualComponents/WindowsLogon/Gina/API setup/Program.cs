using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace API_setup
{
    static class Program
    {
        public static bool servers = false;
        public static bool sufficientprivleges = true;
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
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

            //args = new string[] { "", "C:\\Stuff\\Test.xml" };
            if (args.Length >= 1)
            {
                if (args[0].EndsWith(".xml"))
                {
                    //The settings xml file
                    List<PersistSettings.Errors> errors;
                    PersistSettings.persistXMLtoRegistry(args[0], out errors);

                    if ((args.Length >= 2) && (args[1] == "-sF"))
                    {
                        //-sF means completely silent (no errors even)
                        MessageBox.Show("Debug : Completed");
                        return;
                    }
                    else
                    {
                        if (errors.Count > 0)
                        {
                            MessageBox.Show(String.Format("There were {0} error{1} while processing the XML file", errors.Count, errors.Count == 1 ? "" : "s"));
                            foreach (PersistSettings.Errors e in errors)
                            {
                                MessageBox.Show(string.Format("Location : {0}\nData : {1}\nMessage : {2}", e.Section, e.Data, e.Message));
                            }
                        }
                    }
                    if ((args.Length >= 2) && (args[1] == "-s"))
                    {
                        //-s means do not show the dialog at all (but DO show errors)
                        return;
                    }
                }    

            }
            //while (servers == false && sufficientprivleges == true)
            {
                Application.Run(new Form2());
            }
        }
    }
}
