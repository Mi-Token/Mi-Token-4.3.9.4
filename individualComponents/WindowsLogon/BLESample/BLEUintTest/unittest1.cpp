#include "stdafx.h"
#include "CppUnitTest.h"

#include "BLE_API.h"
#include "UnitTest_BLEIO.h"
#include "BLE_IO.h"
#include "UnitTestDevice.h"
#include "UnitTest_MetaData.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define COMPort "COM_UNIT_TEST"
#define NamedPipePort "\\\\.\\pipe\\UNIT_TEST_PIPE"

namespace BLEUintTest
{		

	TEST_CLASS(BLE_POLL)
	{
	public:
		TEST_METHOD(SimplePoll)
		{
			//setup
			UnitTest_IO* myIO = new UnitTest_IO();
			myIO->myState.enableAsyncReads = true;
			
			UnitTestDevice UTD;

			UTD.giveRandomMac();
			UTD.setMetaData(NULL, 0);

			//Override interface
			Set_IO_Interface(myIO);


			//Test
			int ret = BLE_InitializeEx(COMPort, NamedPipePort, true);
			Assert::AreEqual(0, ret, L"BLE_InitializeEx didn't return 0");

			UTD.sendPollOff(myIO, 30);

			Sleep(100); //give it some time as the read is async

			int devCount = BLE_GetDeviceFoundCount();
			Assert::AreEqual(1, devCount, L"Did not find the expected 1 device");

			DeviceInfo devInfo;
			BLE_GetDeviceInfo(0, &devInfo);

			Assert::AreEqual(true, UTD.sameMac(reinterpret_cast<mac_address*>(&devInfo.address)), L"Addresses are not the same");

			//Uninit
			BLE_Finalize();
			Set_IO_Interface(NULL);
			myIO = NULL;
		};

		TEST_METHOD(MultiPoll)
		{
			//setup
			UnitTest_IO* myIO = new UnitTest_IO();
			myIO->myState.enableAsyncReads = true;

			UnitTestDevice UTD1, UTD2;
			UTD1.giveRandomMac();
			UTD2.giveRandomMac();
			UTD1.setMetaData(NULL, 0);
			UTD2.setMetaData(NULL, 0);

			//override interface
			Set_IO_Interface(myIO);

			//Test
			int ret = BLE_InitializeEx(COMPort, NamedPipePort, true);
			Assert::AreEqual(0, ret, L"BLE_InitializeEx didn't return 0");

			UTD1.sendPollOff(myIO, 10);
			UTD2.sendPollOff(myIO, 20);

			Sleep(100);
			int devCount = BLE_GetDeviceFoundCount();
			Assert::AreEqual(2, devCount, L"Did not find the expected 2 devices");

			DeviceInfo devInfo;
			

			for(int i = 0 ; i < 2 ; ++i)
			{
				BLE_GetDeviceInfo(i, &devInfo);
				Assert::AreEqual(true, 
				(UTD1.sameMac(reinterpret_cast<mac_address*>(&devInfo.address)) ||
				UTD2.sameMac(reinterpret_cast<mac_address*>(&devInfo.address))),  L"MAC Address not found in list");
			}

			//Uninit
			BLE_Finalize();
			Set_IO_Interface(NULL);
			myIO = NULL;
		}

		TEST_METHOD(AddressFilter)
		{
			//setup
			UnitTest_IO* myIO = new UnitTest_IO();
			myIO->myState.enableAsyncReads = true;

			UnitTestDevice UTD1, UTD2;

			//generate a random mac for UTD1 (we need to store it as it is the MAC we will filter on
			mac_address filterMac;
			for(int i = 0 ; i < 6; ++i) 
			{
				filterMac.addr[i] = rand() & 0xFF; 
			}


			//Set UTD1s mac, give UTD2 a random mac
			UTD1.setMac(filterMac);
			UTD2.giveRandomMac();

			//on the very low chance that UTD2 gets the same mac, keep generating new random macs for it until they are different
			while(UTD2.sameMac(&filterMac)) 
			{
				UTD2.giveRandomMac(); 
			}

			//No meta data, we are just filtering on mac
			UTD1.setMetaData(NULL, 0);
			UTD2.setMetaData(NULL, 0);

			//override IO interface
			Set_IO_Interface(myIO);

			//Test
			int ret = BLE_InitializeEx(COMPort, NamedPipePort, true);
			Assert::AreEqual(0, ret, L"BLE_InitializeEx did not return 0");

			UTD1.sendPollOff(myIO, 30);
			UTD2.sendPollOff(myIO, 30);

			Sleep(100);

			//check that 2 devices were found
			int devCount = BLE_GetDeviceFoundCount();
			Assert::AreEqual(2, devCount, L"Did not find the expected 2 devices");

			//now lets make an address filter
			uint8 addfilter[sizeof(mac_address)];
			memcpy(&(addfilter[0]), &(filterMac.addr), sizeof(mac_address));

			//Create the FSO using no meta-data filtering, and 1 address filter
			FilteredSearchObject FSO;
			FSO = BLE_StartFilteredSearchEx(0, NULL, 1, addfilter);

			//Check how many filtered devices we have found (scan through the list until we no longer find any
			int filterDevCount = 0;
			DeviceInfo devInfo;
			while(BLE_ContinueFilteredSearch(FSO, &devInfo) == 0)
			{
				filterDevCount++;
			}

			Assert::AreEqual(1, filterDevCount, L"Filter didn't find the expected number of items");


			//restart the search and do it again - test RestartFilteredSearch works as expected
			BLE_RestartFilteredSearch(FSO);
			filterDevCount = 0;
			while(BLE_ContinueFilteredSearch(FSO, &devInfo) == 0)
			{
				filterDevCount++;
			}
			Assert::AreEqual(1, filterDevCount, L"Filter didn't find the expected number of items after restarting the filtered search");

			//restart it again, and check the device is the correct one
			BLE_RestartFilteredSearch(FSO);
			ret = BLE_ContinueFilteredSearch(FSO, &devInfo);
			Assert::AreEqual(0, ret, L"BLE_ContinueFilteredSearch returned incorrect value when getting devInfo");

			Assert::AreEqual(true, UTD1.sameMac(reinterpret_cast<mac_address*>(&devInfo.address)), L"Different MAC Addresses found");

			//Free up FSO
			BLE_FinishFilteredSearch(FSO);


			//Uninit
			BLE_Finalize();
			Set_IO_Interface(NULL);
			myIO = NULL;
		}

		TEST_METHOD(MetaDataFilter)
		{
			//setup
			UnitTest_IO* myIO = new UnitTest_IO();
			myIO->myState.enableAsyncReads = true;

			UnitTestDevice UTD1, UTD2;

			mac_address filterMac;
			for(int i = 0 ; i < sizeof(mac_address) ; ++i)
			{
				filterMac.addr[i] = rand() & 0xFF;
			}

			UTD1.setMac(filterMac);
			UTD2.giveRandomMac();

			while(UTD2.sameMac(&filterMac))
			{
				UTD2.giveRandomMac();
			}

			UnitTest_MetaData MD1, MD2;
			static uint8 metaData1[3];
			static uint8 metaData2[9];
			for(int i = 0 ; i < sizeof(metaData1) ; ++i)
			{
				metaData1[i] = rand() & 0xFF;
			}
			for(int i = 0 ; i < sizeof(metaData2) ; ++i)
			{
				metaData2[i] = rand() & 0xFF;
			}
			
			const int meta1Flag = 4;
			const int meta2Flag = 7;
			MD1.addNode(meta1Flag, sizeof(metaData1), metaData1);
			MD2.addNode(meta2Flag, sizeof(metaData2), metaData2);

			int md1len, md2len;
			MD1.packAndReturn(md1len);
			MD2.packAndReturn(md2len);

			UTD1.setMetaData(MD1.packAndReturn(md1len), md1len);
			UTD2.setMetaData(MD2.packAndReturn(md2len), md2len);

			//Set IO interface
			Set_IO_Interface(myIO);

			//Test
			int ret = BLE_InitializeEx(COMPort, NamedPipePort, true);
			Assert::AreEqual(0, ret, L"BLE_InitializeEx did not return 0");

			UTD1.sendPollOff(myIO, 30);
			UTD2.sendPollOff(myIO, 30);

			Sleep(100);

			//check that 2 devices were found
			int devCount = BLE_GetDeviceFoundCount();
			Assert::AreEqual(2, devCount, L"Did not find the expected 2 devices");

			//now for the meta data filter
			DeviceData dd1;
			dd1.flag = meta1Flag;
			dd1.length = sizeof(metaData1);
			dd1.data = metaData1;

			DeviceData dd2;
			dd2.flag = meta2Flag;
			dd2.length = sizeof(metaData2);
			dd2.data = metaData2;

			FilteredSearchObject FSO1, FSO2;
			FSO1 = BLE_StartFilteredSearchEx(1, &dd1, 0, NULL);
			FSO2 = BLE_StartFilteredSearchEx(1, &dd2, 0, NULL);
			Assert::AreNotEqual(static_cast<void*>(NULL), FSO1, L"FSO1 is null");
			Assert::AreNotEqual(static_cast<void*>(NULL), FSO2, L"FSO2 is null");

			int filterCount = 0;
			DeviceInfo devInfo;
			while(BLE_ContinueFilteredSearch(FSO1, &devInfo) == 0)
			{
				filterCount++;
			}

			Assert::AreEqual(1, filterCount, L"FSO1 found an unexpected number of items");
			filterCount = 0;
			while(BLE_ContinueFilteredSearch(FSO2, &devInfo) == 0)
			{
				filterCount++;
			}
			Assert::AreEqual(1, filterCount, L"FSO2 found an unexpected number of items");

			//ensure the items found are correct
			BLE_RestartFilteredSearch(FSO1);
			BLE_ContinueFilteredSearch(FSO1, &devInfo);
			Assert::AreEqual(true, UTD1.sameMac(reinterpret_cast<mac_address*>(&devInfo.address)), L"FSO1 didn't find UTD1");

			BLE_RestartFilteredSearch(FSO2);
			BLE_ContinueFilteredSearch(FSO2, &devInfo);
			Assert::AreEqual(true, UTD2.sameMac(reinterpret_cast<mac_address*>(&devInfo.address)), L"FSO2 didn't find UTD2");

			//Free FSO
			BLE_FinishFilteredSearch(FSO1);
			BLE_FinishFilteredSearch(FSO2);

			//Uninit
			BLE_Finalize();
			Set_IO_Interface(NULL);
			myIO = NULL;

			//do not worry about the metadata, that is freed in UnitTest_MetaData deconstructor. So don't worry about it.




		}
	};


