using System;
using System.Drawing;
using System.Windows.Forms;

namespace MiTokenWindowsLogon
{
    public partial class ServerRegistrationDialog : Form
    {
        public string IPAddress { get; set; }
        public string SharedSecret { get; set; }
        public int RADIUSPort { get; set; }

        private const uint MIN_TCP_PORT = 1;
        private const uint MAX_TCP_PORT = 65535;
        private const int SERVER_NAME_LENGTH = 100;
        private const int SERVER_DESCRIPTION = 250;
        private const int FORM_WIDTH = 465;
        private const int FORM_HEIGHT = 250;

        public ServerRegistrationDialog(string grpBoxText, string btnText)
        {
            InitializeComponent();
            grpBoxServerDescription.Text = grpBoxText;
            btnCreateNewServer.Text = btnText;

            MinimumSize = new Size(FORM_WIDTH, FORM_HEIGHT);
            MaximumSize = MinimumSize;
        }

        public ServerRegistrationDialog(string grpBoxText, string btnText, RadiusServer server)
        {
            InitializeComponent();
            this.StartPosition = FormStartPosition.CenterScreen;

            grpBoxServerDescription.Text = grpBoxText;
            txtIPAddress.Text = server.IPAddress;
            txtSharedServerSecret.Text = server.ServerSharedSecret;
            txtRADIUSPort.Text = server.ServerPort.ToString();
            btnCreateNewServer.Text = btnText;

            MinimumSize = new Size(FORM_WIDTH, FORM_HEIGHT);
            MaximumSize = MinimumSize;
        }

        private void btnCreateNewServer_Click(object sender, EventArgs e)
        {
            string completeErrorMessage = string.Empty;
            System.Net.IPAddress validIP;

            if (string.IsNullOrEmpty(txtIPAddress.Text))
                completeErrorMessage = "IP address should not be empty.";
            else if(!System.Net.IPAddress.TryParse(txtIPAddress.Text, out validIP))
                completeErrorMessage = "IP address is invalid.";

            string errorMessage;
            double portIntVal;

            if (string.IsNullOrEmpty(txtRADIUSPort.Text))
            {
                errorMessage = "Pleae enter the RADIUS port.";
                completeErrorMessage = string.IsNullOrEmpty(completeErrorMessage) ? errorMessage : completeErrorMessage + Environment.NewLine + errorMessage;
            }
            else if (!Double.TryParse(txtRADIUSPort.Text, out portIntVal))
            {
                errorMessage = "Port must be an integer.";
                completeErrorMessage = string.IsNullOrEmpty(completeErrorMessage) ? errorMessage : completeErrorMessage + Environment.NewLine + errorMessage;
            }
            else if (portIntVal < MIN_TCP_PORT || portIntVal > MAX_TCP_PORT)
            {
                errorMessage = "Port must be within the range 1 and 65535.";
                completeErrorMessage = string.IsNullOrEmpty(completeErrorMessage) ? errorMessage : completeErrorMessage + Environment.NewLine + errorMessage;
            }

            if (string.IsNullOrEmpty(txtSharedServerSecret.Text))
            {
                errorMessage = "Shared secret should not be empty.";
                completeErrorMessage = string.IsNullOrEmpty(completeErrorMessage) ? errorMessage : completeErrorMessage + Environment.NewLine + errorMessage;
            }

            if (string.IsNullOrEmpty(completeErrorMessage))
            {
                IPAddress = txtIPAddress.Text;
                SharedSecret = txtSharedServerSecret.Text;
                RADIUSPort = int.Parse(txtRADIUSPort.Text);

                DialogResult = DialogResult.OK;
            }
            else
                MessageBox.Show(completeErrorMessage, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

        }

        private void ServerRegistrationDialog_Load(object sender, EventArgs e)
        {
            AcceptButton = btnCreateNewServer;
        }
    }
}
