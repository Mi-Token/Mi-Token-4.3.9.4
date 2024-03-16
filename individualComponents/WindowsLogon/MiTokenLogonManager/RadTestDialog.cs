using System;
using System.Drawing;
using System.Net.Radius;
using System.Net.Sockets;
using System.Windows.Forms;
using log4net;
using log4net.Core;
using log4net.Layout;

namespace MiTokenWindowsLogon
{
    public partial class RadTestDialog : Form
    {
        private static ILog log = LogManager.GetLogger(typeof(MiTokenConfigurationDialog));

        RadiusServer server;
        public RadTestDialog(RadiusServer server)
        {
            InitializeComponent();

            this.server = server;

            /* Initialize RichTextBoxAppender.*/
            var rba = new RichTextBoxAppender(richTextBoxLog);
            rba.Threshold = Level.All;
            rba.Layout = new PatternLayout("%date{dd-MM-yyyy HH:mm:ss.fff} %message %n");
            rba.AddMapping(Level.Info, Color.Black, 10.0f, false);
            rba.AddMapping(Level.Debug, Color.Black, 10.0f, false);
            rba.AddMapping(Level.Warn, Color.Black, 10.0f, true);
            rba.AddMapping(Level.Error, Color.Red, 10.0f, true);

            log4net.Config.BasicConfigurator.Configure(rba);
            rba.ActivateOptions();

            this.AcceptButton = btnTestRADIUSServer;
        }

        private void btnTestRADIUSServer_Click(object sender, EventArgs e)
        {
            try
            {
                string description;
                RadiusClient rc = new RadiusClient(server.IPAddress,server.ServerPort, 0,DPHelper.Decrypt(server.ServerSharedSecret, string.Empty, out description));
                RadiusPacket authPacket = rc.Authenticate(txtUserName.Text, txtPassword.Text + txtOTP.Text);
                RadiusPacket receivedPacket = rc.SendAndReceivePacket(authPacket);

                if (receivedPacket == null)
                    throw new Exception("Can't contact remote radius server !");

                log.Debug(receivedPacket.Type.ToString());

                foreach (RadiusAttribute attr in receivedPacket.Attributes)
                {
                    log.Debug(attr.Type.ToString() + " = " + attr.Value);
                }
            }
            catch (Exception eex)
            {
                var ex = eex as SocketException;
                if (ex != null && ex.SocketErrorCode == SocketError.TimedOut)
                {
                    log.Error("Timed out waiting for a RADIUS reply. Please check:");
                    log.Error(" 1. For firewalls that may be blocking RADIUS (UDP 1812)");
                    log.Error(" 2. That this server's IP address is configured as a RADIUS client. Watch out for IPv6.");
                    log.Error("");
                    log.Error("");
                }
                log.Error("Error : " + ex.ToString());
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
        }



    }
}
