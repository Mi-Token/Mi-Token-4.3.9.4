using System;
using System.Text;
using System.Net;
using System.Windows.Forms;

namespace API_Tester
{
    public partial class Form1 : Form
    {

        Trustall customPolicy = new Trustall();
        
        public Form1()
        {
            
            System.Net.ServicePointManager.CertificatePolicy = customPolicy;
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            txtboxAuthResult.Text = string.Empty;
            txtboxAuthResult.Refresh();
            Cursor.Current = Cursors.WaitCursor;
            var watch = System.Diagnostics.Stopwatch.StartNew();

            try
            {

                if (txtboxAPIServer.Text == "" || txtboxOneTimePass.Text == "" || txtboxUsername.Text == "")
                {

                    txtboxAuthResult.Text = "Not all fields are filled in";
                    return;
                }

                HttpWebRequest http = (HttpWebRequest)WebRequest.Create("https://" + txtboxAPIServer.Text + "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonVerifyOTPEx?username=" + txtboxUsername.Text + "&otp=" + txtboxOneTimePass.Text + "&flags=0");
                http.Proxy = null;
                http.Timeout = int.Parse(txtTimeout.Text);

                DateTime dt = DateTime.Now;

                using (var response = (HttpWebResponse)http.GetResponse())
                {
                    ;
                }
                watch.Stop();

                var elapsedMs = watch.ElapsedMilliseconds;
                DateTime dtHandshake = Trustall.GetLastHadshakeTime();
                string strHandshake = (dtHandshake == DateTime.MinValue || dt > dtHandshake)? "No server certificate check for SSL handshake." : string.Format("Server certificate check for SSL handshake took {0} msec.", Convert.ToUInt64((dtHandshake - dt).TotalMilliseconds).ToString());

                var sb = new StringBuilder("Authenticated.");
                sb.AppendLine();
                sb.AppendFormat("API call duration: {0} msec.", elapsedMs.ToString());
                sb.AppendLine();
                sb.Append(strHandshake);
                txtboxAuthResult.Text = sb.ToString();
            }

            catch (WebException ex)
            {
                watch.Stop();
                var elapsedMs = watch.ElapsedMilliseconds;

                string serverStrResponse = "";
                if (ex.Response != null)
                {
                    try
                    {
                        using (var hwr = (HttpWebResponse)ex.Response)
                        using (var stream = hwr.GetResponseStream())
                        {
                            byte[] data = new byte[hwr.ContentLength];
                            stream.Read(data, 0, data.Length);
                            stream.Close();
                            serverStrResponse = System.Text.Encoding.ASCII.GetString(data);
                        }
                    }
                    catch (Exception ex2)
                    {
                        serverStrResponse = "Mi-Token API Tester Internal Error : Failed to get server response : " + ex2.Message;
                    }
                }

                switch (ex.Status)
                {
                    case WebExceptionStatus.Timeout:
                        txtboxAuthResult.Text = "Timeout, check the Server address and adjust timeout";
                        break;
                    case WebExceptionStatus.ProtocolError:
                        txtboxAuthResult.Text = "The Server Replied with the message:\r\n" + serverStrResponse + "\r\n";
                        txtboxAuthResult.Text += "There was a protocol error this could mean the verification failed or that the path to the api has been set incorrectly by the developer";

                        txtboxAuthResult.Text += "Please retry with a new OTP before checking for an update to the program. Any further problems should be directed to support@mi-token.com";

                        break;
                    case WebExceptionStatus.TrustFailure:
                        txtboxAuthResult.Text = "The API service is currently running but the certificate issued by the server appears to be have been self signed. This is how a standard mi-token API certificate behaves.";

                        txtboxAuthResult.Text += " If you deselect Verify Certificates then this message will go away and you will be able to authenticate.";

                        break;
                    default:
                        txtboxAuthResult.Text = ex.ToString();

                        txtboxAuthResult.Text += ex.Status.ToString();

                        break;

                }

                txtboxAuthResult.Text += ex.Message;
                txtboxAuthResult.Text += ex.StackTrace;
                txtboxAuthResult.Text += Environment.NewLine + Environment.NewLine + "API call duration: " + elapsedMs.ToString() + " msec.";
            }


            txtboxOneTimePass.Text = "";

            txtboxOneTimePass.Focus();
        }

       
    }
}
