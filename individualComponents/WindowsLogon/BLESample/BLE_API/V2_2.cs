using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace BLE_API
{
    public partial class BLE
    {
        public class V2_2
        {
            //Version IDs
            private static int MinVersion_Generic = Inner.Version(2, 2, 7); //Most recent main function definition change.
            private static int MinVersion_CommandChain = Inner.Version(2, 2, 5); //used by AppendCommandChain. Function call format changed in 2.2.5
            internal static int MinVersion_ChainOffset = Inner.Version(2, 2, 3); //used by CommandChain.cs to verify the MiTokenBLEV2_2_Inner_GetChainPointerOffset function
            private static int MinVersion_UpdateConnectionParameters = Inner.Version(2, 2, 4);
            internal static int MinVersion_ForceUnsafeDisconnection = Inner.Version(2, 2, 10);
            internal static int MinVersion_SetWriteBlockBufferCount = Inner.Version(2, 2, 11);
            internal static int MinVersion_JeromeCode = Inner.Version(2, 2, 12);
            internal static int MinVersion_Bonding = Inner.Version(2, 2, 13);

            public class MiTokenBLE : BLE.V2_1.MiTokenBLE
            {
                protected IntPtr _v2_2_interface;
                public MiTokenBLE()
                    : base(false)
                {
                    Inner.VerifyDllVersion(MinVersion_Generic);
                    _interfaceHandle = DLLFunctions.V2_2.MiTokenBLE.MiTokenBLE_CreateInterfaceOfVersion(Core.Interface_Version_2_2);
                    _v2_2_interface = DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_GetV2_2_Interface(_interfaceHandle);
                    AddInterfaceHandle();
                }

                internal IntPtr handle
                {
                    get
                    {
                        return _v2_2_interface;
                    }
                }

                protected override void AddInterfaceHandle()
                {
                    base.AddInterfaceHandle();
                    Inner.interfaceDictionaryV2_2.Add(_v2_2_interface, this);
                }

                new public void DeleteInstance()
                {
                    if (Inner.interfaceDictionaryV2_2.ContainsKey(_v2_2_interface))
                    {
                        Inner.interfaceDictionaryV2_2.Remove(_v2_2_interface);
                    }
                    _v2_2_interface = IntPtr.Zero;

                    base.DeleteInstance();
                }

                new public void CreateNewInstance()
                {
                    if ((_interfaceHandle != IntPtr.Zero) || (_v2_2_interface != IntPtr.Zero))
                    {
                        DeleteInstance();
                    }

                    ResetNewPollResults();
                    _interfaceHandle = DLLFunctions.V2_2.MiTokenBLE.MiTokenBLE_CreateInterfaceOfVersion(Core.Interface_Version_2_2);
                    _v2_2_interface = DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_GetV2_2_Interface(_interfaceHandle);

                    AddInterfaceHandle();
                }

                public void EnableNonUniqueAttributeUUIDMode()
                {
                    DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_EnableNonUniqueAttributeUUIDMode(_v2_2_interface);
                }

                public void Pipe_SyncWaitForExclusive()
                {
                    var ret = Pipe_HasExclusiveAccess();
                    while (ret != Core.API_NP_RET.BLE_API_EXCLUSIVE_ALLOWED)
                    {
                        System.Threading.Thread.Sleep(1000);
                        ret = Pipe_HasExclusiveAccess();
                        if (ret == Core.API_NP_RET.BLE_API_EXCLUSIVE_REJECTED)
                        {
                            return;
                        }
                    }

                    return;
                }

                new public Device connectToDevice(Core.Structures.DeviceInfo deviceToConnectTo)
                {
                    Device newDevice = new Device(this, deviceToConnectTo.macAddress);

                    return newDevice;
                }

                public Core.API_RET AppendCommandChain(CommandChain chain, bool checkAndRemoveDuplicatePointers = false)
                {
                    Inner.VerifyDllVersion(MinVersion_CommandChain);

                    IntPtr ipObject;
                    IntPtr ipObjSize = IntPtr.Zero;

                    byte[] objByte = new byte[4];
                    Core.API_RET retCode = Core.API_RET.BLE_API_SUCCESS;

                    try
                    {
                        ipObjSize = PointerHandling.allocatePtrForData(objByte);
                        ipObject = DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_CreateNewCommandChainLinkObject(ipObjSize);
                        if (ipObject == IntPtr.Zero)
                        {
                            return Core.API_RET.BLE_API_ERR_NULL_POINTER;
                        }

                        PointerHandling.setFromPtr(ipObjSize, objByte);
                        int size = BitConverter.ToInt32(objByte, 0);
                        CommandChain.setChainPointerValues(ipObject, chain);

                        DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_AppendCommandChainLink(_v2_2_interface, ipObject, checkAndRemoveDuplicatePointers);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipObjSize);
                    }

                    return retCode;
                }

                public void ForceUnsafeDisconnection(byte connectionHandle)
                {
                    Inner.VerifyDllVersion(MinVersion_ForceUnsafeDisconnection);

                    DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_ForceUnsafeDisconnection(_v2_2_interface, connectionHandle);
                }
            }

            public class Device : BLE.V2_1.Device
            {
                static DLLFunctions.V2_2.Device.AllocateBuffer bufferAllocator = (int length) =>
                {
                    IntPtr buffer = PointerHandling.allocateForSize(length);
                    return buffer;
                };

                static DLLFunctions.V2_2.Device.DeallocateBuffer bufferDeallocator = (IntPtr buffer) =>
                {
                    PointerHandling.deallocatePtr(buffer);
                };

                IntPtr _v2_2_interface;
                V2_2.MiTokenBLE _commandV2_2_Interface;

                public Device(V2_2.MiTokenBLE bleInterface, byte[] deviceMac)
                    : base(bleInterface, deviceMac)
                {
                    _commandV2_2_Interface = bleInterface;
                    Inner.VerifyDllVersion(MinVersion_Generic);
                    _v2_2_interface = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_GetV2_2_Interface(_myInterface);
                }

                public Core.CONN_RET SyncWaitForConnection(int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncWaitForConnection(_v2_2_interface, timeout);
                }

                public Core.CONN_RET SyncWaitForDisconnection(int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncWaitForDisconnection(_v2_2_interface, timeout);
                }

                public Core.CONN_RET SyncScanServicesInRange(int handle, ushort maxRange, int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncScanServicesInRange(_v2_2_interface, handle, maxRange, timeout);
                }
                public Core.CONN_RET SyncScanServicesInRange(UInt16 handle, UInt16 maxRange, int timeout)
                {
                    return SyncScanServicesInRange((int)handle, (ushort)maxRange, timeout);
                }

                public Core.CONN_RET SyncRequestAttribute(int handle, int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncRequestAttribute(_v2_2_interface, handle, timeout);
                }

                public Core.CONN_RET SyncRequestLongAttribute(int handle, int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncRequestLongAttribute(_v2_2_interface, handle, timeout);
                }

                public Core.CONN_RET SyncSetAttribute(int handle, byte[] data, int timeout)
                {
                    Core.CONN_RET ret = 0;

                    IntPtr ipData = IntPtr.Zero;
                    byte dataLength = (byte)data.Length;
                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(data);

                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncSetAttribute(_v2_2_interface, handle, dataLength, ipData, timeout);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }

                    return ret;
                }

                public Core.CONN_RET SyncReadAttribute(int handle, bool longRead, out byte[] data, int timeout)
                {
                    data = new byte[0];
                    IntPtr ipDataLen = IntPtr.Zero;
                    IntPtr ipPtrData = IntPtr.Zero;

                    Core.CONN_RET ret = 0;

                    try
                    {
                        ipDataLen = PointerHandling.callocateForSize(4);
                        ipPtrData = PointerHandling.callocateForSize(IntPtr.Size);

                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncReadAttribute(_v2_2_interface, handle, longRead, ipDataLen, ipPtrData, bufferAllocator, bufferDeallocator, timeout);

                        byte[] length = new byte[4];
                        Marshal.Copy(ipDataLen, length, 0, 4);
                        byte[] intPtrLocation = new byte[IntPtr.Size];
                        Marshal.Copy(ipPtrData, intPtrLocation, 0, IntPtr.Size);
                        IntPtr ip;
                        if (IntPtr.Size == 8)
                        {
                            Int64 i64Data = BitConverter.ToInt64(intPtrLocation, 0);
                            ip = new IntPtr(i64Data);
                        }
                        else
                        {
                            Int32 i32Data = BitConverter.ToInt32(intPtrLocation, 0);
                            ip = new IntPtr(i32Data);
                        }
                        Int32 i32Length = BitConverter.ToInt32(length, 0);
                        if ((i32Length == 0) && (ip == IntPtr.Zero))
                        {
                            data = new byte[0];
                        }
                        else
                        {
                            data = new byte[i32Length];
                            Marshal.Copy(ip, data, 0, i32Length);
                            bufferDeallocator(ip);
                        }
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipDataLen);
                        PointerHandling.deallocatePtr(ipPtrData);
                    }


                    return ret;

                }

                public Core.CONN_RET SyncWaitForState(int handle, bool longRead, List<byte[]> states, int timeout)
                {
                    IntPtr ipStateList = IntPtr.Zero;

                    Core.CONN_RET ret = 0;

                    //Set up the complete state buffer
                    if (states.Count == 0)
                    {
                        return Core.CONN_RET.BLE_CONN_BAD_REQUEST;
                    }
                    int stateLen = states[0].Length;
                    for (int i = 1; i < states.Count; ++i)
                    {
                        if (states[i].Length != stateLen)
                        {
                            return Core.CONN_RET.BLE_CONN_BAD_REQUEST;
                        }
                    }

                    byte[] stateArray = new byte[states.Count * stateLen];
                    for (int i = 0; i < states.Count; ++i)
                    {
                        Array.Copy(states[i], 0, stateArray, i * stateLen, stateLen);
                    }

                    try
                    {
                        ipStateList = PointerHandling.allocatePtrForData(stateArray);

                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncWaitForState(_v2_2_interface, handle, longRead, ipStateList, (byte)states.Count, (byte)stateLen, bufferAllocator, bufferDeallocator, timeout);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipStateList);
                    }

                    return ret;
                }

                public Core.CONN_RET ScanServicesInRangeOfGUIDService(ushort handle, ushort maxRange)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_ScanServicesInRangeOfGUIDService(_v2_2_interface, handle, maxRange);
                }

                public Core.CONN_RET SyncScanServicesInRangeOfGUIDService(ushort handle, ushort maxRange, int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncScanServicesInRangeOfGUIDService(_v2_2_interface, handle, maxRange, timeout);
                }

                public Core.CONN_RET GetCharacteristicHandleEx(ushort serviceID, ushort attributeID, out short charHandle, bool serviceIsGUID = false, bool attributeIsGUID = false, int skipCount = 0)
                {
                    charHandle = 0;
                    Core.CONN_RET ret = Core.CONN_RET.BLE_CONN_BAD_REQUEST;

                    IntPtr ipCharHandle = IntPtr.Zero;
                    try
                    {
                        ipCharHandle = PointerHandling.allocateForSize(2);
                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_GetCharacteristicHandleEx(_v2_2_interface, serviceID, attributeID, ipCharHandle, serviceIsGUID, attributeIsGUID, skipCount);

                        byte[] b2 = new byte[2];
                        PointerHandling.setFromPtr(ipCharHandle, b2);
                        charHandle = BitConverter.ToInt16(b2, 0);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipCharHandle);
                    }

                    return ret;
                }

                public Core.CONN_RET SyncGetCharacteristicHandleEx(ushort serviceID, ushort attributeID, out ushort charHandle, int timeout, bool serviceIsGUID = false, bool attributeIsGUID = false, int skipCount = 0)
                {
                    charHandle = 0;
                    Core.CONN_RET ret = Core.CONN_RET.BLE_CONN_BAD_REQUEST;

                    IntPtr ipCharHandle = IntPtr.Zero;
                    try
                    {
                        ipCharHandle = PointerHandling.allocateForSize(2);
                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncGetCharacteristicHandleEx(_v2_2_interface, serviceID, attributeID, ipCharHandle, serviceIsGUID, attributeIsGUID, skipCount, timeout);

                        byte[] b2 = new byte[2];
                        PointerHandling.setFromPtr(ipCharHandle, b2);
                        charHandle = BitConverter.ToUInt16(b2, 0);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipCharHandle);
                    }

                    return ret;
                }

                public Core.CONN_RET SyncCustomWait(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, ushort checkServiceUUID, ushort checkAttributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount, int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncCustomWait(_v2_2_interface, checkConnection, checkServiceScan, checkAttributeScan, checkWriting, checkReading, checkServiceUUID, checkAttributeUUID, serviceUUIDIsGUID, attributeUUIDIsGUID, skipCount, timeout);
                }

                public Core.CONN_RET SyncCustomWait(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, ushort attributeUUID, int timeout)
                {
                    return SyncCustomWait(checkConnection, checkServiceScan, checkAttributeScan, checkWriting, checkReading, 0x0000, attributeUUID, false, false, 0, timeout);
                }

                public Core.CONN_RET SetAttributeFromCharHandle(ushort charHandle, byte[] data)
                {
                    IntPtr ipData = IntPtr.Zero;
                    Core.CONN_RET ret = Core.CONN_RET.BLE_CONN_BAD_REQUEST;

                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(data);
                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SetAttributeFromCharHandle(_v2_2_interface, charHandle, (byte)data.Length, ipData);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }

                    return ret;
                }

                public Core.CONN_RET SyncSetAttributeFromCharHandle(ushort charHandle, byte[] data, int timeout)
                {
                    IntPtr ipData = IntPtr.Zero;
                    Core.CONN_RET ret = Core.CONN_RET.BLE_CONN_BAD_REQUEST;

                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(data);
                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncSetAttributeFromCharHandle(_v2_2_interface, charHandle, (byte)data.Length, ipData, timeout);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }

                    return ret;
                }

                public Core.CONN_RET RequestAttributeFromCharHandle(ushort charHandle, bool longRead)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_RequestAttributeFromCharHandle(_v2_2_interface, charHandle, longRead);
                }

                public Core.CONN_RET SyncRequestAttributeFromCharHandle(ushort charHandle, bool longRead, int timeout)
                {
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncRequestAttributeFromCharHandle(_v2_2_interface, charHandle, longRead, timeout);
                }

                public Core.CONN_RET ReadAttributeFromCharHandle(ushort handle, out byte[] data)
                {
                    data = new byte[0];
                    IntPtr ipDataLen = IntPtr.Zero;
                    IntPtr ipPtrData = IntPtr.Zero;

                    Core.CONN_RET ret = 0;

                    try
                    {
                        ipDataLen = PointerHandling.callocateForSize(4);
                        ipPtrData = PointerHandling.callocateForSize(IntPtr.Size);

                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_ReadAttributeFromCharHandle(_v2_2_interface, handle, ipDataLen, ipPtrData, bufferAllocator, bufferDeallocator);

                        byte[] length = new byte[4];
                        Marshal.Copy(ipDataLen, length, 0, 4);
                        byte[] intPtrLocation = new byte[IntPtr.Size];
                        Marshal.Copy(ipPtrData, intPtrLocation, 0, IntPtr.Size);
                        IntPtr ip;
                        if (IntPtr.Size == 8)
                        {
                            Int64 i64Data = BitConverter.ToInt64(intPtrLocation, 0);
                            ip = new IntPtr(i64Data);
                        }
                        else
                        {
                            Int32 i32Data = BitConverter.ToInt32(intPtrLocation, 0);
                            ip = new IntPtr(i32Data);
                        }
                        Int32 i32Length = BitConverter.ToInt32(length, 0);
                        data = new byte[i32Length];
                        Marshal.Copy(ip, data, 0, i32Length);
                        bufferDeallocator(ip);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipDataLen);
                        PointerHandling.deallocatePtr(ipPtrData);
                    }


                    return ret;
                }


                public Core.CONN_RET SyncReadAttributeFromCharHandle(ushort handle, out byte[] data, int timeout)
                {
                    data = new byte[0];
                    IntPtr ipDataLen = IntPtr.Zero;
                    IntPtr ipPtrData = IntPtr.Zero;

                    Core.CONN_RET ret = 0;

                    try
                    {
                        ipDataLen = PointerHandling.callocateForSize(4);
                        ipPtrData = PointerHandling.callocateForSize(IntPtr.Size);

                        ret = DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SyncReadAttributeFromCharHandle(_v2_2_interface, handle, ipDataLen, ipPtrData, bufferAllocator, bufferDeallocator, timeout);

                        byte[] length = new byte[4];
                        Marshal.Copy(ipDataLen, length, 0, 4);
                        byte[] intPtrLocation = new byte[IntPtr.Size];
                        Marshal.Copy(ipPtrData, intPtrLocation, 0, IntPtr.Size);
                        IntPtr ip;
                        if (IntPtr.Size == 8)
                        {
                            Int64 i64Data = BitConverter.ToInt64(intPtrLocation, 0);
                            ip = new IntPtr(i64Data);
                        }
                        else
                        {
                            Int32 i32Data = BitConverter.ToInt32(intPtrLocation, 0);
                            ip = new IntPtr(i32Data);
                        }
                        Int32 i32Length = BitConverter.ToInt32(length, 0);
                        data = new byte[i32Length];
                        Marshal.Copy(ip, data, 0, i32Length);
                        bufferDeallocator(ip);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipDataLen);
                        PointerHandling.deallocatePtr(ipPtrData);
                    }


                    return ret;
                }

                public Core.CONN_RET UpdateConnectionParameters(UInt16 minimum, UInt16 maximum, UInt16 timeout, UInt16 latency)
                {
                    Inner.VerifyDllVersion(MinVersion_UpdateConnectionParameters);
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_UpdateConnectionParameters(_v2_2_interface, minimum, maximum, timeout, latency);
                }

                public Core.CONN_RET SetWriteBlockBufferCount(int newMaxBlocksToBuffer)
                {
                    Inner.VerifyDllVersion(MinVersion_SetWriteBlockBufferCount);
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SetWriteBlockBufferCount(_v2_2_interface, newMaxBlocksToBuffer);
                }

                public Core.CONN_RET SetExtendedWriteOffsetMode(bool offsetModeEnabled)
                {
                    Inner.VerifyDllVersion(MinVersion_SetWriteBlockBufferCount);
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SetExtendedWriteOffsetMode(_v2_2_interface, offsetModeEnabled);
                }
                public Core.CONN_RET SetExtendedWriteBlockSize(int newMaxBytesPerBlock)
                {
                    Inner.VerifyDllVersion(MinVersion_SetWriteBlockBufferCount);
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_SetExtendedWriteBlockSize(_v2_2_interface, newMaxBytesPerBlock);
                }

                public Core.CONN_RET EnableJeromeAccess()
                {
                    Inner.VerifyDllVersion(MinVersion_JeromeCode);
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_EnableJeromeAccess(_v2_2_interface);
                }

                public Core.CONN_RET Jerome_SendNextBlock(bool canBeLastBlock)
                {
                    Inner.VerifyDllVersion(MinVersion_JeromeCode);
                    return DLLFunctions.V2_2.Device.MiTokenBLEV2_2_CONN_Jerome_SendNextBlock(_v2_2_interface, canBeLastBlock);
                }

                public Core.BOND_RET AddBond(out byte[] bondData)
                {
                    Inner.VerifyDllVersion(MinVersion_Bonding);

                    bondData = null;
                    IntPtr ipBondData = IntPtr.Zero;
                    Core.BOND_RET ret = Core.BOND_RET.BOND_RET_FAILED;
                    try
                    {
                        ipBondData = PointerHandling.allocateForSize(32);
                        ret = DLLFunctions.Bond.MiTokenBLEV2_2_CONN_Bond_AddBondToToken(_commandV2_2_Interface.handle, _v2_2_interface, IntPtr.Zero, ipBondData);
                        if (ret == Core.BOND_RET.BOND_RET_SUCCESS)
                        {
                            bondData = new byte[20];
                            PointerHandling.setFromPtr(ipBondData, bondData);
                        }
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipBondData);
                    }
                    return ret;
                }

                public bool RemoveAllBonds()
                {
                    Inner.VerifyDllVersion(MinVersion_Bonding);

                    return DLLFunctions.Bond.MiTokenBLEV2_2_CONN_Bond_RemoveAll(_commandV2_2_Interface.handle, _v2_2_interface);
                }
            }
        }

    }
}
