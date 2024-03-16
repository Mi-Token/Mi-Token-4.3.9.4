using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using BLE_API;

namespace BLE_Configuration_Tool
{
    class DevicePollConfig
    {
        static readonly int[] PollSamples = new int[] { 20, 20, 100 };

        public bool lockOnlyOnDisconnect = false;

        static int MaxMissingSamples(int type)
        {
            return (PollSamples[type] * 30 / 100);
        }

        public byte[] deviceMacAddress;

        bool SameMacAddress(byte[] mac1, byte[] mac2)
        {
            bool result = false;
            if (mac1.Length == mac2.Length)
            {
                result = true;
                for(int i = 0 ; ((i < mac1.Length) && (result)) ; ++i)
                {
                    if(mac1[i] != mac2[i])
                    {
                        result = false;
                    }
                }
            }

            return result;
        }

        public SampleResults results = new SampleResults();

        public SByte lockRSSI
        {
            get
            {
                if (lockOnlyOnDisconnect)
                {
                    return SByte.MinValue;
                }
                else
                {
                    return results.lockRSSI;
                }
            }
        }

        public class SampleResults
        {
            private Results[] resultData = null;
            public SampleResults()
            {
                flushAllResults();
            }

            public int pollSleepTime = 100;

            public enum ResultType : int
            {
                None = -1,
                Unlock = 0,
                Lock = 1,
                Test = 2,
            };

            public int min0Polls = 0;
            public sbyte maxLockRSSI = sbyte.MaxValue;


            public void resetCurrentResults()
            {
                if (locationToWriteTo == ResultType.None)
                {
                    return; //nothing to do
                }
                resultData[(int)locationToWriteTo].restartResults(resultData[(int)locationToWriteTo].maxResults);
            }

            ResultType locationToWriteTo = ResultType.None;
            public class Results
            {
                public int maxResults
                {
                    get
                    {
                        if (resultData == null)
                        {
                            return 0;
                        }
                        return resultData.Length;
                    }
                }

                private sbyte[] resultData;
                private int resultWritePosition = -1;
                private int resultsWritten = 0;
                private bool trustResults = false;

                public sbyte NewResult
                {
                    set
                    {
                        if (resultWritePosition != -1)
                        {
                            resultData[resultWritePosition] = value;
                            resultWritePosition++;
                            if (resultWritePosition >= resultData.Length)
                            {
                                resultWritePosition = -1;
                            }
                            resultsWritten++;
                            trustResults = false;
                        }
                    }
                }


                private sbyte _averageRSSI;
                private sbyte _SD;
                private sbyte _Max;
                private sbyte _Min;
                private int _max0Poll;
                private int _missingSamples;

                public int progressPercent
                {
                    get
                    {
                        if (resultWritePosition == -1)
                        {
                            return 0;
                        }
                        if ((resultData == null) || (resultData.Length == 0))
                        {
                            return 0;
                        }

                        return resultsWritten * 100 / resultData.Length;
                    }
                }

                public void restartResults(int polls)
                {
                    resultData = new sbyte[polls];
                    resultWritePosition = 0;
                    resultsWritten = 0;

                }
                protected void updateResultsIfRequired()
                {
                    if (!trustResults)
                    {
                        Recalculate();
                    }
                }

                public sbyte averageRSSI { get { updateResultsIfRequired(); return _averageRSSI; } }
                public sbyte SD { get { updateResultsIfRequired(); return _SD; } }
                public sbyte Max { get { updateResultsIfRequired(); return _Max; } }
                public sbyte Min { get { updateResultsIfRequired(); return _Min; } }
                public int Max0Poll { get { updateResultsIfRequired(); return _max0Poll; } }
                public int missingSamples { get { updateResultsIfRequired(); return _missingSamples; } }

                public bool FinishedPolling { get { return resultWritePosition == -1; } }
                public bool HasResults { get { return (resultsWritten > 0); } }

                protected void CheckMinMax(sbyte value, ref sbyte min, ref sbyte max)
                {
                    if(value > max)
                    {
                        max = value;
                    }
                    if(value < min)
                    {
                        min = value;
                    }
                }

