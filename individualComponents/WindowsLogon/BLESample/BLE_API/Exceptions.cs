using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLE_API
{
    public partial class BLE
    {
        public class Exceptions
        {
            public class DllVersionDisallowedException : Exception
            {
                private string msg;
                public DllVersionDisallowedException(string message)
                {
                    msg = message;
                }

                public override string Message
                {
	                get 
	                { 
		                 return msg;
	                }
                }
            }

            public class DLLVersionCheckFailed : Exception
            {
                private string msg;
                public DLLVersionCheckFailed(string message, Exception innerException)
                {
                    msg = message;
                    this.innerException = innerException;
                }

                public override string Message
                {
                    get
                    {
                        return msg;
                    }
                }
                public Exception innerException;
            }

            public class WrongDLLVersion : Exception
            {
                private int expectedVersion, version;
                public WrongDLLVersion(int expVersion, int ver)
                {
                    expectedVersion = expVersion;
                    version = ver; 
                }

                public override string Message
                {
                    get
                    {
                        int evMaj, evMin, evRev, vMaj, vMin, vRev;
                        Inner.ReverseVersion(expectedVersion, out evMaj, out evMin, out evRev);
                        Inner.ReverseVersion(version, out vMaj, out vMin, out vRev);
                        return string.Format("Wrong DLL Version. Expected {0}.{1}.{2}, DLL Version is {3}.{4}.{5}", evMaj, evMin, evRev, vMaj, vMin, vRev);
                    }
                }
            }
        }
    }
}
