using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BLE_Configuration_Tool
{
    static class MiToken_BLE_UUIDs
    {
        public static readonly ushort BLE_UUID_SERVICE_DEVICE_INFO = 0x180A;
        public static readonly ushort BLE_UUID_SERVICE_PUSH_BUTTON = 0xFFE0;
        public static readonly ushort BLE_UUID_SERVICE_SYSTEM_CONFIG = 0xA500;
        public static readonly ushort BLE_UUID_SERVICE_SECURE_BOND = 0xA200;
        public static readonly ushort BLE_UUID_SERVICE_AUTHENTICATION = 0xA400;
        public static readonly ushort BLE_UUID_SERVICE_USER_KEY = 0xA300;
        public static readonly ushort BLE_UUID_SERVICE_HASH = 0xA400;
        public static readonly ushort BLE_UUID_SERVICE_SECRET = 0xA600;
        public static readonly ushort BLE_UUID_SERVICE_OATH = 0xA000;
        //Attributes
        public static readonly ushort BLE_UUID_ATTRIBUTE_DEVICE_INFO_SYSTEM_ID = 0x2A23;
        public static readonly ushort BLE_UUID_ATTRIBUTE_DEVICE_INFO_FIRMWARE_VERSION = 0x2A26;
        public static readonly ushort BLE_UUID_ATTRIBUTE_DEVICE_INFO_HARDWARE_VERSION = 0x2A27;

        public static readonly ushort BLE_UUID_ATTRIBUTE_PUSH_BUTTON_BUTTON_PRESSED = 0xFFE1;
        public static readonly ushort BLE_UUID_ATTRIBUTE_PUSH_BUTTON_CHARACTERISTIC_CONFIG = 0x2902;

        public static readonly ushort BLE_UUID_ATTRIBUTE_SYSTEM_CONFIG_BUFFER = 0xA501;
        public static readonly ushort BLE_UUID_ATTRIBUTE_SYSTEM_CONFIG_ENCRYPTOR = 0xA502;

        public static readonly ushort BLE_UUID_ATTRIBUTE_SECURE_BOND_STATUS = 0xA201;
        public static readonly ushort BLE_UUID_ATTRIBUTE_SECURE_BOND_PUBLIC_KEY = 0xA202;
        public static readonly ushort BLE_UUID_ATTRIBUTE_SECURE_BOND_VERIFICATION_BUFFER = 0xA203;
        public static readonly ushort BLE_UUID_ATTRIBUTE_SECURE_BOND_CONFIG = 0xA204;
        public static readonly ushort BLE_UUID_ATTRIBUTE_SECURE_BOND_ENCRYPT_CONFIG = 0xA205;

        public static readonly ushort BLE_UUID_ATTRIBUTE_AUTHENTICATION_MODE = 0xA401;
        public static readonly ushort BLE_UUID_ATTRIBUTE_AUTHENTICATION_STATUS = 0xA402;
        public static readonly ushort BLE_UUID_ATTRIBUTE_AUTHENTICATION_BUFFER = 0xA403;

        public static readonly ushort BLE_UUID_ATTRIBUTE_USER_KEY_BUFFER = 0xA301;

        public static readonly ushort BLE_UUID_ATTRIBUTE_HASH_STATUS = 0xA401;
        public static readonly ushort BLE_UUID_ATTRIBUTE_HASH_BUFFER = 0xA402;

        public static readonly ushort BLE_UUID_ATTRIBUTE_SECRET_BUFFER = 0xA601;

        public static readonly ushort BLE_UUID_ATTRIBUTE_OATH_HOTP_BUFFER = 0xA001;
        public static readonly ushort BLE_UUID_ATTRIBUTE_OATH_HOTP_DIGITS = 0xA002;
        public static readonly ushort BLE_UUID_ATTRIBUTE_OATH_HOTP_KEY_SOURCE = 0xA003;
    }
}
