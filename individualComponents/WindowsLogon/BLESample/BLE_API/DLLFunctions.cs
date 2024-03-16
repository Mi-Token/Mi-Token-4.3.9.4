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
        internal static class DLLFunctions
        {
            internal static class Inner
            {
                [DllImport("BLEDLLV2.dll")]
                internal static extern int DLL_GetVersion();
                [DllImport("BLEDLLV2.dll")]
                internal static extern int DLL_CheckWrapperVersion(int versionID);
            }

            internal static class V2_1
            {
                internal class MiTokenBLE
                {
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_CreateInterface();
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLE_DeleteInterface(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_Initialize(IntPtr instance, IntPtr COMPort, IntPtr NamedPipePort, bool isServer);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern bool MiTokenBLE_ConnectedToCOM(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_ReInitCOM(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern bool MiTokenBLE_COMOwnerKnown(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_SetCOMWaitHandle(IntPtr instance, IntPtr waitHandle);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_WaitForCOMToBeFree(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_NP_RET MiTokenBLE_ReleaseCOMPort(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLE_SetPollID(IntPtr instance, int newPollID);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern int MiTokenBLE_GetPollID(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern int MiTokenBLE_GetDeviceFoundCount(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_GetDeviceInfo(IntPtr instance, int deviceID, IntPtr pDeviceInfo);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_GetDeviceMetaInfo(IntPtr instance, int deviceID, int flag, IntPtr buffer, IntPtr bufferLength);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_RestartScanner(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_StartFilteredSearch(IntPtr instance, int filterCount, IntPtr filters);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_StartFilteredSearchEx(IntPtr instance, int filterCount, IntPtr filters, int addressCount, IntPtr addressArray);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_StartFilteredSearchFromDLLExt(IntPtr instance, IntPtr filterObject);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_ConvertFilteredSearchToByteStream(IntPtr instance, IntPtr filter, IntPtr byteStream, IntPtr byteStreamLength);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_ConvertByteStreamToFilteredSearch(IntPtr instance, IntPtr byteStream, int byteStreamLength);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLE_ContinueFilteredSearch(IntPtr instance, IntPtr filter, IntPtr pDeviceInfo);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLE_RestartFilteredSearch(IntPtr instance, IntPtr filter);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLE_FinishedFilteredSearch(IntPtr instance, IntPtr filter);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_StartProfile(IntPtr instance, IntPtr macAddress, IntPtr requestID);
                }

                internal class Device
                {
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_Initialize(IntPtr instance, IntPtr macAddress);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern bool MiTokenBLE_CONN_timedOut(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLE_CONN_GetAddress(IntPtr instance, IntPtr macAddress);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_IsConnected(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_ScanServicesInRange(IntPtr instance, UInt16 handle, UInt16 maxRange);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_SetAttribute(IntPtr instance, UInt16 handle, byte length, IntPtr data);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_StartGetAttribute(IntPtr instance, UInt16 handle);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_GetAttribute(IntPtr instance, UInt16 handle, byte maxLength, IntPtr realLength, IntPtr buffer);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_Disconnect(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_PollRSSI(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_GetRSSI(IntPtr instance, IntPtr rssi);
                    //[DllImport("BLEDLLV2.dll")]
                    //internal static extern Core.CONN_RET MiTokenBLE_CONN_ButtonPressedCallback(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_ReadLong(IntPtr instance, UInt16 handle);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_CONN_CreateNewMessageBuffer(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_WriteMessageBuffer(IntPtr instance, IntPtr bufferInstance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_AddMessageToMessageBuffer(IntPtr instance, IntPtr bufferInstance, UInt16 handle, UInt16 length, IntPtr data);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_GetCharacteristicHandle(IntPtr instance, UInt16 handle, IntPtr charHandle);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern int MiTokenBLE_CONN_GetRequestID(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern byte MiTokenBLE_CONN_GetLastError(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLE_CONN_TimeoutRequest(IntPtr instance);
                }

                internal class Ext
                {
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_DLLExt_CreateNewFilter();
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_DLLExt_AddFilter(IntPtr prevFilter, int flag, int len, IntPtr data);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLE_DLLExt_AddAddressFilter(IntPtr rootFilter, int addressCount, IntPtr data);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_DLLExt_FinalizeFilter(IntPtr rootFilter);
                }

                internal class Pipe
                {
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_NP_RET MiTokenBLE_Pipe_HasExclusiveAccess(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_NP_RET MiTokenBLE_Pipe_ReleaseExclusiveAccess(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_NP_RET MiTokenBLE_Pipe_RequestExclusiveAccess(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_NP_RET MiTokenBLE_Pipe_SendMessage(IntPtr instance, IntPtr data, int length, int connectionID);

                    [UnmanagedFunctionPointer(System.Runtime.InteropServices.CallingConvention.Cdecl)]
                    internal delegate void _MiTokenBLE_GotPipeMessage(IntPtr instance, IntPtr data, int length);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_NP_RET MiTokenBLE_Pipe_SetNamedPipeMessageCallback(IntPtr instance, _MiTokenBLE_GotPipeMessage pipeCallback);
                }
            }

            internal static class V2_2
            {
                internal static class MiTokenBLE
                {
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLE_CreateInterfaceOfVersion(int versionID);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLEV2_2_EnableNonUniqueAttributeUUIDMode(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.API_RET MiTokenBLEV2_2_AppendCommandChainLink(IntPtr instance, IntPtr newChainLink, bool checkAndRemoveDuplicatePointers);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLEV2_2_GetV2_2_Interface(IntPtr V2_1instance);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLEV2_2_CreateNewCommandChainLinkObject(IntPtr objSize);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLEV2_2_DeleteCommandChainLinkObject(IntPtr obj);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern int MiTokenBLEV2_2_Inner_GetChainPointerOffset(IntPtr chainObject);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern void MiTokenBLEV2_2_ForceUnsafeDisconnection(IntPtr instance, byte connectionHandle);

                }
                internal static class Device
                {
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncWaitForConnection(IntPtr instance, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncWaitForDisconnection(IntPtr instance, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncScanServicesInRange(IntPtr instance, int handle, ushort maxRange, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncRequestAttribute(IntPtr instance, int handle, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncRequestLongAttribute(IntPtr instance, int handle, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncSetAttribute(IntPtr instance, int handle, byte length, IntPtr data, int timeout);

                    [UnmanagedFunctionPointer(System.Runtime.InteropServices.CallingConvention.StdCall)]
                    internal delegate IntPtr AllocateBuffer(int length);
                    [UnmanagedFunctionPointer(System.Runtime.InteropServices.CallingConvention.StdCall)]
                    internal delegate void DeallocateBuffer(IntPtr buffer);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncReadAttribute(IntPtr instance, int handle, bool longRead, IntPtr dataLen, IntPtr pData, AllocateBuffer allocator, DeallocateBuffer deallocator, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncWaitForState(IntPtr instance, int handle, bool longRead, IntPtr stateList, byte stateCount, byte stateLength, AllocateBuffer allocator, DeallocateBuffer deallocator, int timeout);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_ScanServicesInRangeOfGUIDService(IntPtr instance, ushort handle, ushort maxRange);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncScanServicesInRangeOfGUIDService(IntPtr instance, ushort handle, ushort maxRange, int timout);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_GetCharacteristicHandleEx(IntPtr instance, ushort serviceID, ushort attributeID, IntPtr charHandle, bool serviceIsGUID, bool attributeIsGUID, int skipCount);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncGetCharacteristicHandleEx(IntPtr instance, ushort serviceID, ushort attributeID, IntPtr charHandle, bool serviceIsGUID, bool attributeIsGUID, int skipCount, int timeout);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern IntPtr MiTokenBLEV2_2_CONN_GetV2_2_Interface(IntPtr baseInterface);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncCustomWait(IntPtr instance, bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, ushort checkServiceUUID, ushort checkAttributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SetAttributeFromCharHandle(IntPtr instance, ushort handle, byte length, IntPtr data);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncSetAttributeFromCharHandle(IntPtr instance, ushort handle, byte length, IntPtr data, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_RequestAttributeFromCharHandle(IntPtr instance, ushort handle, bool longRead);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncRequestAttributeFromCharHandle(IntPtr instance, ushort handle, bool longRead, int timeout);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_ReadAttributeFromCharHandle(IntPtr instance, ushort handle, IntPtr length, IntPtr data, AllocateBuffer allocator,  DeallocateBuffer deallocator);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SyncReadAttributeFromCharHandle(IntPtr instance, ushort handle, IntPtr length, IntPtr data, AllocateBuffer allocator, DeallocateBuffer deallocator, int timeout);
	
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_UpdateConnectionParameters(IntPtr instance, ushort minimum, ushort maximum, ushort timeout, ushort latency);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SetWriteBlockBufferCount(IntPtr instance, int newMaxBlocksToBuffer);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SetExtendedWriteOffsetMode(IntPtr instance, bool offsetModeEnabled);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_SetExtendedWriteBlockSize(IntPtr instance, int newMaxBlocksToBuffer);

                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_EnableJeromeAccess(IntPtr instance);
                    [DllImport("BLEDLLV2.dll")]
                    internal static extern Core.CONN_RET MiTokenBLEV2_2_CONN_Jerome_SendNextBlock(IntPtr instance, bool canBeLastBlock);
                }
            }

            internal static class Bond
            {
                [DllImport("BLEDLLV2.dll")]
                internal static extern Core.BOND_RET MiTokenBLEV2_2_CONN_Bond_AddBondToToken(IntPtr bleInstance, IntPtr connInstance, IntPtr sha1Hash, IntPtr bondData);
                [DllImport("BLEDLLV2.dll")]
                internal static extern bool MiTokenBLEV2_2_CONN_Bond_RemoveAll(IntPtr bleInstance, IntPtr connInstance);
            }
        }
    }
}