	TEST_CLASS(BLE_IO)
	{
	public:
		TEST_METHOD(NoCOMAccess)
		{
			//Check return code when we cannot access the COM Port

			//Setup
			UnitTest_IO* myIO = new UnitTest_IO();
			myIO->myState.emulateCOMPortOpenFailure = true;
			myIO->myState.useNamedPipe = false;

			//Override the normal BLE_IO interface with the UnitTest interface we just made
			Set_IO_Interface(myIO);

			//Test
			int ret = BLE_InitializeEx(COMPort, NamedPipePort, true);

			Assert::AreEqual(0, ret, L"Unknown return code from BLE_InitializeEX, Should always be 0");

			ret = BLE_ConnectedToCOM();
			Assert::AreEqual(0, ret, L"API Thinks it is connected to the COM when emulateCOMPortFailture is true");

			BLE_Finalize();
			
			//Fix interface overriding - note: Set_IO_Interfacce will delete myIO
			Set_IO_Interface(NULL);
			myIO = NULL; //DO NOT Delete, it is already done with Set_IO_Interface
		};
	

		TEST_METHOD(COMAccess)
		{
			//Check return code when we can access COM Port

			//Setup
			UnitTest_IO* myIO = new UnitTest_IO();
			myIO->myState.emulateCOMPortOpenFailure = false;
			myIO->myState.useNamedPipe = false;
			myIO->myState.emulatePipeConnection = false;

			//Override normal BLE_IO
			Set_IO_Interface(myIO);

			//Test
			int ret = BLE_InitializeEx(COMPort, NamedPipePort, true);

			Assert::AreEqual(0, ret, L"Unknown return code from BLE_InitializeEX, Should always be 0");

			ret = BLE_ConnectedToCOM();
			Assert::AreEqual(1, ret, L"API Thinks it is not connected to the COM when emulateCOMPortFailture is false");

			BLE_Finalize();

			//fix interface overriding
			Set_IO_Interface(NULL);
			myIO = NULL;
		};

		TEST_METHOD(COMOverrideOnClient)
		{
			//Check that the COM Port will be used if it fails to connect to the named pipe server
			
			//Setup
			UnitTest_IO* myIO = new UnitTest_IO();
			myIO->myState.emulateCOMPortOpenFailure = false;
			myIO->myState.useNamedPipe = false;
			myIO->myState.emulatePipeConnection = false;

			//Set override
			Set_IO_Interface(myIO);

			//Test
			int ret = BLE_InitializeEx(COMPort, NamedPipePort, false);
			Assert::AreEqual(0, ret, L"Unknown return code from BLE_InitializeEX, Should always be 0");

			ret = BLE_ConnectedToCOM();
			Assert::AreEqual(1, ret, L"API Thinks it is not connected to the COM when emulateCOMPortFailture is false and emulatePipeConnection is false");
			
			BLE_Finalize();

			//fix interface overriding
			Set_IO_Interface(NULL);
			myIO = NULL;
		}
	};

