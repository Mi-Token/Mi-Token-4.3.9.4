//Contains the extra API call to do AnonVerifyOTPEx
//This code is not part of the main API because it is an Anon method and the API already has non-anon methods to do authentication


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using MiToken;

namespace LoginStressTester
{
    public static class MiTokenAPIExt
    {
        public static string AnonVerifyOTPEx(string server, string username, string otp, int hotpSteps, int totpSteps, string exp, string mod)
        {
            Dictionary<string, string> args = new Dictionary<string, string>();
            args.Add("username", username);
            args.Add("otp", otp);
            args.Add("hotpSteps", hotpSteps.ToString());
            args.Add("totpSteps", totpSteps.ToString());
            args.Add("exponent", exp);
            args.Add("modulus", mod);
            args.Add("flags", "3");

            return MiToken.API.AnonAPICall(server + @"/mi-token/api/authentication/v4/RawHTTPWithSSL/", "AnonVerifyOTPEx", args, API.APIMethodType.Get);
        }
    }
}
