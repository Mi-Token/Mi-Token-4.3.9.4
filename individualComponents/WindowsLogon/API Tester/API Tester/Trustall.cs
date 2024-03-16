using System;
using System.Net;
using System.Threading;
using System.Security.Cryptography.X509Certificates;

public class Trustall : System.Net.ICertificatePolicy
{
  public Trustall()
    {
      
    }

    public bool CheckValidationResult(ServicePoint sp,
     X509Certificate cert, WebRequest req, int problem)
    {
        long lTime = DateTime.Now.ToFileTime();
        Interlocked.Exchange(ref dt_s, lTime);

        return true;
    }

    static internal DateTime GetLastHadshakeTime()
    {
        long lTime = Interlocked.Exchange(ref dt_s, 0L);

        return lTime == 0L? DateTime.MinValue: DateTime.FromFileTime(lTime);
    }

    static long dt_s;
}