	TEST_CLASS(FilteredSearchTester)
	{
	public:
		

		TEST_METHOD(ByteStreamConversionIdenticle)
		{
			//Ensure that the byte stream from BLE_ConvertFilteredSearchToByteStream and BLE_ConvertByteStreamToFilteredSearch are equal

			//Setup a filtered search object using hardset filters
			DeviceData filters[2];
			for(int fid = 0 ; fid < _countof(filters) ; ++fid)
			{
				//use random data for flag - it shouldn't matter
				filters[fid].flag = rand() & 0xFF;
				filters[fid].length = (rand() & 0x0F) + 1; //random data length from 1 - 16
				filters[fid].data = new uint8[filters[fid].length];

				for(int fildata = 0 ; fildata < filters[fid].length ; ++fildata)
				{
					filters[fid].data[fildata] = rand() & 0xFF; //random data
				}
			}

			int addressCount = rand() & 0xF; //upto 16 random address
			int addressLen = addressCount * 6;
			uint8* addressData = NULL;
			if(addressCount > 0)
			{
				addressData = new uint8[addressCount * 6];
				for(int aid = 0 ; aid < (addressCount * 6) ; ++aid)
				{
					addressData[aid] = rand() & 0xFF; //random address data
				}
			}

			FilteredSearchObject fso = BLE_StartFilteredSearchEx(_countof(filters), filters, addressCount, addressData);

			//Free allocated data from above this line
			if(addressData)
			{
				delete[] addressData;
			}
			for(int fid = 0 ; fid < _countof(filters); ++fid)
			{
				delete[] filters[fid].data;
			}

			//All data is free


			//make sure the FSO was created
			Assert::AreNotEqual(static_cast<void*>(NULL), fso, L"Filtered Search Object is NULL");
			
			//get the byteStream from the fso
			uint8* byteStream = NULL;
			int byteLength = 0;
			int tempByteLength = 0;
			int ret = BLE_ConvertFilteredSearchToByteStream(fso, byteStream, byteLength);

			//make sure ret is MORE_DATA
			Assert::AreEqual(ret, static_cast<int>(BLE_API_MORE_DATA), L"ConvertedFilteredSearchToByteStream did not return BLE_API_MORE_DATA");
			Assert::AreNotEqual(0, byteLength, L"ConvertFilteredSearchToByteStream set byteLength to 0.");

			//ensure that ConvertFilteredSearchToByteStream will still return BLE_API_MORE_DATA if we again call it
			tempByteLength = byteLength; //store byteLength for moment
			ret = BLE_ConvertFilteredSearchToByteStream(fso, byteStream, byteLength);

			Assert::AreEqual(static_cast<int>(BLE_API_MORE_DATA), ret, L"ConvertedFilteredSearchToByteStream did not return BLE_API_MORE_DATA with Set ByteLength");
			Assert::AreNotEqual(0, byteLength, L"ConvertFilteredSearchToByteStream set byteLength to 0 with set bytelength.");
			Assert::AreEqual(tempByteLength, byteLength, L"ConvertFilteredSearchToByteStream returned a different length on byteLength call");

			//Do actual call now
			byteStream = new uint8[byteLength];
			ret = BLE_ConvertFilteredSearchToByteStream(fso, byteStream, byteLength);
			Assert::AreEqual(static_cast<int>(BLE_API_SUCCESS), ret, L"ConvertFilteredSearchToByteStream did not return BLE_API_SUCCESS with Set ByteStream and ByteLength");
			Assert::AreNotEqual(0, byteLength, L"ConvertFilteredSearchToByteStream set byteLength to 0 with set ByteStream");
			Assert::AreEqual(tempByteLength, byteLength, L"ConvertFilteredSearchToByteStream returned a different length when ByteStream set");

			//Now create a new FSO object using the bytestream
			FilteredSearchObject fso2 = BLE_CovnertByteStreamToFilteredSearch(byteStream, byteLength);
			Assert::AreNotEqual(static_cast<void*>(NULL), fso2, L"ConvertByteStreamToFilteredSearch returned NULL");

			//Again read the bytestream data, this time off FSO2
			uint8* byteStream2 = NULL;
			int byteLength2 = 0;
			ret = BLE_ConvertFilteredSearchToByteStream(fso2, byteStream2, byteLength2);
			Assert::AreEqual(static_cast<int>(BLE_API_MORE_DATA), ret, L"ConvertFilteredSearchToByteStream did not return BLE_API_MORE_DATA on conversion of FSO2");
			Assert::AreNotEqual(0, byteLength2, L"BLE_ConvertFilteredSearchToByteStream returned length of 0 on conversion of FSO2");
			Assert::AreEqual(byteLength, byteLength2, L"BLE_ConvertFilteredSearchToByteStream returned a different length for FSO2 in comparison to FSO");

			byteStream2 = new uint8[byteLength2];
			ret = BLE_ConvertFilteredSearchToByteStream(fso2, byteStream2, byteLength2);
			Assert::AreEqual(static_cast<int>(BLE_API_SUCCESS), ret, L"ConvertFilteredSearchToByteStream did not return BLE_API_SUCCESS on conversion of FSO2");
			Assert::AreEqual(byteLength, byteLength2, L"BLE_ConvertFilteredSearchToByteStream returned a different length for FSO2 in comparison to FSO when loading ByteStream2");
			
			Assert::AreEqual(0, memcmp(byteStream, byteStream2, byteLength), L"FSO and FSO2 had different bytestreams");
			
			BLE_FinishFilteredSearch(fso);
			BLE_FinishFilteredSearch(fso2);
			fso = NULL;
			fso2 = NULL;

			free(byteStream);
			free(byteStream2);
		}

	};
}