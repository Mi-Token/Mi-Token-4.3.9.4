using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MiTokenBLEWatcher
{
    public static class MiTokenBLEUUID
    {
        public static class SERVICE_UUID
        {
            public const UInt16 DEVICE_INFO = 0x180A;
            public const UInt16 PUSH_BUTTON = 0xFFE0;
            public const UInt16 SYSTEM_CONFIG = 0xA500;
            public const UInt16 SECURE_BOND = 0xA200;
            public const UInt16 AUTHENTICATION = 0xA400;
            public const UInt16 USER_KEY = 0xA300;

            public const UInt16 HASH_OLD = 0xA400;
            public const UInt16 HASH = 0xA700;

            public const UInt16 SECRET = 0xA600;
            public const UInt16 OATH = 0xA000;
        }

        public static class ATTRIBUTE_UUID
        {
            public static class DEVICE_INFO
            {
                public const UInt16 SYSTEM_ID = 0x2A23;
                public const UInt16 FIRMWARE_VERSION = 0x2A26;
                public const UInt16 HARDWARE_VERSION = 0x2A27;
            }

            public static class HASH_OLD
            {
                public const UInt16 STATUS = 0xA401;
                public const UInt16 BUFFER = 0xA402;
            }

            public static class HASH
            {
                public const UInt16 STATUS = 0xA701;
                public const UInt16 BUFFER = 0xA702;
            }

        }
        
    }
}
