using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Timers;
using BLE_API;
using System.Threading;
using System.Diagnostics;

namespace BLE_Provisioning_Tool
{
    // The Image Header will not be encrypted, but it will be included in a Signature.
    public class ImgHdr
    {
        // Secure OAD uses the Signature for image validation instead of calculating a CRC, but the use
        // of CRC==CRC-Shadow for quick boot-up determination of a validated image is still used.
        public UInt16 crc0;       // CRC must not be 0x0000 or 0xFFFF.
        public UInt16 crc1;       // CRC-shadow must be 0xFFFF.
        // User-defined Image Version Number - default logic uses simple a '!=' comparison to start an OAD.
        public UInt16 ver;
        public UInt16 len;        // Image length in 4-byte blocks (i.e. HAL_FLASH_WORD_SIZE blocks).
        public byte[] uid;     // User-defined Image Identification bytes.
        public byte[] res;     // Reserved space for future use.

        static public readonly byte UID_LEN = 4;
        static public readonly byte RES_LEN = 4;
        static public readonly int TOTAL_LEN = sizeof(UInt16) + sizeof(UInt16) + sizeof(UInt16) + sizeof(UInt16) + UID_LEN + RES_LEN;

        public ImgHdr()
        {
            uid = new Byte[UID_LEN];
            res = new Byte[RES_LEN];
        }

        public bool isImgA()
        {
            return (bool)((ver & 0x1) == 0);
        }

        public int getVerNum()
        {
            return (ver >> 0x01);
        }

        public int getByteSize()
        {
            return len * (OadService.OAD_BLOCK_SIZE / OadService.HAL_FLASH_WORD_SIZE);
        }

        public String getByteSizeStr()
        {
            return String.Format("{0:n0} bytes", getByteSize());
        }

        public int getBlockSize()
        {
            return len / (OadService.OAD_BLOCK_SIZE / OadService.HAL_FLASH_WORD_SIZE);
        }

        public String getTypeStr()
        {
            if (isImgA())
                return "A";
            else
                return "B";
        }

