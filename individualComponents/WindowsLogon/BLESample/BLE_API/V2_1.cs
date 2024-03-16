using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Threading;

namespace BLE_API
{
    public partial class BLE
    {
        public class V2_1
        {
            //Version IDs
            private static int MinVersion_Generic = Inner.Version(2, 1, 0);
            private static int MinVersion_TimeoutRequest = Inner.Version(2, 2, 10); //quite a late addition

            public delegate void MiTokenBLE_GotPipeMessage(BLE.V2_1.MiTokenBLE instance, byte[] data);
            public static event MiTokenBLE_GotPipeMessage MiTokenBLE_GotMessageCallback;

            internal static DLLFunctions.V2_1.Pipe._MiTokenBLE_GotPipeMessage priv_MiTokenBLE_GotPipeMessage = (IntPtr instance, IntPtr pData, int length) =>
            {
                if (BLE.V2_1.MiTokenBLE_GotMessageCallback != null)
                {
                    if (BLE.Inner.interfaceDictionary.ContainsKey(instance))
                    {
                        BLE.V2_1.MiTokenBLE mtInstance = BLE.Inner.interfaceDictionary[instance];
                        byte[] data = new byte[length];
                        PointerHandling.setFromPtr(pData, data);

                        BLE.V2_1.MiTokenBLE_GotMessageCallback(mtInstance, data);
                    }
                }
            };

            public partial class MiTokenBLE
            {
                protected IntPtr _interfaceHandle;
                protected Thread _pollThread;
                protected int _currentPollID;
                protected int _pollIDDelay = 100;
                protected int _pollThreadCount = 0;

                public delegate void NewPollResultsDelegate(MiTokenBLE sender);
                public event NewPollResultsDelegate NewPollResults;

                public MiTokenBLE()
                    : this(true)
                { }

                protected MiTokenBLE(bool initHandle)
                {
                    Inner.DLLV2_1_Allowed();
                    Inner.VerifyDllVersion(MinVersion_Generic);

                    _pollThread = null;
                    _currentPollID = 0;
                    if (initHandle)
                    {
                        _interfaceHandle = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_CreateInterface();
                        Inner.interfaceDictionary.Add(_interfaceHandle, this);
                    }
                }
                protected virtual void AddInterfaceHandle()
                {
                    Inner.interfaceDictionary.Add(_interfaceHandle, this);
                }

                internal IntPtr myHandle { get { return _interfaceHandle; } }

                public void CreateNewInstance()
                {
                    Inner.VerifyDllVersion(MinVersion_Generic);
                    if(_interfaceHandle != IntPtr.Zero)
                    {
                        DeleteInstance();
                    }

                    ResetNewPollResults();
                    _interfaceHandle = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_CreateInterface();
                    Inner.interfaceDictionary.Add(_interfaceHandle, this);
                }

                protected void ResetNewPollResults()
                {
                    NewPollResults = null;
                }

                public void DeleteInstance()
                {
                    if (_pollThreadCount > 0)
                    {
                        _pollThreadCount = 0;
                        RemovePollThreadInstance();
                    }

                    if (_interfaceHandle != IntPtr.Zero)
                    {
                        if (Inner.interfaceDictionary.ContainsKey(_interfaceHandle))
                        {
                            Inner.interfaceDictionary.Remove(_interfaceHandle);
                        }

                        IntPtr tempHandle = _interfaceHandle;
                        _interfaceHandle = IntPtr.Zero;
                        DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_DeleteInterface(tempHandle);
                    }

                }

                private void PollThreadFunction()
                {

                    while (true)
                    {
                        _currentPollID++;
                        SetPollID(_currentPollID);
                        Thread.Sleep(_pollIDDelay);
                        if (NewPollResults != null) { NewPollResults(this); }
                    }
                }

                public int CurrentPollID { get { return _currentPollID; } }

                public void AddPollThreadInstance()
                {
                    if (_pollThread == null)
                    {
                        (_pollThread = new Thread(PollThreadFunction)).Start();
                    }

                    _pollThreadCount++;
                }

