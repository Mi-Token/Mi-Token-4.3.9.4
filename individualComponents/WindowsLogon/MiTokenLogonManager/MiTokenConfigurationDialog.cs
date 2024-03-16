using System;
using System.Data;
using System.Drawing;
using System.Windows.Forms;

namespace MiTokenWindowsLogon
{
    public partial class MiTokenConfigurationDialog : Form
    {
        private const string MITOKEN_RADIUS_SERVER_SETTINGS = @"SOFTWARE\Mi-Token";

        private DataTable serverTable  = new DataTable();
        private BindingSource bs = new BindingSource();

        public MiTokenConfigurationDialog()
        {
            InitializeComponent();

            /*Add the handles to Server Grid view.*/
            dataGridViewRADIUSServers.AutoResizeColumns(DataGridViewAutoSizeColumnsMode.AllCells);
            dataGridViewRADIUSServers.MouseClick += new MouseEventHandler(DataGridView_MouseDown);
            dataGridViewRADIUSServers.MouseDoubleClick += new MouseEventHandler(DataGridView_MouseDoubleClick);
            dataGridViewRADIUSServers.SizeChanged += new EventHandler(ServerGridView_SizeChanged);
            //dataGridViewRADIUSServers.ColumnWidthChanged += new DataGridViewColumnEventHandler(dataGridViewRADIUSServers_ColumnWidthChanged);

            serverTable = new DataTable();
            serverTable.Columns.Add("Trial Order");
            serverTable.Columns.Add("IP Address Key");
            serverTable.Columns.Add("IP Address");
            serverTable.Columns.Add("Shared Secret Key");
            serverTable.Columns.Add("Shared Secret");
            serverTable.Columns.Add("RADIUS Port Key");
            serverTable.Columns.Add("RADIUS Port");
            
            foreach (RadiusServer server in RegistryHelper.RetrieveRadiusServers(MITOKEN_RADIUS_SERVER_SETTINGS))
               serverTable.Rows.Add(server.DisplayOrder, server.IPAddressKey, server.IPAddress, server.ServerSharedSecretKey, server.ServerSharedSecret, server.ServerPortKey, server.ServerPort);

            bs.DataSource = serverTable;
            dataGridViewRADIUSServers.DataSource = bs;

            dataGridViewRADIUSServers.Columns["IP Address Key"].Width = 0;
            dataGridViewRADIUSServers.Columns["IP Address Key"].Visible = false;

            dataGridViewRADIUSServers.Columns["Shared Secret Key"].Width = 0;
            dataGridViewRADIUSServers.Columns["Shared Secret Key"].Visible = false;

            dataGridViewRADIUSServers.Columns["RADIUS Port Key"].Width = 0;
            dataGridViewRADIUSServers.Columns["RADIUS Port Key"].Visible = false;


            dataGridViewRADIUSServers.Columns["Trial Order"].Width = 25;
            dataGridViewRADIUSServers.Columns["IP Address"].Width = 25;
            dataGridViewRADIUSServers.Columns["Shared Secret"].Width = 25;
            dataGridViewRADIUSServers.Columns["RADIUS Port"].Width = 25;

            ServerGridView_SizeChanged(null, null);
        }

        void dataGridViewRADIUSServers_ColumnWidthChanged(object sender, DataGridViewColumnEventArgs e)
        {
            dataGridViewRADIUSServers.AutoResizeColumns(DataGridViewAutoSizeColumnsMode.AllCells);
        }

