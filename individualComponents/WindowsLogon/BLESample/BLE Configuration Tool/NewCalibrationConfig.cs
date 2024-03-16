using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BLE_Configuration_Tool
{
    class NewCalibrationConfig
    {

        int _pollsRead = 0;
        int _maxPollsToRead = 100;
        int _pollReadTotal = 0;
        int _pollReadRaw = 0;
        public void AddSignalPoll(int newPollPercent, int newPollRaw)
        {
            _pollsRead++;
            _pollReadTotal += newPollPercent;
            _pollReadRaw += newPollRaw;
        }

        public int Progress
        {
            get
            {
                int i =(_pollsRead * 100 / _maxPollsToRead);
                if(i < 0) { i = 0 ; }
                if(i > 100) { i = 100; }

                return i;
            }
        }

        public int PollReadAverage
        {
            get
            {
                if (_pollsRead == 0)
                {
                    return 0;
                }
                else
                {
                    return (_pollReadTotal / _pollsRead);
                }
            }
        }

        public int PollReadRawAverage
        {
            get
            {
                if (_pollsRead == 0)
                {
                    return 0;
                }
                else
                {
                    return (_pollReadRaw / _pollsRead);
                }
            }
        }

        private const int A = -58;//-70;
        private const int n = 2;

        public double LoginDistance
        {
            get
            {
                return Math.Pow(10, (((double)PollReadRawAverage - A) / (-10 * n)));
            }
        }

        private const double dOffset = 5.0;

        public double LockDistance
        {
            get
            {
                return LoginDistance + dOffset;
            }
        }

        public int LockRSSICalc
        {
            get
            {
                return (int)RSSIFromDistance(LockDistance);
            }
        }

        private const int minRSSI = -89;
        private const int maxRSSI = -38;

        public int LockRSSIPercent
        {
            get
            {
                return RSSIPercentFromRSSI(LockRSSICalc);
            }
        }

        private const double maxSensitivityRange = 2;

        private int RSSIFromDistance(double distance)
        {
            return (int)Math.Round((-10 * n * Math.Log10(distance)) + A, 0);
        }

        private int RSSIPercentFromRSSI(double RSSI)
        {
            return (int)Math.Round((decimal)((100 * (RSSI - minRSSI)) / (maxRSSI - minRSSI)), 0);
        }

        public void getLockUnlockForSensitivity(double sensitivity, ref int unlockLevel, ref int lockLevel)
        {
            double modUnlockDist = LoginDistance + ((sensitivity) * maxSensitivityRange);
            double modUnlockRSSI = RSSIFromDistance(modUnlockDist);
            int modUnlockPercent = RSSIPercentFromRSSI(modUnlockRSSI);

            double modLockDist = LockDistance + ((-sensitivity) * maxSensitivityRange);
            double modLockRSSI = RSSIFromDistance(modLockDist);
            int modLockPercent = RSSIPercentFromRSSI(modLockRSSI);

            lockLevel = modLockPercent;
            unlockLevel = modUnlockPercent;
        }
    }
}