                public void RemovePollThreadInstance()
                {
                    _pollThreadCount--;
                    if ((_pollThreadCount <= 0) && (_pollThread != null))
                    {
                        _pollThread.Abort();
                        _pollThread = null;
                        _pollThreadCount = 0;
                    }
                }


                public Core.API_RET Initialize(string COMPort, string NamedPipePort, bool isServer)
                {
                    IntPtr ipCOMPort = IntPtr.Zero, ipNamedPipe = IntPtr.Zero;
                    Core.API_RET ret = 0;
                    try
                    {
                        ipCOMPort = PointerHandling.allocatePtrForString(COMPort);
                        ipNamedPipe = PointerHandling.allocatePtrForString(NamedPipePort);

                        ret = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_Initialize(_interfaceHandle, ipCOMPort, ipNamedPipe, isServer);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipCOMPort);
                        PointerHandling.deallocatePtr(ipNamedPipe);
                    }

                    return ret;
                }

                public bool ConnectedToCOM()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_ConnectedToCOM(_interfaceHandle);
                }

                public Core.API_RET ReinitCOM()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_ReInitCOM(_interfaceHandle);
                }

                public bool COMOwnerKnown()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_COMOwnerKnown(_interfaceHandle);
                }

                private int SetCOMWaitHandle()
                {
                    throw new NotImplementedException();
                }

                public Core.API_RET WaitForCOMToBeFree()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_WaitForCOMToBeFree(_interfaceHandle);
                }

                public Core.API_NP_RET ReleaseCOMPort()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_ReleaseCOMPort(_interfaceHandle);
                }

                public void SetPollID(int newPollID)
                {
                    DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_SetPollID(_interfaceHandle, newPollID);
                }

                public int GetPollID()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_GetPollID(_interfaceHandle);
                }

                public int GetDeviceFoundCount()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_GetDeviceFoundCount(_interfaceHandle);
                }

                private static int BytePtrToBLEDeviceInfo(byte[] encBytes, ref Core.Structures.DeviceInfo deviceInfo)
                {
                    if ((deviceInfo.macAddress == null) || (deviceInfo.macAddress.Length != 6))
                    {
                        deviceInfo.macAddress = new byte[6];
                    }

                    for (int i = 0; i < 6; ++i)
                    {
                        deviceInfo.macAddress[i] = encBytes[i];
                    }

                    deviceInfo.RSSI = (char)encBytes[6];

                    deviceInfo.lastPollSeen = (uint)BitConverter.ToInt32(encBytes, 7);

                    return 0;
                }

                public Core.API_RET GetDeviceInfo(int deviceID, ref Core.Structures.DeviceInfo deviceInfo)
                {
                    IntPtr devInfo = IntPtr.Zero;
                    Core.API_RET retCode = 0;
                    byte[] encBytes = new byte[11];
                    try
                    {
                        devInfo = PointerHandling.allocatePtrForData(encBytes);
                        retCode = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_GetDeviceInfo(_interfaceHandle, deviceID, devInfo);
                        PointerHandling.setFromPtr(devInfo, encBytes);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(devInfo);
                    }

                    if (retCode != 0)
                    {
                        return retCode;
                    }

                    BytePtrToBLEDeviceInfo(encBytes, ref deviceInfo);

                    return retCode;
                }

                public Core.API_RET GetDeviceMetaInfo(int deviceID, int flag, out byte[] data)
                {
                    data = null;
                    byte[] lenByte = new byte[4];
                    IntPtr ipLen = IntPtr.Zero, ipData = IntPtr.Zero;

                    Core.API_RET retCode = 0;

                    try
                    {
                        ipLen = PointerHandling.allocatePtrForData(lenByte);
                        retCode = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_GetDeviceMetaInfo(_interfaceHandle, deviceID, flag, ipData, ipLen);
                        PointerHandling.setFromPtr(ipLen, lenByte);

                        if (retCode == Core.API_RET.BLE_API_MORE_DATA)
                        {
                            int IVal = BitConverter.ToInt32(lenByte, 0);
                            byte[] binData = new byte[IVal];
                            ipData = PointerHandling.allocatePtrForData(binData);
                            retCode = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_GetDeviceMetaInfo(_interfaceHandle, deviceID, flag, ipData, ipLen);
                            PointerHandling.setFromPtr(ipData, binData);
                            data = binData;
                        }
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                        PointerHandling.deallocatePtr(ipLen);
                    }

                    return retCode;
                }

                public Core.API_RET RestartScanner()
                {
                    return DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_RestartScanner(_interfaceHandle);
                }

                public Filter CreateFilter()
                {
                    return new Filter(this);
                }

                public Device connectToDevice(Core.Structures.DeviceInfo deviceToConnectTo)
                {
                    Device newDevice = new Device(this, deviceToConnectTo.macAddress);

                    return newDevice;
                }

                
            }

            public class Device
            {
                protected MiTokenBLE _connectedInterface;
                protected IntPtr _myInterface;
                protected byte[] _myMac;
                protected int _myUID;
                public Device(MiTokenBLE bleInterface, byte[] deviceMac)
                {
                    Inner.DLLV2_1_Allowed();

                    Inner.VerifyDllVersion(MinVersion_Generic);
                    _connectedInterface = bleInterface;
                    _myMac = deviceMac;
                    _myUID = 0;

                    //call RefreshConnection - this will start a profile of this device mac
                    //Note : We ignore the return code, it will always be false the first time anyway.
                    RefreshConnection();
                }

                public bool SuccessfulReturn(BLE.Core.CONN_RET retCode, out string strCode)
                {
                    strCode = retCode.ToString();
                    return (retCode == Core.CONN_RET.BLE_CONN_SUCCESS);
                }

                public bool RefreshConnection()
                {
                    IntPtr ipMac = IntPtr.Zero, ipUID = IntPtr.Zero;
                    byte[] dataZero = new byte[4];

                    int _oldUID = _myUID;
                    try
                    {
                        ipMac = PointerHandling.allocatePtrForData(_myMac);
                        ipUID = PointerHandling.allocatePtrForData(dataZero);
                        _myInterface = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_StartProfile(_connectedInterface.myHandle, ipMac, ipUID);
                        PointerHandling.setFromPtr(ipUID, dataZero);

                        _myUID = BitConverter.ToInt32(dataZero, 0);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipUID);
                        PointerHandling.deallocatePtr(ipMac);
                    }

                    return (_myUID == _oldUID);
                }

                Core.CONN_RET Initialize(byte[] deviceMac)
                {
                    throw new NotImplementedException();
                }

                public bool TimedOut()
                {
                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_timedOut(_myInterface);
                }

                public void GetAddress(ref byte[] addr)
                {
                    if (addr == null || (addr.Length != 6))
                    {
                        addr = new byte[6];
                    }

                    IntPtr ipMac = IntPtr.Zero;
                    try
                    {
                        PointerHandling.allocatePtrForData(addr);
                        DLLFunctions.V2_1.Device.MiTokenBLE_CONN_GetAddress(_myInterface, ipMac);
                        PointerHandling.setFromPtr(ipMac, addr);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipMac);
                    }
                }
                public Core.CONN_RET IsConnected()
                {
                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_IsConnected(_myInterface);
                }

                public Core.CONN_RET ScanServicesInRange(UInt16 handle, UInt16 maxRange)
                {
                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_ScanServicesInRange(_myInterface, handle, maxRange);
                }

                public Core.CONN_RET SetAttribute(UInt16 handle, byte[] data)
                {
                    IntPtr ipData = IntPtr.Zero;

                    Core.CONN_RET retCode = Core.CONN_RET.BLE_CONN_SUCCESS;
                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(data);
                        byte len = (byte)data.Length;
                        retCode = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_SetAttribute(_myInterface, handle, len, ipData);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }

                    return retCode;
                }

                public Core.CONN_RET StartGetAttribute(UInt16 handle)
                {
                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_StartGetAttribute(_myInterface, handle);
                }

                public Core.CONN_RET GetAttribute(UInt16 handle, out byte[] data)
                {
                    data = null;
                    IntPtr ipData = IntPtr.Zero, ipDataLen = IntPtr.Zero;
                    byte[] dataLen = new byte[4];
                    Core.CONN_RET retCode = Core.CONN_RET.BLE_CONN_SUCCESS;

                    try
                    {
                        ipDataLen = PointerHandling.allocatePtrForData(dataLen);
                        retCode = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_GetAttribute(_myInterface, handle, 0, ipDataLen, ipData);
                        if (retCode == Core.CONN_RET.BLE_CONN_SUCCESS)
                        {
                            PointerHandling.setFromPtr(ipDataLen, dataLen);
                            int iDataLen = BitConverter.ToInt32(dataLen, 0);

                            byte[] tempdata = new byte[iDataLen];
                            ipData = PointerHandling.allocatePtrForData(tempdata);
                            retCode = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_GetAttribute(_myInterface, handle, (byte)iDataLen, ipDataLen, ipData);
                            PointerHandling.setFromPtr(ipData, tempdata);
                            data = tempdata;
                        }
                    }
                    finally
                    { }

                    return retCode;
                }

                public Core.CONN_RET Disconnect()
                {
                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_Disconnect(_myInterface);
                }

                public Core.CONN_RET PollRSSI()
                {
                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_PollRSSI(_myInterface);
                }

                public Core.CONN_RET GetRssi(out byte rssi)
                {
                    IntPtr ipRSSI = IntPtr.Zero;
                    Core.CONN_RET retCode;
                    byte[] bRSSI = new byte[1];
                    try
                    {
                        ipRSSI = PointerHandling.allocatePtrForData(bRSSI);
                        retCode = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_GetRSSI(_myInterface, ipRSSI);
                        PointerHandling.setFromPtr(ipRSSI, bRSSI);
                        rssi = bRSSI[0];
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipRSSI);
                    }

                    return retCode;

                }

                public Core.CONN_RET ReadLong(UInt16 handle)
                {
                    Core.CONN_RET retCode = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_ReadLong(_myInterface, handle);

                    return retCode;
                }

                public byte GetLastError()
                {
                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_GetLastError(_myInterface);
                }

                public Core.CONN_RET TimeoutRequest()
                {
                    Inner.VerifyDllVersion(MinVersion_TimeoutRequest);

                    return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_TimeoutRequest(_myInterface);
                }

                public MessageBuffer CreateNewMessageBuffer()
                {
                    return new MessageBuffer(this);
                }

                public class MessageBuffer
                {
                    Device _myDevice;
                    IntPtr _myInterface;
                    public MessageBuffer(Device connectedDevice)
                    {
                        _myDevice = connectedDevice;
                        _myInterface = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_CreateNewMessageBuffer(_myDevice._myInterface);
                    }

                    public Core.CONN_RET Write()
                    {
                        return DLLFunctions.V2_1.Device.MiTokenBLE_CONN_WriteMessageBuffer(_myDevice._myInterface, _myInterface);
                    }

                    public Core.CONN_RET AddMessageToMessageBuffer(UInt16 handle, byte[] data)
                    {
                        IntPtr ipData = IntPtr.Zero;
                        Core.CONN_RET retCode;

                        try
                        {
                            ipData = PointerHandling.allocatePtrForData(data);
                            retCode = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_AddMessageToMessageBuffer(_myDevice._myInterface, _myInterface, handle, (UInt16)data.Length, ipData);
                        }
                        finally
                        {
                            PointerHandling.deallocatePtr(ipData);
                        }

                        return retCode;
                    }
                };

                public Core.CONN_RET GetCharacteristicsHandle(UInt16 handle, out UInt16 characteristicsHandle)
                {
                    IntPtr ipData = IntPtr.Zero;

                    Core.CONN_RET retCode;
                    byte[] bdata = new byte[2];

                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(bdata);
                        retCode = DLLFunctions.V2_1.Device.MiTokenBLE_CONN_GetCharacteristicHandle(_myInterface, handle, ipData);
                        PointerHandling.setFromPtr(ipData, bdata);
                        characteristicsHandle = BitConverter.ToUInt16(bdata, 0);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }

                    return retCode;
                }


            };

            public class Filter
            {
                IntPtr _rootFilter, _lastFilter, _completedFilter;
                bool allowAdditions, allowSearches;
                MiTokenBLE _interfaceHandle;

                public Filter(MiTokenBLE MiTokenInterface)
                {
                    Inner.DLLV2_1_Allowed();

                    _interfaceHandle = MiTokenInterface;

                    Inner.VerifyDllVersion(MinVersion_Generic);

                    _rootFilter = DLLFunctions.V2_1.Ext.MiTokenBLE_DLLExt_CreateNewFilter();
                    _lastFilter = _rootFilter;
                    allowAdditions = true;
                    allowSearches = false;
                }


                public bool AddFilter(char flag, byte[] data)
                {
                    if (!allowAdditions)
                    {
                        return false;
                    }

                    IntPtr ipData = IntPtr.Zero;
                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(data);
                        _lastFilter = DLLFunctions.V2_1.Ext.MiTokenBLE_DLLExt_AddFilter(_lastFilter, flag, data.Length, ipData);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }
                    return true;
                }

                public bool AddAddressFilter(byte[] addressList)
                {
                    if (!allowAdditions)
                    {
                        return false;
                    }
                    if (addressList.Length % 6 != 0)
                    {
                        return false;
                    }

                    IntPtr ipData = IntPtr.Zero;
                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(addressList);
                        DLLFunctions.V2_1.Ext.MiTokenBLE_DLLExt_AddAddressFilter(_rootFilter, addressList.Length / 6, ipData);
                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }

                    return true;
                }

                public bool FinalizeFilter()
                {
                    if ((!allowAdditions) || (allowSearches))
                    {
                        return false;
                    }

                    allowAdditions = false;
                    _completedFilter = DLLFunctions.V2_1.Ext.MiTokenBLE_DLLExt_FinalizeFilter(_rootFilter);
                    _rootFilter = IntPtr.Zero; //invalidated after the call to MiTokenBLE_DLLExt_FinalizeFilter
                    _lastFilter = IntPtr.Zero;
                    allowSearches = true;
                    return true;
                }

                public Searcher createNewSearcher()
                {
                    if (!allowSearches)
                    {
                        return null;
                    }

                    return new Searcher(_interfaceHandle, _completedFilter);
                }

                public int ConvertFilteredSearchToByteStream()
                {
                    throw new NotImplementedException();
                }

                public IntPtr ConvertByteStreamToFilteredSearch()
                {
                    throw new NotImplementedException();
                }

                public int ContinueFilteredSearch()
                {
                    throw new NotImplementedException();
                }

                public void RestartFilteredSearch()
                {
                    throw new NotImplementedException();
                }

                public void FinishFilteredSearch()
                {
                    throw new NotImplementedException();
                }


                public class Searcher
                {
                    private IntPtr _mySearcher;
                    bool canContinue;
                    MiTokenBLE _myInterface;

                    public Searcher(MiTokenBLE MiTokenInterface, IntPtr filter)
                    {
                        Inner.DLLV2_1_Allowed();
                        Inner.VerifyDllVersion(MinVersion_Generic);

                        _myInterface = MiTokenInterface;
                        _mySearcher = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_StartFilteredSearchFromDLLExt(MiTokenInterface.myHandle, filter);
                        canContinue = true;
                    }

                    public Searcher(MiTokenBLE MiTokenInterface, byte[] filterByteSteam)
                    {
                        Inner.DLLV2_1_Allowed();
                        Inner.VerifyDllVersion(MinVersion_Generic);

                        _myInterface = MiTokenInterface;
                        IntPtr ipByteStream = IntPtr.Zero;
                        _mySearcher = IntPtr.Zero;

                        try
                        {
                            ipByteStream = PointerHandling.allocatePtrForData(filterByteSteam);
                            _mySearcher = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_ConvertByteStreamToFilteredSearch(MiTokenInterface.myHandle, ipByteStream, filterByteSteam.Length);
                        }
                        finally
                        {
                            PointerHandling.deallocatePtr(ipByteStream);
                        }
                        canContinue = true;
                    }

                    public byte[] GetByteStreamFromSearcher()
                    {
                        IntPtr ipByteStream = IntPtr.Zero, ipLen = IntPtr.Zero;
                        byte[] zeroBin = { 0, 0, 0, 0 };
                        byte[] bData = null;
                        Core.API_RET retCode;

                        try
                        {
                            ipLen = PointerHandling.allocatePtrForData(zeroBin);
                            retCode = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_ConvertFilteredSearchToByteStream(_myInterface.myHandle, _mySearcher, ipByteStream, ipLen);
                            if (retCode == Core.API_RET.BLE_API_MORE_DATA)
                            {
                                PointerHandling.setFromPtr(ipLen, zeroBin);
                                int length = BitConverter.ToInt32(zeroBin, 0);
                                byte[] tempRet = new byte[length];
                                ipByteStream = PointerHandling.allocatePtrForData(tempRet);
                                retCode = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_ConvertFilteredSearchToByteStream(_myInterface.myHandle, _mySearcher, ipByteStream, ipLen);
                                if (retCode == 0)
                                {
                                    PointerHandling.setFromPtr(ipByteStream, tempRet);
                                    bData = tempRet;
                                }
                            }
                        }
                        finally
                        {
                            PointerHandling.deallocatePtr(ipByteStream);
                            PointerHandling.deallocatePtr(ipLen);
                        }

                        return bData;
                    }

                    public void CloseFilter()
                    {
                        if (_mySearcher != IntPtr.Zero)
                        {
                            canContinue = false;
                            DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_FinishedFilteredSearch(_myInterface.myHandle, _mySearcher);
                            _mySearcher = IntPtr.Zero;
                            _myInterface = null;
                        }
                    }

                    public bool GetNextNode(ref Core.Structures.DeviceInfo deviceInfo)
                    {
                        if (!canContinue)
                        {
                            return false;
                        }

                        byte[] encBytes = new byte[11];
                        IntPtr ipData = IntPtr.Zero;
                        Core.API_RET retCode = 0;
                        try
                        {
                            ipData = PointerHandling.allocatePtrForData(encBytes);
                            retCode = DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_ContinueFilteredSearch(_myInterface.myHandle, _mySearcher, ipData);
                            PointerHandling.setFromPtr(ipData, encBytes);
                        }
                        finally
                        {
                            PointerHandling.deallocatePtr(ipData);
                        }

                        if (retCode != 0)
                        {
                            canContinue = false;
                            DLLFunctions.V2_1.MiTokenBLE.MiTokenBLE_FinishedFilteredSearch(_myInterface.myHandle, _mySearcher);
                            _mySearcher = IntPtr.Zero;
                            return false;
                        }

                        deviceInfo.FromBytes(encBytes);

                        return true;
                    }

                }
            }


            public partial class MiTokenBLE
            {
                #region NamedPipe Functions
                public Core.API_NP_RET Pipe_HasExclusiveAccess()
                {
                    return DLLFunctions.V2_1.Pipe.MiTokenBLE_Pipe_HasExclusiveAccess(_interfaceHandle);
                }

                public Core.API_NP_RET Pipe_ReleaseExclusiveAccess()
                {
                    return DLLFunctions.V2_1.Pipe.MiTokenBLE_Pipe_ReleaseExclusiveAccess(_interfaceHandle);
                }

                public Core.API_NP_RET Pipe_RequestExclusiveAccess()
                {
                    return DLLFunctions.V2_1.Pipe.MiTokenBLE_Pipe_RequestExclusiveAccess(_interfaceHandle);
                }

                public Core.API_NP_RET Pipe_SendMessage(byte[] data, int connectionID)
                {
                    IntPtr ipData = IntPtr.Zero;
                    Core.API_NP_RET retCode = Core.API_NP_RET.BLE_API_NP_SUCCESS;

                    try
                    {
                        ipData = PointerHandling.allocatePtrForData(data);
                        retCode = DLLFunctions.V2_1.Pipe.MiTokenBLE_Pipe_SendMessage(_interfaceHandle, ipData, data.Length, connectionID);

                    }
                    finally
                    {
                        PointerHandling.deallocatePtr(ipData);
                    }

                    return retCode;
                }

                public Core.API_NP_RET Pipe_SetMessageCallback()
                {
                    return DLLFunctions.V2_1.Pipe.MiTokenBLE_Pipe_SetNamedPipeMessageCallback(_interfaceHandle, V2_1.priv_MiTokenBLE_GotPipeMessage);
                }
                #endregion
            }
        }
    }
}
