using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLE_API
{
    public partial class BLE
    {
        public static class Settings
        {
            public static bool Disallow_DLLV1 = true, Disallow_DLLV2 = true, Disallow_DLLV2_1 = false, Disallow_DLLV2_2 = false;

            static void SetDefaultSettings()
            {
                Disallow_DLLV1 = true;
                Disallow_DLLV2 = true;
                Disallow_DLLV2_1 = false;
                Disallow_DLLV2_2 = false;
            }
            
        }
    }
}
