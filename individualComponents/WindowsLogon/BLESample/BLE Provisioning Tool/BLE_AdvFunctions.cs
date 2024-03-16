using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using BLE_API;

namespace BLE_Provisioning_Tool
{
    class BLE_AdvFunctions
    {
        static private readonly int MAX_ATTRIBUTE_READ_PKT_SIZE = 22; // bytes
        static public readonly int UNDEFINED_LENGTH = 0xFFFF;
        public const int PROCESSING_TIMEOUT_CONNECTION_MS = 5000; // Type different as used in a parameter default
        static public readonly int PROCESSING_TIMEOUT_SHORT_MS = 1000;
        static public readonly int PROCESSING_TIMEOUT_LONG_MS = 5000;

        private static readonly UInt16 FAST_CONN_UPDATE_MIN = 8; // 1.25ms units
        private static readonly UInt16 FAST_CONN_UPDATE_MAX = 16; // 1.25ms units
        private static readonly UInt16 FAST_CONN_UPDATE_TIMEOUT = 50; // 10ms units
        private static readonly UInt16 FAST_CONN_UPDATE_LATENCY = 0;

        private static readonly UInt16 SLOW_CONN_UPDATE_MIN = 30; // 1.25ms units
        private static readonly UInt16 SLOW_CONN_UPDATE_MAX = 30; // 1.25ms units
        private static readonly UInt16 SLOW_CONN_UPDATE_TIMEOUT = 50; // 10ms units
        private static readonly UInt16 SLOW_CONN_UPDATE_LATENCY = 0;

        static public async Task<Tuple<bool, byte[]>> ReadRequest(BLE.V2_2.Device connectedDevice, UInt16 UUID, int expectedByteLength)
        {            
            return await Task.Run<Tuple<bool, byte[]>>(() =>
            {
                bool bResult = false;
                byte[] resultBytes = null;

                // Determine if normal or long read is required
                if (expectedByteLength <= MAX_ATTRIBUTE_READ_PKT_SIZE)
                {
                    // Perform normal read
                    bResult = BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.SyncReadAttribute(UUID, false, out resultBytes, PROCESSING_TIMEOUT_SHORT_MS);
                }
                else
                {
                    // Perform long read, also includes UNDEFINED_LENGTH
                    bResult = BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.SyncReadAttribute(UUID, true, out resultBytes, PROCESSING_TIMEOUT_SHORT_MS);
                }

                return new Tuple<bool, byte[]>(bResult, resultBytes);
            });
        }

        static public async Task<bool> WriteRequest(BLE.V2_2.Device connectedDevice, UInt16 UUID, byte[] dataToWrite)
        {
            return await Task.Run<bool>(() =>
            {
                return (BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.SyncSetAttribute(UUID, dataToWrite, PROCESSING_TIMEOUT_SHORT_MS));
            });
        }

        static public async Task<bool> WaitForDisconnection(BLE.V2_2.Device connectedDevice, int timeoutMs)
        {
            return await Task.Run<bool>(() =>
            {
                return (BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.SyncWaitForDisconnection(timeoutMs));
            });
        }

