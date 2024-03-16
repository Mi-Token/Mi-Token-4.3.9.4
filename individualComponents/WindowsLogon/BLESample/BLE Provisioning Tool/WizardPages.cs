using System;
using System.Windows.Forms;

namespace BLE_Provisioning_Tool
{
    class WizardPages : TabControl 
    {
        protected override void WndProc(ref Message m) 
        {
            // Hide tabs by trapping the TCM_ADJUSTRECT message
            if (m.Msg == 0x1328 && !DesignMode)
            {
                m.Result = (IntPtr)1;
            }
            else
            {
                base.WndProc(ref m);
            }
        }

        protected override void OnDeselected(TabControlEventArgs e)
        {
            base.OnDeselected(e);
            //EnableTab(e.TabPage, false);
        }

        protected override void OnSelected(TabControlEventArgs e)
        {
            base.OnSelected(e);
            EnableTab(e.TabPage, true);
        }

        public static void EnableTab(TabPage page, bool enable)
        {
            foreach (Control ctl in page.Controls) ctl.Enabled = enable;
        }
    }
}