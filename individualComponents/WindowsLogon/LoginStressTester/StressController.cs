using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;


namespace LoginStressTester
{
    class StressController
    {
        User[] users;

        //Auto reset event to control how many more tests to do, as well as which users are being tested
        AutoResetEvent are = new AutoResetEvent(false);

        //Auto Reset event for when all the tests are done
        AutoResetEvent testDone = new AutoResetEvent(false);

        int requestsLeft;
        int threadsRunning;

        //RND for chosing which user to request from next
        Random rnd = new Random((int)DateTime.Now.Ticks);
        public string ServerAddress;
        List<DateTime> completeTimes;

        int hotp, totp;

        List<User> tempUsers = new List<User>();

        public void setServerAddress(string address)
        {
            ServerAddress = address;
        }


        public void addTempTokenUser(string username, string tempToken)
        {
            User u = new User(username, tempToken);
            tempUsers.Add(u);
        }

        public void finalizeUsers()
        {
            users = tempUsers.ToArray();
            tempUsers = new List<User>();
        }


        public List<DateTime> RunStressTest(int threadsToRun, int requestsToRun, int hotp, int totp)
        {
            this.hotp = hotp;
            this.totp = totp;

            if(users.Length < threadsToRun)
            {
                threadsToRun = users.Length;
            }

            requestsLeft = requestsToRun;
            threadsRunning = threadsToRun;

            are.Set();
            testDone.Reset();
            completeTimes = new List<DateTime>();
            //create all the threads
            for (int i = 0; i < threadsRunning; ++i)
            {
                Thread T = new Thread(new ThreadStart(() =>
                    {
                        threadCode();
                    }));

                T.Start();
            }

            //wait until all the threads are done
            testDone.WaitOne();

            return completeTimes;
        }

        //TODO : Not yet done, will eventually create a user, given them a token, figure out the token's seed and use that to generate OTPs
        public void createUsers(string userPrefix, int numberToCreate)
        {
            for (int i = 0; i < numberToCreate; ++i)
            {
                string tokenRes = MiToken.API.CreateSoftToken();
                //string bindToken = MiToken.API.Assign(userPrefix + i.ToString(), "");

            }
        }

        //Main code for the threads
        void threadCode()
        {
            bool running = true;
            while (running)
            {
                int userID;

                //start of exclusive threaded block
                {
                    are.WaitOne();
                    completeTimes.Add(DateTime.Now);
                    if (requestsLeft > 0)
                    {
                        requestsLeft--;
                    }
                    else
                    {
                        running = false;
                        threadsRunning--;
                        if (threadsRunning == 0)
                        {
                            testDone.Set();
                        }
                        are.Set();
                        break;
                    }
                    userID = rnd.Next(0, users.Length);
                    //find a user we aren't already testing
                    while (users[userID].testing == true)
                    {
                        //the user selected is being tested, try another one
                        userID = rnd.Next(0, users.Length);
                    }

                    users[userID].testing = true;
                    are.Set();
                } //end of exclusive threaded block
                

                //create OTP and send off the request. We don't actually care about the result
                string result = users[userID].doRequest(ServerAddress, hotp, totp);
                users[userID].testing = false;
            }
        }
    }
}
