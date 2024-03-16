using System;
using System.Net;
using System.Net.Security;
using System.Windows.Forms;
using System.Security.Cryptography.X509Certificates;

namespace API_setup
{
    public partial class tester : Form
    {
        private static bool validateCert(object sender, X509Certificate cert, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            return true;
        }

        public tester()
        {
            InitializeComponent();
            System.Net.ServicePointManager.ServerCertificateValidationCallback = validateCert;
            System.Net.ServicePointManager.Expect100Continue = false;
        }

        private bool isIP(string str)
        {
            string[] split = str.Trim().Split('.');
            if (split.Length != 4) return false;
            try
            {
                for (int i = 0; i < 4; i++)
                    if (Convert.ToInt16(split[i]) > 255 | Convert.ToInt16(split[i]) < 0)
                        return false;
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }
        
        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                if (txtboxAPIServer.Text == "" || txtboxOneTimePass.Text == "" || txtboxUsername.Text == "")
                {

                    txtboxAuthResult.Text = "Not all fields are filled in";
                    return;
                }
                string IP = "";
                if (!isIP(txtboxAPIServer.Text))
                {
                    try
                    {
                        System.Net.IPAddress[] adds = System.Net.Dns.GetHostAddresses(txtboxAPIServer.Text);
                        foreach (System.Net.IPAddress add in adds)
                        {
                            if (add.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                            {
                                //IPV4
                                IP = add.ToString();
                                break;
                            }
                        }
                        if (IP == "")
                            throw new Exception();
                        
                    }
                    catch (Exception)
                    {
                        txtboxAuthResult.Text = "Error looking up IP for Hostname";
                        return;
                    }
                }
                else
                    IP = txtboxAPIServer.Text;
                HttpWebRequest http = (HttpWebRequest)WebRequest.Create("https://" + IP + "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonVerifyOTPEx?username=" + txtboxUsername.Text + "&otp=" + txtboxOneTimePass.Text + "&flags=0");
                http.Proxy = null;
                http.Timeout = 1500;

                WebResponse response = http.GetResponse();

                response.Close();

                txtboxAuthResult.Text = "Authenticated!";
            }

            catch (WebException ex)
            {
                switch (ex.Status)
                {
                    case WebExceptionStatus.Timeout:
                        txtboxAuthResult.Text += "Timeout, Check the Server IP";
                        break;
                    case WebExceptionStatus.ProtocolError:
                        if (ex.Message.Contains("401"))
                        {
                            txtboxAuthResult.Text = "Access denied. The API Server is probably working - please try again with an OTP which will work.";
                        }
                        else
                        {
                            txtboxAuthResult.Text += "There was a protocol error this could mean the verification failed or that the path to the api has been set incorrectly by the developer";

                            txtboxAuthResult.Text += " Please retry with a new OTP before checking for an update to the program. Any further problems should be directed to support@mi-token.com";
                        }

                        break;
                    case WebExceptionStatus.TrustFailure:
                        txtboxAuthResult.Text = "The API service is currently running but the certificate issued by the server appears to be have been self signed. This is how a standard mi-token API certificate behaves.";

                        txtboxAuthResult.Text += " If you deselect Verify Certificates then this message will go away and you will be able to authenticate.";

                        break;
                    default:
                        txtboxAuthResult.Text += ex.ToString();

                        txtboxAuthResult.Text += ex.Status.ToString();

                        break;

                }
            }


            txtboxOneTimePass.Text = "";

            txtboxOneTimePass.Focus();
        }

        private void txtboxOneTimePass_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