        static public async Task<bool> CheckConnection(BLE.V2_2.Device connectedDevice, ManualResetEventSlim procedureCompletedEvent, int timeoutMs = PROCESSING_TIMEOUT_CONNECTION_MS)
        {            
            return await Task.Run<bool>(() =>
            {
                if (BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT == connectedDevice.SyncWaitForConnection(timeoutMs))
                {
                    if (BLE.Core.CONN_RET.BLE_CONN_ERR_CONNECTION_TIMED_OUT == connectedDevice.TimeoutRequest())
                    {
                        //we have successfully requested the timeout
                        connectedDevice.Disconnect();                        
                    }
                    return false;
                }

                Debug.WriteLine(String.Format("{0}\tCheckConnection:Waiting for residual events to complete", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
#if DEBUG_DIAGNOSTICS
                Stopwatch sw = new Stopwatch();
                sw.Start();
#endif
                procedureCompletedEvent.Reset();
                procedureCompletedEvent.Wait(2000);                                
#if DEBUG_DIAGNOSTICS
                sw.Stop();
                Debug.WriteLine(String.Format("{0}\tCheckConnection:procedureCompletedEvent ElapsedMilliseconds={1}", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt"), sw.ElapsedMilliseconds));
#endif
                Debug.WriteLine(String.Format("{0}\tCheckConnection:Residual events to complete!", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
                return true;
            });
        }

        static public async Task<bool> RefreshConnection(BLE.V2_2.Device connectedDevice)
        {
            return await Task.Run<bool>(() =>
            {
                return connectedDevice.RefreshConnection();
            });
        }

        static public async Task<bool> FindServiceAttributes(BLE.V2_2.Device connectedDevice, UInt16 serviceUUID, bool isGUID = false)
        {
            return await Task.Run<bool>(() =>
            {
                if (isGUID)
                {
                    return (BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.SyncScanServicesInRangeOfGUIDService(serviceUUID, 0xFF, PROCESSING_TIMEOUT_SHORT_MS));
                }
                else
                {
                    return (BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.SyncScanServicesInRange(serviceUUID, 0xFF, PROCESSING_TIMEOUT_SHORT_MS));
                }
            });
        }

        static public async Task<bool> UpdateConnectionParameters(BLE.V2_2.Device connectedDevice, ManualResetEventSlim resultEvent, SemaphoreSlim resultSempaphore, bool isFast = true)
        {
            Debug.WriteLine(String.Format("{0}\tUpdateConnectionParameters:Wait for semaphore to be free", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            await resultSempaphore.WaitAsync();
            Debug.WriteLine(String.Format("{0}\tUpdateConnectionParameters:Sempahore now free", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));

            // Reset event
            resultEvent.Reset();
            bool result = await Task.Run<bool>(() =>
            {
                // Send Connection Update Parameter
                Debug.WriteLine(String.Format("{0}\tUpdateConnectionParameters:Send connection update", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
                bool returnResult = false;
                if (isFast)
                {
                    returnResult = (BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.UpdateConnectionParameters(FAST_CONN_UPDATE_MIN, FAST_CONN_UPDATE_MAX, FAST_CONN_UPDATE_TIMEOUT, FAST_CONN_UPDATE_LATENCY));                    
                }
                else
                {
                    returnResult = (BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.UpdateConnectionParameters(SLOW_CONN_UPDATE_MIN, SLOW_CONN_UPDATE_MAX, SLOW_CONN_UPDATE_TIMEOUT, SLOW_CONN_UPDATE_LATENCY));
                }

                // Wait for response
                if (returnResult)
                {
                    Debug.WriteLine(String.Format("{0}\tUpdateConnectionParameters:Wait for connection update response", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
                    returnResult = resultEvent.Wait(PROCESSING_TIMEOUT_SHORT_MS);
                }
                
                return returnResult;
            });
            
            resultSempaphore.Release();
            Debug.WriteLine(String.Format("{0}\tUpdateConnectionParameters:Released Semaphore", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            return result;
        }

        static public async Task<Tuple<bool, UInt16>>GetCharacteristicHandleEx(BLE.V2_2.Device connectedDevice, ushort serviceID, ushort attributeID, bool serviceIsGUID = false, bool attributeIsGUID = false, int skipCount= 0)
        {
            return await Task.Run<Tuple<bool, UInt16>>(() =>
            {
                UInt16 hnd = 0;
                bool bResult = BLE.Core.CONN_RET.BLE_CONN_SUCCESS == connectedDevice.SyncGetCharacteristicHandleEx(serviceID,attributeID, out hnd, PROCESSING_TIMEOUT_SHORT_MS, serviceIsGUID, attributeIsGUID, skipCount);
                return new Tuple<bool, UInt16>(bResult, hnd);                
            });
        }

        static public async Task<bool>SetAttributeFromCharHandle(BLE.V2_2.Device connectedDevice, ushort charHnd, byte[] data)
        {
            return await Task.Run<bool>(() =>
            {
                return (BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT == connectedDevice.SyncSetAttributeFromCharHandle(charHnd, data, PROCESSING_TIMEOUT_SHORT_MS));
            });
        }
        
        static public async Task<bool> ReadRequestUntilEqual(BLE.V2_2.Device connectedDevice, UInt16 UUID, byte[] data1Expected, byte[] data2Expected = null)
        {
            var task = ReadRequestUntilEqualNoTimeout(connectedDevice, UUID, data1Expected, data2Expected);
            if (await Task.WhenAny(task, Task.Delay(BLE_AdvFunctions.PROCESSING_TIMEOUT_LONG_MS)) == task)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        static private async Task<bool> ReadRequestUntilEqualNoTimeout(BLE.V2_2.Device connectedDevice, UInt16 UUID, byte[] data1Expected, byte[] data2Expected = null)
        {            
            while(true)
            {
                var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, UUID, data1Expected.Length);
                if (result.Item1)
                {
                    if (Helper.ArraysEqual(result.Item2, data1Expected))
                    {
                        return true;
                    }

                    if (data2Expected != null)
                    {
                        if (Helper.ArraysEqual(result.Item2, data2Expected))
                        {
                            return true;
                        }
                    }                    
                }
                else
                {
                    return false;
                }
            }
        }

        static public async Task<Tuple<bool, byte[]>> MT_ReadRequest(BLE.V2_2.Device connectedDevice, UInt16 UUID, byte[] cmd, int expectedDataByteLength)
        {
            // Write command to retrieve intended Variable
            if (await WriteRequest(connectedDevice, UUID, cmd))
            {
                // Retrieve Value of Variable
                return await BLE_AdvFunctions.ReadRequest(connectedDevice, UUID, expectedDataByteLength);
            }
            return new Tuple<bool, byte[]>(false,null);
        }
    }
}