        ContextMenu popupMenu;
        private void DataGridView_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            PopupContextMenu(e.Location);
        }
        private void DataGridView_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
                PopupContextMenu(e.Location);
        }

        private void PopupContextMenu(Point p)
        {
            DataGridView.HitTestInfo testInfo = dataGridViewRADIUSServers.HitTest(p.X, p.Y);
            if (testInfo.Type == DataGridViewHitTestType.Cell)
            {
                dataGridViewRADIUSServers.ClearSelection();
                dataGridViewRADIUSServers.Rows[testInfo.RowIndex].Selected = true;

                popupMenu = new ContextMenu();
                popupMenu.MenuItems.Add(new MenuItem("Edit server", Menu_Edit_OnClick));
                popupMenu.MenuItems.Add(new MenuItem("Delete server", Menu_Delete_OnClick));
                popupMenu.MenuItems.Add(new MenuItem("Test server", Menu_TestServer_OnClick));

                popupMenu.Show(dataGridViewRADIUSServers, p);
            }
            else if (testInfo.Type == DataGridViewHitTestType.None)
            {
                dataGridViewRADIUSServers.ClearSelection();
                popupMenu = new ContextMenu();
                popupMenu.MenuItems.Add(new MenuItem("Add new server", Menu_Add_OnClick));

                popupMenu.Show(dataGridViewRADIUSServers, p);
            }
        }

        private void ServerGridView_SizeChanged(object sender, System.EventArgs e)
        {
            if (dataGridViewRADIUSServers.ColumnCount != 0)
                foreach (DataGridViewColumn column in dataGridViewRADIUSServers.Columns)
                {
                    if (column.Name == "Trial Order" || column.Name == "RADIUS Port")
                        column.Width = dataGridViewRADIUSServers.Size.Width / 7;
                    else if (column.Name == "IP Address")
                        column.Width = 2 * dataGridViewRADIUSServers.Size.Width / 7;
                    else //if (column.Name == "Shared Secret")
                        column.Width = 3 * dataGridViewRADIUSServers.Size.Width / 7;
 
                    //column.Width = dataGridViewRADIUSServers.Size.Width / 4;
                }
        }
        

        private void Menu_Edit_OnClick(object sender, EventArgs e)
        {
            string description;

            DataGridViewSelectedRowCollection row = dataGridViewRADIUSServers.SelectedRows;

            RadiusServer server = new RadiusServer();
            server.IPAddressKey = row[0].Cells[1].Value.ToString();
            server.IPAddress = row[0].Cells[2].Value.ToString();
            server.ServerSharedSecretKey =  row[0].Cells[3].Value.ToString();
            server.ServerSharedSecret = DPHelper.Decrypt(row[0].Cells[4].Value.ToString(), string.Empty, out description);
            server.ServerPortKey = row[0].Cells[5].Value.ToString();
            server.ServerPort = int.Parse(row[0].Cells[6].Value.ToString());

            ServerRegistrationDialog dlg = new ServerRegistrationDialog("Update RADIUS server", "Update", server);
            if (dlg.ShowDialog(this) == DialogResult.OK)
            {
                try
                {
                    server.IPAddress = dlg.IPAddress;
                    server.ServerSharedSecret = dlg.SharedSecret;
                    server.ServerPort = dlg.RADIUSPort;

                    RegistryHelper.WriteRadiusServer(MITOKEN_RADIUS_SERVER_SETTINGS, ref server);

                    int relativePosition = row[0].Index;
                    serverTable.Rows[relativePosition][2] = server.IPAddress;
                    serverTable.Rows[relativePosition][6] = server.ServerPort.ToString();
                    serverTable.Rows[relativePosition][4] = server.ServerSharedSecret;
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void Menu_Add_OnClick(object sender, EventArgs e)
        {
            ServerRegistrationDialog dlg = new ServerRegistrationDialog("Add server description", "Add");

            if (dlg.ShowDialog(this) == DialogResult.OK)
            {
                try
                {
                    /*Add the new server to Server_Description table.*/
                    RadiusServer server = new RadiusServer();
                    server.IPAddress = dlg.IPAddress;
                    server.ServerSharedSecret = dlg.SharedSecret;// DPHelper.Encrypt(DPHelper.Store.USE_MACHINE_STORE, dlg.SharedSecret, string.Empty, "Shared secret for RADIUS server.");
                    server.ServerPort = dlg.RADIUSPort;

                    RegistryHelper.WriteRadiusServer(MITOKEN_RADIUS_SERVER_SETTINGS, ref server);
                    serverTable.Rows.Add(server.DisplayOrder, server.IPAddressKey, server.IPAddress, server.ServerSharedSecretKey, server.ServerSharedSecret, server.ServerPortKey, server.ServerPort);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void Menu_Delete_OnClick(object sender, EventArgs e)
        {
            DataGridViewSelectedRowCollection row = dataGridViewRADIUSServers.SelectedRows;
            try
            {

                RadiusServer server = new RadiusServer();
                server.IPAddressKey = row[0].Cells[1].Value.ToString();
                server.ServerSharedSecretKey = row[0].Cells[3].Value.ToString();
                server.ServerPortKey = row[0].Cells[5].Value.ToString();

                /*Delete the server from the server list.*/
                RegistryHelper.DeleteRadiusServer(MITOKEN_RADIUS_SERVER_SETTINGS, server);
                serverTable.Rows.RemoveAt(row[0].Index);

            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void Menu_TestServer_OnClick(object sender, EventArgs e)
        {
            DataGridViewSelectedRowCollection row = dataGridViewRADIUSServers.SelectedRows;

            RadiusServer server = new RadiusServer();
            server.IPAddress = row[0].Cells[2].Value.ToString();
            server.ServerPort = int.Parse(row[0].Cells[6].Value.ToString());
            server.ServerSharedSecret = row[0].Cells[4].Value.ToString();
            
            RadTestDialog dlg = new RadTestDialog(server);

            dlg.ShowDialog(this);
        }

        private void MiTokenConfigurationDialog_Load(object sender, EventArgs e)
        {
            
        }

        private void MiTokenConfigurationDialog_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (dataGridViewRADIUSServers.Rows.Count == 0)
            {
                MessageBox.Show("You must configure one Radius Server");
                Program.servers = false;
            }
            else
            {
                Program.servers = true;
            }
        }

        private void dataGridViewRADIUSServers_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }


        //void dataGridView1_ColumnWidthChanged(object sender, DataGridViewColumnEventArgs e)
        //{
        //    this.dataGridViewRADIUSServers.Invalidate();
        //}

    }
}
