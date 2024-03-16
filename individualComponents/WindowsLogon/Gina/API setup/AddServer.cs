using System;
using System.Windows.Forms;

namespace API_setup
{
    public partial class AddServer : Form
    {

        public string IPAddress { get; set; }
        public AddServer()
        {
            InitializeComponent();
        }

        private void btnCreateNewServer_Click(object sender, EventArgs e)
        {
            string completeErrorMessage = string.Empty;
            System.Net.IPAddress validIP;

            if (string.IsNullOrEmpty(txtIPAddress.Text))
                completeErrorMessage = "IP address should not be empty.";
            else if (!System.Net.IPAddress.TryParse(txtIPAddress.Text, out validIP))
            {
                //check hostname.
                try
                {
                    System.Net.IPAddress[] adds = System.Net.Dns.GetHostAddresses(txtIPAddress.Text);
                }
                catch(Exception)
                {
                    completeErrorMessage = "IP address \\ Host Name is invalid.";
                }
            }
            if (string.IsNullOrEmpty(completeErrorMessage))
            {
                IPAddress = txtIPAddress.Text;
                
                DialogResult = DialogResult.OK;
            }
            else
                MessageBox.Show(completeErrorMessage, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

        }

        private void txtIPAddress_KeyPress(object sender, KeyPressEventArgs e)
        {
            
           
            if (e.KeyChar == (char)(Keys.Enter))
            {
                btnCreateNewServer_Click(sender, new EventArgs());
                //Repeated code
                /*
                string completeErrorMessage = string.Empty;
                System.Net.IPAddress validIP;

                if (string.IsNullOrEmpty(txtIPAddress.Text))
                    completeErrorMessage = "IP address should not be empty.";
                else if (!System.Net.IPAddress.TryParse(txtIPAddress.Text, out validIP))
                    completeErrorMessage = "IP address is invalid.";
                if (string.IsNullOrEmpty(completeErrorMessage))
                {
                    IPAddress = txtIPAddress.Text;

                    DialogResult = DialogResult.OK;
                }
                else
                    MessageBox.Show(completeErrorMessage, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                 * */
            }
        }

        public void setIP (string IPAdress){
            this.txtIPAddress.Text = IPAdress;
            btnCreateNewServer.Text = "Save";

        }

        private void AddServer_Load(object sender, EventArgs e)
        {

        }

        private void txtIPAddress_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
