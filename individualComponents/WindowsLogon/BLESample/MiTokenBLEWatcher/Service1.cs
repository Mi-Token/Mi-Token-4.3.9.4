using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;

using System.Threading;
using BLEWatcherCSTest;

namespace MiTokenBLEWatcher
{
    public partial class Service1 : ServiceBase
    {
        public Service1()
        {
            InitializeComponent();
        }
        static public void eventLogMessage(string message, bool requireRegKey = false)
        {
#if DUMMY_CODE
#else

#if NON
            if ((requireRegKey) && (!regKeyLogging))
            {
                return;
            }
#endif
#endif
            const string source = "Mi-Token BLE Watcher";
            const string log = "Application";

            if (!EventLog.SourceExists(source))
            {
                EventLog.CreateEventSource(source, log);
            }
            EventLog.WriteEntry(source, message);
            return;
        }

#if DUMMY_CODE
#else
        WatcherCode watcher = null;
#endif
        Thread mThread = null;




        BLEWatcherStateMachine.StateMachine Machine = null;

        protected override void OnStart(string[] args)
        {

            eventLogMessage("In OnStart");
            eventLogMessage("Path is : " + System.Reflection.Assembly.GetExecutingAssembly().Location);

            Machine = new BLEWatcherStateMachine.StateMachine();
            Machine.Initialize();
            Machine.StartMachine();

            /*
#if DUMMY_CODE
            mThread = new Thread(new ThreadStart(() =>
                {
                    while (true)
                    {
                        eventLogMessage("In MainLoop, Version = " + Program.versionID + "m");
                        Thread.Sleep(1000);
                    }
                }));
            mThread.Start();
#else
            watcher = new WatcherCode();
            if (watcher.initFine)
            {
                (mThread = new Thread(new ThreadStart(() =>
                {

                    try
                    {
                        if (watcher.connectToCOM())
                        {
                            while (watcher.hasAccessToCOMPort(true) == false)
                            {
                                //hasAccessToCOMPort only returns false if we don't know which client we are waiting for
                                //So wait a second and then try again.
                                Thread.Sleep(1000);
                            }

                            //watcher.startPoller();

                            while (true)
                            {
                                watcher.waitForWatcherPowerup();
                                watcher.mainWatching();
                            }

                        }
                    }
                    catch (Exception ex)
                    {
                        watcher.logException(ex);
                    }

                }))).Start();
            }
            else
            {
                watcher = null;
                Stop();
            }
#endif
             * */

        }

        protected override void OnStop()
        {
            eventLogMessage("In OnStop");
            if (Machine != null)
            {
                Machine.StopMachine();
                Machine = null;
            }
#if DUMMY_CODE
#else

            if (watcher != null)
            {
                watcher.closeDown();

            }
#endif

            if (mThread != null)
            {
                mThread.Abort();
            }
        }
    }
}

