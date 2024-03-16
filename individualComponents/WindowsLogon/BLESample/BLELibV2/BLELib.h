#pragma once
#ifndef _BLELIB_H_
#define _BLELIB_H_


#ifdef __cplusplus
extern "C" {
#endif

/*

struct DeviceInfo
{
	unsigned char MacAddress[6];
	char RSSI;
	unsigned int PollLastSeen;
};

int BLE_Initialize(const char* COMPort, unsigned char authAddr[6], unsigned char minRSSI, void (*callbackFound) ());
int BLE_InitializeV2(const char* COMPort, unsigned char* authAddresses, int authAddressesByteSize, unsigned char minRSSI, void (*callbackFound) ());
int BLE_Poll();
int BLE_Finalize();

int BLE_StartConfigurationPoll();
int BLE_GetCurrentPollID();
int BLE_GetDeviceFoundCount();
int BLE_GetDeviceInfo(int deviceID, struct DeviceInfo * deviceInfo);
int BLE_StopConfigurationPoll();

int BLE_StartASyncPoll();
int BLE_StopASyncPoll();
int BLE_SetPollID(int newID);

int BLE_SetAveragingMode(int newMode);
int BLE_SetIIR(int IIRPercent);

struct ConnectionData;


int BLE_ConnectToDevice(bd_addr address, void (*connCallback) (struct ConnectionData* myData, uint8 connectionHandle));
/*
int BLE_DiscoverServicesInitiate(uint8 connectionHandle, uint16 start, uint16 end, void (*serviceFound) (struct ConnectionData* myData, uint8 uidLength, const uint8* uidData, uint16 start, uint16 end), void (*serviceSearchFinished) (struct ConnectionData* myData));
int BLE_FindInformationOnService(uint8 connectionHandle, uint16 startAddress, uint16 endAddress, void (*serviceInformationFound) (struct ConnectionData* myData, uint16 characteristicsHandle, uint8 len, const uint8* data), void (*serviceInformationCompleted) (struct ConnectionData* myData));
int BLE_ReadByHandle(uint8 connectionHandle, uint16 characteristicHandle, void (*handleValue) (struct ConnectionData* myData, uint8 type, uint8 dataLen, const uint8* data));
int BLE_WriteAttribute(uint8 connectionHandle, uint16 attributeHandle, uint8 datalen, const uint8* data, void (*writeCompleted) (struct ConnectionData* myData));
*//*
int BLE_DiscoverServicesInitiate(uint8 connectionHandle, uint16 start, uint16 end);
int BLE_FindInformationOnService(uint8 connectionHandle, uint16 startAddress, uint16 endAddress);
int BLE_ReadByHandle(uint8 connectionHandle, uint16 characteristicHandle);
int BLE_WriteAttribute(uint8 connectionHandle, uint16 attributeHandle, uint8 datalen, const uint8* data);
int BLE_GetRSSI(uint8 connectionHandle);




int BLE_StartProfile(bd_addr address);
int BLE_ScanServicesInRange(int requestID, uint16 handle, uint16 maxLength);
int BLE_SetAttribute(int requestID, uint16 handle, uint8 length, const uint8* data);
int BLE_ReadAttribute(int requestID,  uint16 handle, uint8 maxLength, uint8* data);


//Basically does everything above but automatically.
int BLE_StartProfile(bd_addr address);


struct Callbacks
{
	//Data callbacks
	void (*serviceFound) (struct ConnectionData* myData, uint8 serviceHandleLength, const uint8* serviceHandleData, uint16 start, uint16 end);
	void (*serviceInformationFound) (struct ConnectionData* myData, uint16 characteristicsHandle, uint8 dataLen, const uint8* data);
	void (*handleValue) (struct ConnectionData* myData, uint16 attributeHandle, uint8 type, uint8 dataLen, const uint8* data);
	void (*RSSIValue) (struct ConnectionData* myData, int8 rssi);
	

	//Completed callbacks
	void (*serviceSearchFinished) (struct ConnectionData* myData);
	void (*serviceInformationFinished) (struct ConnectionData* myData);
	void (*writeAttributeFinished) (struct ConnectionData* myData);
};

struct ConnectionData
{
	struct ConnectionData* nextNode;
	int connectionID;
	struct Callbacks callbacks;
	void* extraData;
};

extern struct ConnectionData* rootConnectionData;
extern void (*setExtraData) (struct ConnectionData* connData);
extern void (*freeExtraData) (struct ConnectionData* connData);

struct ConnectionData* getConnectionData(int connectionID);
*/

#ifdef __cplusplus
}
#endif



#endif