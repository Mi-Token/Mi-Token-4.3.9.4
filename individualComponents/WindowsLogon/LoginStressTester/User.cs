using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LoginStressTester
{
    class User
    {
        public User()
        {
            testing = false;
        }
        public User(string name, string tempToken)
        {
            useTempToken = true;
            username = name;
            this.tempToken = tempToken;
        }

        byte[] OTPKey;
        string tempToken;
        bool useTempToken;

        static UInt32 CurCount
        {
            get
            {
                return (UInt32)((DateTime.UtcNow) - new DateTime(1970, 1, 1)).TotalSeconds / 30;
            }
        }

        string username;

        //exp and mod were generated on my VM, as their value doesn't really matter for the stress test, we will just use the same values for all users
        string exp = "AAAAAw";
        string mod = "u2ntj1D06EYy7LfjzvF_JU1huIpaXBEKmUBPOfixEJSIT3JmVz9IDo2_CFKlNyp_rBjrud6FTOLvlVG6r2TBIO0Bts2r5Y2bS7mBRzYwGDAyqtYi73r_c5eN6jN5sBv3GbJxwvABcFEAV3-tFafdIMQyZhhnWHHP7U0vjECOfkrxBteM6lPdvcoiABdpgMHB0-nSv_wMaBqBviYHuar2KL7hwO0GNnHIlIUOUuY00aaBqNN9XdCNuaTccFZfvKbxVJoU6mbuPSysuYpBDWQnExIvvzImJJx0_vSKSgVVzd8w5KP6brZKn8nYr5NkWkw7UZREODs9cGH__9gY-IlEAQ";

        UInt32 lastCount;


        public string doRequest(string serverAddress, int hotp, int totp)
        {
            //Get current timestep, and make sure it is different to the last timestep we tried at, however if we are using temp tokens, we don't are about timesteps
            UInt32 c = CurCount;
            while ((c == lastCount) && (!useTempToken))
            {
                //we need to wait some time, lets wait 20 seconds which should change the CC 2/3rds of the time
                System.Threading.Thread.Sleep(20000);
                c = CurCount;
            }

            //get otp either from temptoken or generate the HMACSHA1 hash.
            string otp = "";
            if (useTempToken)
            {
                otp = tempToken;
            }
            else
            {
                otp = OTPAUTH.GenOTP(c, OTPKey);
            }

            lastCount = c;

            //API call. Using MiTokenAPIExt because AnonVerifyOTPEx does not exist in MiTokenAPI. Not sure if I should add it, see MiTokenAPIEx for more info.
            string result =  MiTokenAPIExt.AnonVerifyOTPEx(serverAddress, username, otp, hotp, totp, exp, mod);

            return result;
        }

        public bool testing;
    }
}
