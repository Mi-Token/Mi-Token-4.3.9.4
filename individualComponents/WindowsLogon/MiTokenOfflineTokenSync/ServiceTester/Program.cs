using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ServiceTester
{
    class Program
    {
        static void Main(string[] args)
        {
            MiTokenOfflineTokenSync.Service1 s1 = new MiTokenOfflineTokenSync.Service1();
            s1.debugStart();

            Console.ReadLine();

            s1.debugEnd();

        }
    }
}
