using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System.Threading;

namespace YubikeyRemovalApplication
{
    class Program
    {
        [DllImport("YubikeyLock")]
        public static extern int _MainFunc();

        


        static void Main(string[] args)
        {
            Thread t = new Thread(run);
            t.Start();
            
        }

        public static void run()
        {
            _MainFunc();
        }
    }
}
