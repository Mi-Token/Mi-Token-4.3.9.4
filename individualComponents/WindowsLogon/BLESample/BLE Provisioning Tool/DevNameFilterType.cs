using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32;

namespace BLE_Provisioning_Tool
{
    class DevNameFilterType
    {            
        public enum eType
        {
            None,
            MtAll,
            MtBlu,
            MtBcn,
            MtBoot,
        };

        static private Dictionary<DevNameFilterType.eType, String> m_filterTypeStringDictionary = new Dictionary<DevNameFilterType.eType, String>()
        {
            {eType.None, "*"},
            {eType.MtAll, "MT-"},
            {eType.MtBlu, "MT-BLU"},
            {eType.MtBcn, "MT-BCN"},
            {eType.MtBoot, "MT-BOOT"},
        };

        public eType Type { get; set; }
        public String getTypeStr() { return m_filterTypeStringDictionary[Type]; }
        static public String getTypeStr(DevNameFilterType.eType type) { return m_filterTypeStringDictionary[type]; }

        public DevNameFilterType()
        {
            Type = eType.None;
        }        
        
        public void SaveToRegistry()
        {
            RegistryKey key = Registry.LocalMachine.OpenSubKey(Constants.BASE_REGISTRY_PATH, true);
            if (key == null)
            {
                key = Registry.LocalMachine.CreateSubKey(Constants.BASE_REGISTRY_PATH);
            }

            key.SetValue(Constants.DEV_NAME_FILTER_TYPE_REGISTER, Type, RegistryValueKind.DWord);

            key.Close();
        }

        public void RestoreFromRegistry()
        {
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey(Constants.BASE_REGISTRY_PATH, false))
            {
                if (key != null)
                {
                    Type = (eType)key.GetValue(Constants.DEV_NAME_FILTER_TYPE_REGISTER, eType.None);
                }
            }
        }
    }
}
