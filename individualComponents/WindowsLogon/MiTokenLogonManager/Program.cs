using System;
using System.Threading;
using System.Windows.Forms;
using log4net;

namespace MiTokenWindowsLogon
{
    static class Program
    {
        public static bool servers = false;
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        /// 
        private static readonly ILog log = LogManager.GetLogger(typeof(Program));


        [STAThread]
        static void Main()
        {

            // set up exception handling
            Application.ThreadException += OnUIThreadException;
            Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);
            AppDomain.CurrentDomain.UnhandledException += OnGlobalUnhandledException;

            // set up logging
            //string assemblyFilename = System.Reflection.Assembly.GetExecutingAssembly().Location;
            //string assemblyDir = Path.GetDirectoryName(assemblyFilename);
            //string workingDirectory = Path.Combine(assemblyDir, "Reporting Install Logs\\" + DateTime.Now.ToString("u").Replace(':', '.').TrimEnd(new[] { 'Z' }));
            //Directory.CreateDirectory(workingDirectory);

            //log4net.Appender.FileAppender appender = new log4net.Appender.FileAppender();
            //appender.Layout = new log4net.Layout.PatternLayout("%timestamp [%thread] %level %logger - %message%newline%exception");
            //appender.File = Path.Combine(workingDirectory, "log.txt");
            //appender.ImmediateFlush = true;
            //appender.Threshold = log4net.Core.Level.All;
            //appender.ActivateOptions();

            //log4net.Config.BasicConfigurator.Configure(appender);

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            while (servers == false)
            {
                Application.Run(new MiTokenConfigurationDialog());
            }
        }

        private static void OnUIThreadException(object sender, ThreadExceptionEventArgs t)
        {
            ShowException(t.Exception);
        }

        private static void OnGlobalUnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            ShowException(e.ExceptionObject as Exception);
        }

        private static void ShowException(Exception e)
        {
            try
            {
                if (e == null)
                    e = new Exception("Unknown exception..!");

                log.Error("An unhandled exception has arisen", e);

                if (Application.OpenForms.Count > 0)
                {
                    ApplicationException outer = new ApplicationException("An unexpected error has occured, so the wizard will now terminate.", e);
                    MiToken.ExceptionMessageBox.ExceptionMessageBox msg = new MiToken.ExceptionMessageBox.ExceptionMessageBox(outer);
                    msg.Show(Application.OpenForms[0]);
                }
                else
                {
                    MessageBox.Show(e.Message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);

                    //MessageBox.Show(e.ToString(), "MiToken", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            finally
            {
                Environment.ExitCode = 5/*Unhandled exception.*/;
                Application.Exit();
            }
        }
    }
}
