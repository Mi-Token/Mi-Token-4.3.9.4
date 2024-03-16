using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BLE_API
{
    public partial class BLE
    {
        public partial class Core
        {
            public static class UUIDs
            {
                public static class DEVICE_INFO
                {
                    public const UInt16 ServiceUUID = 0x180A;

                    public const UInt16 SYSTEM_ID = 0x2A23;
                    public const UInt16 FIRMWARE_VERSION = 0x2A26;
                    public const UInt16 HARDWARE_VERSION = 0x2A27;
                }

                public static class PUSH_BUTTON
                {
                    public const UInt16 ServiceUUID = 0xFFE0;

                    public const UInt16 BUTTON_PRESSED = 0xFFE1;
                    public const UInt16 BUTTON_CHARACTERISTIC_CONFIG = 0x2902;
                }

                public static class SYSTEM_CONFIG
                {
                    public const UInt16 ServiceUUID = 0xA500;

                    public const UInt16 BUFFER = 0xA501;
                    public const UInt16 ENCRYPTOR = 0xA502;
                }

                public static class SECURE_BOND
                {
                    public const UInt16 ServiceUUID = 0xA200;

                    public const UInt16 STATUS = 0xA201;
                    public const UInt16 PUBLIC_KEY = 0xA202;
                    public const UInt16 VERIFICATION_BUFFER = 0xA203;
                    public const UInt16 CONFIG = 0xA204;
                    public const UInt16 ENCRYPT_CONFIG = 0xA205;
                }

                public static class AUTHENTICATION
                {
                    public const UInt16 ServiceUUID = 0xA400;

                    public const UInt16 MODE = 0xA401;
                    public const UInt16 STATUS = 0xA402;
                    public const UInt16 BUFFER = 0xA403;
                }

                public static class USER_KEY
                {
                    public const UInt16 ServiceUUID = 0xA300;

                    public const UInt16 KEY_BUFFER = 0xA301;
                }

                public static class HASH_OLD
                {
                    public const UInt16 ServiceUUID = 0xA400;

                    public const UInt16 STATUS = 0xA401;
                    public const UInt16 BUFFER = 0xA402;
                }

                public static class HASH
                {
                    public const UInt16 ServiceUUID = 0xA700;

                    public const UInt16 STATUS = 0xA701;
                    public const UInt16 BUFFER = 0xA702;
                }

                public static class SECRET
                {
                    public const UInt16 ServiceUUID = 0xA600;

                    public const UInt16 Buffer = 0xA601;
                }

                public static class OATH
                {
                    public const UInt16 ServiceUUID = 0xA000;

                    public const UInt16 HOTP_BUFFER = 0xA001;
                    public const UInt16 HOTP_DIGITS = 0xA002;
                    public const UInt16 HOTP_KEY_SOURCE = 0xA003;
                }

            }
        }
    }
}