                public void Recalculate()
                {
                    int total = 0;
                    int count = 0;
                    int zeroCount = 0;
                    _max0Poll = 0;
                    _Max = sbyte.MinValue;
                    _Min = sbyte.MaxValue;
                    _missingSamples = 0;
                    bool foundSample = false;
                    //get average
                    if (resultsWritten > 0)
                    {
                        for (int i = 0; i < resultsWritten; ++i)
                        {
                            total += resultData[i];
                            if (resultData[i] != 0)
                            {
                                count++;
                                if (foundSample && (zeroCount > _max0Poll))
                                {
                                    _max0Poll = zeroCount;
                                }
                                if (foundSample)
                                {
                                    _missingSamples += zeroCount;
                                }

                                zeroCount = 0;

                                CheckMinMax(resultData[i], ref _Min, ref _Max);

                                foundSample = true;
                            }
                            else
                            {
                                zeroCount++;
                            }
                        }

                        if (zeroCount > _max0Poll)
                        {
                            _max0Poll = zeroCount;
                        }

                        if (count == 0)
                        {
                            _averageRSSI = 0;
                            _SD = 0;
                        }
                        else
                        {
                            _averageRSSI = (sbyte)(total / count);
                        
                            int offset = 0;
                            int offsetSquared = 0;
                            for (int i = 0; i < resultsWritten; ++i)
                            {
                                if (resultData[i] != 0)
                                {
                                    offset = (resultData[i] - _averageRSSI);
                                    offsetSquared += (offset * offset);
                                }
                            }

                            _SD = (sbyte)Math.Sqrt(offsetSquared / count);

                        }
                    }

                    trustResults = true;
                }
            }

            public void flushAllResults()
            {
                resultData = new Results[3];
                resultData[0] = new Results();
                resultData[1] = new Results();
                resultData[2] = new Results();
            }

            public void stopGettingResults()
            {
                locationToWriteTo = ResultType.None;
            }
            
            public void setResults(ResultType type)
            {
                locationToWriteTo = type;
                if (type != ResultType.None)
                {
                    resultData[(int)type].restartResults(PollSamples[(int)type]);
                }
            }

            public sbyte setNextResult
            {
                set
                {
                    if (locationToWriteTo == ResultType.None)
                    {
                        return;
                    }
                    else
                    {
                        resultData[(int)locationToWriteTo].NewResult = value;
                        if (resultData[(int)locationToWriteTo].FinishedPolling)
                        {
                            locationToWriteTo = ResultType.None;
                        }
                    }
                }
            }
            public int progressPercent
            {
                get
                {
                    if (locationToWriteTo == ResultType.None)
                    {
                        return 0;
                    }
                    else
                    {
                        return resultData[(int)locationToWriteTo].progressPercent;
                    }
                }
            }

            public sbyte unlockRSSI
            {
                get
                {
                    Results rd = resultData[(int)ResultType.Unlock];
                    return (sbyte)(rd.averageRSSI - rd.SD);
                }
            }

            public sbyte lockRSSI
            {
                get
                {
                    
                    sbyte max = unlockRSSI;
                    Results rd = resultData[(int)ResultType.Lock];
                    return Math.Min(max, Math.Min(rd.Max, maxLockRSSI));
                }
            }

            public int max0Polls
            {
                get
                {
                    Results ul = resultData[(int)ResultType.Unlock];
                    int fromUnlock = ul.HasResults ? ul.Max0Poll : 0;

                    //Max of fromUnlock, 1000 / pollSpeed and 1. (With an extra check so that if PollSpeed is 0 there is no divBy0 errors
                    fromUnlock = Math.Max(fromUnlock, Math.Max(1000 / Math.Max(pollSleepTime, 1), 1));

                    Results ld = resultData[(int)ResultType.Lock];
                    int fromLock = ld.HasResults ? ul.Max0Poll : 0;

                    int fromExtra = min0Polls;

                    return Math.Min(30, Math.Max(Math.Max(fromUnlock, fromLock), fromExtra));
                }
            }

            public int maxMissingSamples
            {
                get
                {
                    Results ul = resultData[(int)ResultType.Unlock];
                    int fromUnlock = ul.HasResults ? ul.missingSamples : 0;

                    return fromUnlock;
                }
            }

            public ResultType currentlyTesting { get { return locationToWriteTo; } }

            public bool RecommendSlowerSampleRate(bool includeTestResults = false)
            {
                Results ul = resultData[(int)ResultType.Unlock];
                if((ul.HasResults) && (ul.SD > 3))
                {
                    return true;
                }

                if(ul.Max0Poll > MaxMissingSamples((int)ResultType.Unlock))
                {
                    return true;
                }

                Results ld = resultData[(int)ResultType.Lock];
                if ((ld.HasResults) && (ld.SD > 3))
                {
                    return true;
                }

                if(ld.Max0Poll > MaxMissingSamples((int)ResultType.Lock))
                {
                    return true;
                }

                return false;
            }
        }


        BLE.V2_2.Device _conn;

        public DevicePollConfig(BLE.V2_2.Device conn)
        {
            _conn = conn;
        }
    }
}