        public String getUidStr()
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i != UID_LEN; ++i)
            {
                sb.Append(System.Text.Encoding.UTF8.GetString(new byte[] { uid[i] }));
                if (i != UID_LEN - 1)
                    sb.Append(":");
            }
            return sb.ToString();
        }

        public String getVerNumStr()
        {
            return Convert.ToString(getVerNum());
        }

        public byte[] Serialize()
        {
            // Exclude CRC, CRC-Shadow and Reserved bytes
            byte[] b = new byte[TOTAL_LEN - (sizeof(UInt16) + sizeof(UInt16) + RES_LEN)];
            int count = 0;

            // Add Version
            byte[] bver = BitConverter.GetBytes(ver);
            Buffer.BlockCopy(bver, 0, b, count, sizeof(UInt16));
            count += sizeof(UInt16);

            // Add Length
            byte[] blen = BitConverter.GetBytes(len);
            Buffer.BlockCopy(blen, 0, b, count, sizeof(UInt16));
            count += sizeof(UInt16);

            // Add User Identifer
            Buffer.BlockCopy(uid, 0, b, count, sizeof(UInt16));
            //count += UID_LEN; // Not required
            return b;
        }

        public bool ReadNewImgHdr(byte[] buff)
        {
            // Check length of buffer
            if (buff.Length != TOTAL_LEN)
                return false;

            // Skip CRC and CRC-Shadow bytes
            int count = 4; 

            // Read version
            ver = BitConverter.ToUInt16(buff, count);
            count += 2;

            // Read length
            len = BitConverter.ToUInt16(buff, count);
            count += 2;

            // Read user identifer
            for (int i=0; i != UID_LEN; ++i)
            {
                uid[i] = buff[count];
                count++;
            }

            // Read reserved bytes
            for (int i = 0; i != RES_LEN; ++i)
            {
                res[i] = buff[count];
                count++;
            }
            return true;
        }

        public bool ReadTargetImgHdr(byte[] buff)
        {
            // Check length of buffer
            if (buff.Length < TOTAL_LEN - (sizeof(UInt16) + sizeof(UInt16) + RES_LEN))
            {
                return false;
            }
            int count = 0;

            // Read version
            ver = BitConverter.ToUInt16(buff, count);
            count += 2;

            // Read length
            len = BitConverter.ToUInt16(buff, count);
            count += 2;

            // Read user identifer
            for (int i = 0; i != UID_LEN; ++i)
            {
                uid[i] = buff[count];
                count++;
            }
            return true;
        }

        public bool isUidValid(ImgHdr compareHdr)
        {
            // Return as valid if firmware does not use UID for validation yet
            String thisUidStr = getUidStr();
            if ((isImgA() && (thisUidStr == "B:B:B:B")) || (!isImgA() && (thisUidStr == "A:A:A:A")))
            {
                return true;
            }
            
            // Compare only first three characters of UID string (including deliminator) as represent form factor and device chip
            String compareUidStr = compareHdr.getUidStr();
            if (String.Compare(thisUidStr, 0, compareUidStr, 0, 2+1) != 0)
            {
                return false;
            }

            // Check that if current image header is A then comparision image header should not be 'A' and also if current image header is B then comparison image header is 'A'
            if (((isImgA() && compareUidStr.Substring(4, 0) != "A")) || (!isImgA() && compareUidStr.Substring(4, 0) == "A"))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    public class OadService
    {
        // Attribute UUIDs
        static public readonly UInt16 SERV_UUID = 0xFFC0;
        static public readonly UInt16 IMG_IDENTITY_UUID = 0xFFC1;
        static public readonly UInt16 IMG_BLOCK_UUID = 0xFFC2;
        static public readonly UInt16 GATT_CLIENT_CHAR_CFG_UUID = 0x2902;

        // Characteristic Configuration IDs
        static public readonly int IMG_IDENTITY_CHAR_CFG_ID = 0;
        static public readonly int IMG_BLOCK_CHAR_CFG_ID = 1;

        // OAD Constants
        static public readonly int OAD_BLOCK_SIZE = 16;
        static public readonly int HAL_FLASH_WORD_SIZE = 4;
        static public readonly long ESTIMATED_AVG_BLOCK_TIME_MS_NORMAL_MODE = 30; // Based on initial tests with fast connection parameters i.e. FAST_CONN_UPDATE_
        static public readonly long ESTIMATED_AVG_BLOCK_TIME_MS_FAST_MODE = 15; // Based on initial tests with fast connection parameters i.e. FAST_CONN_UPDATE_
        static private readonly int FLASH_PAGE_SIZE_BYTES = 2048;
        static public readonly int IMAGE_A_MAX_SIZE_BYTES = 47 * FLASH_PAGE_SIZE_BYTES;
        static public readonly int IMAGE_B_MAX_SIZE_BYTES = (124 * FLASH_PAGE_SIZE_BYTES) - IMAGE_A_MAX_SIZE_BYTES;
        public const int MAX_OAD_BLOCKS_PER_REQ_FAST_MODE = 3;

        // Target image header
        static ImgHdr m_targetImgHdr = null;

        // Represents if OAD process has begun (to ignore block notifications prior to start of OAD process)
        static bool m_isRunning = false;

        // Properties
        [System.ComponentModel.DefaultValue(0)] 
        public UInt16 m_crcIdentityCCCHnd { get; set; }

        [System.ComponentModel.DefaultValue(0)]
        public UInt16 m_crcBlockCCCHnd { get; set; }

        [System.ComponentModel.DefaultValue(0)]
        static UInt16 m_crcIdentityValHnd { get; set; }

        [System.ComponentModel.DefaultValue(0)]
        static UInt16 m_crcBlockValHnd { get; set; }

        public class UiData
        {
            public TimeSpan EstTimeRemaining { get; set; }
            public int CurrentBytes { get; set; }
            public int TotalBytes { get; set; }
            public int StatusPercent { get; set; }
        };

        public OadService()
        {
            m_isRunning = false;
        }

        public ImgHdr getTargetImgHdr()
        {
            return m_targetImgHdr;
        }

        // Image Identity and Block callback
        static AutoResetEvent m_identityAttributeEvent = new AutoResetEvent(false);
        static AutoResetEvent m_blockAttributeEvent = new AutoResetEvent(false);
        static public BLE.CommandChain.CommandChainCall attribute_value = (IntPtr caller, IntPtr data) =>
        {
            BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_VALUE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_VALUE(data);

            if (msg.atthandle == m_crcIdentityValHnd)
            {
                ImgHdr imgHdr = new ImgHdr();
                if (imgHdr.ReadTargetImgHdr(msg.value))
                {
                    m_targetImgHdr = imgHdr;
                    m_identityAttributeEvent.Set();
                }
            }

            else if (msg.atthandle == m_crcBlockValHnd)
            {
                // Check is OAd processing is running
                if (m_isRunning)
                {
                    // Retrieve block count
                    m_currentBlockCount = BitConverter.ToUInt16(msg.value, 0);
                    m_blockAttributeEvent.Set();
                }
            }
            
            return;
        };        

        public async Task<bool> RetrieveTargetImgHdr(BLE.V2_2.Device connectedDevice)
        {
            // Ensure running flag is cleared
            m_isRunning = false;

            // Ensure OAD service exists
            await BLE_AdvFunctions.FindServiceAttributes(connectedDevice, OadService.SERV_UUID, true);

            // Get Characteristic handles for Identity and Block attributes
            {
                var result = await BLE_AdvFunctions.GetCharacteristicHandleEx(connectedDevice, OadService.SERV_UUID, OadService.GATT_CLIENT_CHAR_CFG_UUID, true, false, OadService.IMG_IDENTITY_CHAR_CFG_ID);
                if (result.Item1)
                {
                    m_crcIdentityCCCHnd = result.Item2;
                }
                else
                {
                    return false;
                }                
            }

            {
                var result = await BLE_AdvFunctions.GetCharacteristicHandleEx(connectedDevice, OadService.SERV_UUID, OadService.IMG_IDENTITY_UUID, true, true, 0);
                if (result.Item1)
                {
                    m_crcIdentityValHnd = result.Item2;
                }
                else
                {
                    return false;
                }
            }

            {
                var result = await BLE_AdvFunctions.GetCharacteristicHandleEx(connectedDevice, OadService.SERV_UUID, OadService.GATT_CLIENT_CHAR_CFG_UUID, true, false, OadService.IMG_BLOCK_CHAR_CFG_ID);
                if (result.Item1)
                {
                    m_crcBlockCCCHnd = result.Item2;
                }
                else
                {
                    return false;
                }
            }

            {
                var result = await BLE_AdvFunctions.GetCharacteristicHandleEx(connectedDevice, OadService.SERV_UUID, OadService.IMG_BLOCK_UUID, true, true, 0);
                if (result.Item1)
                {
                    m_crcBlockValHnd = result.Item2;
                }
                else
                {
                    return false;
                }
            }

            // Manually reset identity attribute event
            m_identityAttributeEvent.Reset();

            // Register for Identity and Block Attribute Notifications and request target's image header from identity attribute (Send both Image A and B header requests)
            byte[] registerNotifyData = new byte[] { 0x01, 0x00 };
            byte[] reqCurrImgAData = new byte[] { 0x00 };
            byte[] reqCurrImgBData = new byte[] { 0x01 };
            if (await BLE_AdvFunctions.SetAttributeFromCharHandle(connectedDevice, m_crcIdentityCCCHnd, registerNotifyData))
            {
                return false;
            }
            if (await BLE_AdvFunctions.SetAttributeFromCharHandle(connectedDevice, m_crcIdentityValHnd, reqCurrImgAData))
            {
                return false;
            }
            if (await BLE_AdvFunctions.SetAttributeFromCharHandle(connectedDevice, m_crcIdentityValHnd, reqCurrImgBData))
            {
                return false;
            }

            // Wait for Target Image header to be populated                
            return await Task.Run<bool>(() =>
            {
                return m_identityAttributeEvent.WaitOne(BLE_AdvFunctions.PROCESSING_TIMEOUT_SHORT_MS);
            });
        }

        // Total number of blocks to expected to be received 
        static int m_totalBlockCount = 0;

        // Current block count to be sent (updated by image block notification)
        static int m_currentBlockCount = 0;

        // Represents estimated time remaining
        static TimeSpan estRemainingTime = new TimeSpan();

        public async Task<bool> PerformOAD(BLE.V2_2.Device connectedDevice, ManualResetEventSlim connectionDisconnectedEvent, ImgHdr imgHdr, String newImgFileName, IProgress<UiData> uiData, CancellationToken cancelToken, bool isFastMode, int oadBlocksPerReqFastMode = MAX_OAD_BLOCKS_PER_REQ_FAST_MODE)
        {           
            // Calculate the total number of OAD blocks to be transferred for a successful OAD
            int totalBlockCount = imgHdr.len / (OAD_BLOCK_SIZE / HAL_FLASH_WORD_SIZE);
            if (totalBlockCount > UInt16.MaxValue)
            {
                return false;
            }
            m_totalBlockCount = totalBlockCount;

            // Read entire file
            byte[][] blockTransferData = new byte[m_totalBlockCount][];
            if (File.Exists(newImgFileName))
            {
                using (BinaryReader br = new BinaryReader(File.Open(newImgFileName, FileMode.Open)))
                {
                    // Read rest of file
                    UInt16 blockCount = 0;
                    while (blockCount != m_totalBlockCount)
                    {
                        // Attempt to read a block of bytes
                        byte[] blockDataBytes = br.ReadBytes(OAD_BLOCK_SIZE);

                        // If bytes read not correct size then fill with zeroes
                        if (blockDataBytes.Length < OAD_BLOCK_SIZE)
                        {
                            byte[] tmp1 = new byte[OAD_BLOCK_SIZE];
                            Buffer.BlockCopy(blockDataBytes, 0, tmp1, 0, blockDataBytes.Length);
                            blockDataBytes = tmp1;
                        }

                        // Package data ready to be sent
                        blockTransferData[blockCount] = Helper.BuildByte(new byte[][] {
                            BitConverter.GetBytes(blockCount),
                            blockDataBytes
                        });

                        // Increment block count
                        blockCount++;
                    }
                }
            }
            else
            {
                return false;
            }

            // Ensure this is set to zero
            m_currentBlockCount = 0;

            // Ensure running flag is set
            m_isRunning = true;

            // Ensure normal offset mode is set
            connectedDevice.SetExtendedWriteOffsetMode(true);

            // Send image header            
            if (BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT == connectedDevice.SyncSetAttributeFromCharHandle(m_crcIdentityValHnd, imgHdr.Serialize(), 10000))
            {
                return false;
            }

            // Apply settings for fast/normal mode            
            if (isFastMode)
            {
                // Setup BLE for non-acknowledge(fast) writes
                connectedDevice.SetExtendedWriteOffsetMode(false);
                connectedDevice.EnableJeromeAccess();

                // Create buffer for all blocks and headers  
                BLE.V2_1.Device.MessageBuffer buff = new BLE.V2_1.Device.MessageBuffer(connectedDevice);
                for (int cnt = 0; cnt != blockTransferData.Length; ++cnt)
                {
                    buff.AddMessageToMessageBuffer(m_crcBlockValHnd, blockTransferData[cnt]);
                }
                buff.Write();
            }
            else
            {
                // Manually reset block attribute event
                m_blockAttributeEvent.Reset();

                // Register for block notifications
                byte[] registerNotifyData = new byte[] { 0x01, 0x00 };
                if (await BLE_AdvFunctions.SetAttributeFromCharHandle(connectedDevice, m_crcBlockCCCHnd, registerNotifyData))
                {
                    return false;
                }
            }

            // Block StopWatch
            Stopwatch blockSw = new Stopwatch();
            blockSw.Start();

            // Block Queue used for time left estimation
            Queue<long> blockTimes = new Queue<long>(); // Represents time taken to send a block and for the token to be ready to receive a another block
            uint maxBlockTimeQueue = (isFastMode ? (uint)160 : (uint)80); // Set this to a large value so that estimated time is not changed too frequently
            long averageBlockTime = 0; // Initialise this to 0 to indicate queue is being filled for first time

            // Ensure this is reset back to zero
            m_currentBlockCount = 0;

            // Required to jump start normal mode
            if (!isFastMode)
            {
                // Send a single acknowledge block
                if (BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT == connectedDevice.SyncSetAttributeFromCharHandle(m_crcBlockValHnd, blockTransferData[m_currentBlockCount++], BLE_AdvFunctions.PROCESSING_TIMEOUT_SHORT_MS))
                {
                    // Update UI
                    updateUI(uiData);

                    // Return result
                    return isOadComplete();
                }
            }
            
            // Continue to send image blocks until complete or timer (2 second) expires
            while (m_currentBlockCount < m_totalBlockCount)
            {
                // Check if cancellation has been requested
                if (cancelToken.IsCancellationRequested)
                {
                    // Ensure to disable non-acknowledged writes
                    if (isFastMode)
                    {
                        connectedDevice.SetExtendedWriteOffsetMode(true);
                    }

                    Debug.WriteLine(String.Format("PerformOAD:Cancel"));

                    return false;
                }

                // Check is device disconnected
                if (connectionDisconnectedEvent.IsSet)
                {
                     // Ensure to disable non-acknowledged writes
                    if (isFastMode)
                    {
                        connectedDevice.SetExtendedWriteOffsetMode(true);
                    }

                    Debug.WriteLine(String.Format("PerformOAD:Disconnected"));

                    return false;
                }

                // Wait for next block request to arrive
                if (!isFastMode)
                {
                    if (!(await Task.Run<bool>(() => { return m_blockAttributeEvent.WaitOne(2000); })))
                    {
                        Debug.WriteLine(String.Format("PerformOAD:Block Timeout: m_currentBlockCount=[{0}] m_totalBlockCount=[{1}]", m_currentBlockCount, m_totalBlockCount));

                        // Update UI
                        updateUI(uiData);

                        // Return result
                        return isOadComplete();
                    }
                }

                // Block StopWatch
                blockSw.Stop();
                Debug.WriteLine(String.Format("PerformOAD:Block Time elapsed: {0}", blockSw.Elapsed));

                long blockSwMs = blockSw.ElapsedMilliseconds;
                blockSw.Restart();

                // Fill up queue until maximum is reached
                if (blockTimes.Count >= maxBlockTimeQueue)
                {
                    blockTimes.Dequeue();
                }
                blockTimes.Enqueue(blockSwMs);

                // Assign average block time based on size of queue
                if (blockTimes.Count == maxBlockTimeQueue)
                {
                    // Set average block time based on a full queue
                    averageBlockTime = (long)Math.Round(blockTimes.Average(), 0);
                }
                else
                {
                    // When filling queue for first time use an estimated buffer, otherwise keep last calcualted block time
                    if (averageBlockTime == 0)
                    {
                        averageBlockTime = isFastMode ? ESTIMATED_AVG_BLOCK_TIME_MS_FAST_MODE : ESTIMATED_AVG_BLOCK_TIME_MS_NORMAL_MODE;
                    }
                }

                // Calculate estimated time
                estRemainingTime = TimeSpan.FromMilliseconds(((m_totalBlockCount - m_currentBlockCount) * averageBlockTime) / (isFastMode ? oadBlocksPerReqFastMode : 1));

#if DEBUG_DIAGNOSTICS
                // Request StopWatch
                Stopwatch reqSw = new Stopwatch();
                reqSw.Start();
#endif
                // Send block(s)
                if (isFastMode)
                {
                    // Send oadBlocksPerReqFastMode non-acknlowdged writes at a time
                    int blockCount = 0;
                    while (m_currentBlockCount < m_totalBlockCount && blockCount != oadBlocksPerReqFastMode)
                    {
                        connectedDevice.Jerome_SendNextBlock(false);
                        m_currentBlockCount++;
                        blockCount++;
                    }
                    // Wait for at ~1ms
                    SpinWait.SpinUntil(() => false, 1);
                }
                else
                {
                    // Send a single acknowledge block
                    if (BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT == connectedDevice.SyncSetAttributeFromCharHandle(m_crcBlockValHnd, blockTransferData[m_currentBlockCount++], BLE_AdvFunctions.PROCESSING_TIMEOUT_SHORT_MS))
                    {
                        // Update UI
                        updateUI(uiData);

                        // Return result
                        return isOadComplete();
                    }
                }

#if DEBUG_DIAGNOSTICS
                // Request StopWatch
                reqSw.Stop();
                Debug.WriteLine(String.Format("PerformOAD:Request Time elapsed: {0}", reqSw.Elapsed));
#endif
                // Update UI
                updateUI(uiData);
            }

            // Ensure to disable non-acknowledged writes
            if (isFastMode)
            {
                connectedDevice.SetExtendedWriteOffsetMode(true);
            }

            Debug.WriteLine(String.Format("PostLoop: m_currentBlockCount=[{0}] m_totalBlockCount=[{1}]", m_currentBlockCount, m_totalBlockCount));

            // Update UI
            updateUI(uiData);

            // Return result
            return isOadComplete();
        }

        private void updateUI(IProgress<OadService.UiData> uiData)
        {
            // Package data to be updated
            OadService.UiData ud = new OadService.UiData();            
            ud.StatusPercent = Math.Min((int)(((double)(m_currentBlockCount) / (double)(m_totalBlockCount)) * 100), 100);
            ud.EstTimeRemaining = estRemainingTime;
            ud.CurrentBytes = m_currentBlockCount * OAD_BLOCK_SIZE;
            ud.TotalBytes = m_totalBlockCount * OAD_BLOCK_SIZE;

            // Report data to UI
            uiData.Report(ud);
        }

        private bool isOadComplete()
        {
            Debug.WriteLine(String.Format("isOadComplete: m_currentBlockCount=[{0}] m_totalBlockCount=[{1}]", m_currentBlockCount, m_totalBlockCount));            

            // Once timer has expired check if the current block number is equal to or greater than the number of blocks in which case OAD is successful; otherwise OAD has failed
            if ((m_currentBlockCount + 1) >= m_totalBlockCount)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
