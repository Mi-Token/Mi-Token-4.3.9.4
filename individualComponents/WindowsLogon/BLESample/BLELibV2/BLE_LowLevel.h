#pragma once

#ifndef _BLE_LOWLEVEL_H_
#define _BLE_LOWLEVEL_H_

#include "BLE_IO.h"
#include "BLE_API_Types.h"

class BLE_LowLevel
{
public:
	BLE_LowLevel();
	void SetIO(IBLE_IO* BLE_IO);

	void InitBLEDevice();
	void ResetSystem();
	BLE_API_RET ConnectToDevice(mac_address address);
	BLE_API_RET DiscoverServicesInitiate(uint8 connectionID, uint16 start, uint16 end);
	BLE_API_RET FindInformationOnService(uint8 connectionHandle, uint16 startAddress, uint16 endAddress);
	BLE_API_RET AttributeWrite(uint8 connectionID, uint16 characteristicHandle, uint8 length, const uint8* data);
	BLE_API_RET Disconnect(uint8 connectionHandle);
	BLE_API_RET RestartScanner();
	BLE_API_RET PollRSSI(uint8 connectionHandle);
	BLE_API_RET ExtendedAttributeWrite(uint8 connectionID, uint16 attributeHandle, uint16 offset, uint8 length, const uint8* data);
	BLE_API_RET ExecuteWrite(uint8 connectionID, uint8 commit);
	BLE_API_RET ReadAttribute(uint8 connectionID, uint16 handle);
	BLE_API_RET EndProcedure();
	BLE_API_RET LongReadAttribute(uint8 connectionID, uint16 handle);
	BLE_API_RET GapDiscover(uint8 mode);

	void ble_send_message(uint8 msgid, ...);

	
protected:
	IBLE_IO* _BLE_IO;
};



namespace _ble
{
	namespace _cmd
	{
		namespace _system
		{
			void _reset (BLE_LowLevel* pLowLevel, uint8 boot_in_dfu);
			void _hello (BLE_LowLevel* pLowLevel);
			namespace _address
			{
				void _get (BLE_LowLevel* pLowLevel);
			}
			namespace _reg
			{
				void _write (BLE_LowLevel* pLowLevel, uint16 address, uint8 value);
				void _read (BLE_LowLevel* pLowLevel, uint16 address);
			}
			namespace _get
			{
				void _counters (BLE_LowLevel* pLowLevel);
				void _connections (BLE_LowLevel* pLowLevel);
				void _info (BLE_LowLevel* pLowLevel);
			}
			namespace _read
			{
				void _memory (BLE_LowLevel* pLowLevel, uint32 address, uint8 length);
			}
			namespace _endpoint
			{
				void _tx (BLE_LowLevel* pLowLevel, uint8 endpoint, uint8 data_len, const uint8* data_data);
				void _rx (BLE_LowLevel* pLowLevel, uint8 endpoint, uint8 size);
				namespace _set
				{
					void _watermarks (BLE_LowLevel* pLowLevel, uint8 endpoint, uint8 rx, uint8 tx);
				}
			}
			namespace _whitelist
			{
				void _append (BLE_LowLevel* pLowLevel, mac_address* address, uint8 address_type);
				void _remove (BLE_LowLevel* pLowLevel, mac_address* address, uint8 address_type);
				void _clear (BLE_LowLevel* pLowLevel);
			}
		}
		namespace _flash
		{
			namespace _ps
			{
				void _defrag (BLE_LowLevel* pLowLevel);
				void _dump (BLE_LowLevel* pLowLevel);
				namespace _erase
				{
					void _all (BLE_LowLevel* pLowLevel);
				}
				void _save (BLE_LowLevel* pLowLevel, uint16 key, uint8 value_len, const uint8* value_data);
				void _load (BLE_LowLevel* pLowLevel, uint16 key);
			}
			namespace _erase
			{
				void _page (BLE_LowLevel* pLowLevel, uint8 page);
			}
			namespace _write
			{
				void _words (BLE_LowLevel* pLowLevel, uint16 address, uint8 words_len, const uint8* words_data);
			}
		}
		namespace _attributes
		{
			void _write (BLE_LowLevel* pLowLevel, uint16 handle, uint8 offset, uint8 value_len, const uint8* value_data);
			namespace _read
			{
				void _type (BLE_LowLevel* pLowLevel, uint16 handle);
			}
			namespace _user
			{
				namespace _read
				{
					void _response (BLE_LowLevel* pLowLevel, uint8 connection, uint8 att_error, uint8 value_len, const uint8* value_data);
				}
				namespace _write
				{
					void _response (BLE_LowLevel* pLowLevel, uint8 connection, uint8 att_error);
				}
			}
		}
		namespace _connection
		{
			void _disconnect (BLE_LowLevel* pLowLevel, uint8 connection);
			namespace _get
			{
				void _rssi (BLE_LowLevel* pLowLevel, uint8 connection);
				void _status (BLE_LowLevel* pLowLevel, uint8 connection);
			}
			void _update (BLE_LowLevel* pLowLevel, uint8 connection, uint16 interval_min, uint16 interval_max, uint16 latency, uint16 timeout);
			namespace _version
			{
				void _update (BLE_LowLevel* pLowLevel, uint8 connection);
			}
			namespace _channel
			{
				namespace _map
				{
					void _get (BLE_LowLevel* pLowLevel, uint8 connection);
					void _set (BLE_LowLevel* pLowLevel, uint8 connection, uint8 map_len, const uint8* map_data);
				}
			}
			namespace _features
			{
				void _get (BLE_LowLevel* pLowLevel, uint8 connection);
			}
			namespace _raw
			{
				void _tx (BLE_LowLevel* pLowLevel, uint8 connection, uint8 data_len, const uint8* data_data);
			}
		}
		namespace _attclient
		{
			namespace _find
			{
				namespace _by
				{
					namespace _type
					{
						void _value (BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end, uint16 uuid, uint8 value_len, const uint8* value_data);
					}
				}
				void _information (BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end);
			}
			namespace _read
			{
				namespace _by
				{
					namespace _group
					{
						void _type (BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end, uint8 uuid_len, const uint8* uuid_data);
					}
					void _type (BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end, uint8 uuid_len, const uint8* uuid_data);
					void _handle (BLE_LowLevel* pLowLevel, uint8 connection, uint16 chrhandle);
				}
				void _long (BLE_LowLevel* pLowLevel, uint8 connection, uint16 chrhandle);
				void _multiple (BLE_LowLevel* pLowLevel, uint8 connection, uint8 handles_len, const uint8* handles_data);
			}
			namespace _attribute
			{
				void _write (BLE_LowLevel* pLowLevel, uint8 connection, uint16 atthandle, uint8 data_len, const uint8* data_data);
			}
			namespace _write
			{
				void _command (BLE_LowLevel* pLowLevel, uint8 connection, uint16 atthandle, uint8 data_len, const uint8* data_data);
			}
			namespace _indicate
			{
				void _confirm (BLE_LowLevel* pLowLevel, uint8 connection);
			}
			namespace _prepare
			{
				void _write (BLE_LowLevel* pLowLevel, uint8 connection, uint16 atthandle, uint16 offset, uint8 data_len, const uint8* data_data);
			}
			namespace _execute
			{
				void _write (BLE_LowLevel* pLowLevel, uint8 connection, uint8 commit);
			}
		}
		namespace _sm
		{
			namespace _encrypt
			{
				void _start (BLE_LowLevel* pLowLevel, uint8 handle, uint8 bonding);
			}
			namespace _set
			{
				namespace _bondable
				{
					void _mode (BLE_LowLevel* pLowLevel, uint8 bondable);
				}
				void _parameters (BLE_LowLevel* pLowLevel, uint8 mitm, uint8 min_key_size, uint8 io_capabilities);
				namespace _oob
				{
					void _data (BLE_LowLevel* pLowLevel, uint8 oob_len, const uint8* oob_data);
				}
			}
			namespace _delete
			{
				void _bonding (BLE_LowLevel* pLowLevel, uint8 handle);
			}
			namespace _passkey
			{
				void _entry (BLE_LowLevel* pLowLevel, uint8 handle, uint32 passkey);
			}
			namespace _get
			{
				void _bonds (BLE_LowLevel* pLowLevel);
			}
		}
		namespace _gap
		{
			namespace _set
			{
				namespace _privacy
				{
					void _flags (BLE_LowLevel* pLowLevel, uint8 peripheral_privacy, uint8 central_privacy);
				}
				void _mode (BLE_LowLevel* pLowLevel, uint8 discover, uint8 connect);
				void _filtering (BLE_LowLevel* pLowLevel, uint8 scan_policy, uint8 adv_policy, uint8 scan_duplicate_filtering);
				namespace _scan
				{
					void _parameters (BLE_LowLevel* pLowLevel, uint16 scan_interval, uint16 scan_window, uint8 active);
				}
				namespace _adv
				{
					void _parameters (BLE_LowLevel* pLowLevel, uint16 adv_interval_min, uint16 adv_interval_max, uint8 adv_channels);
					void _data (BLE_LowLevel* pLowLevel, uint8 set_scanrsp, uint8 adv_data_len, const uint8* adv_data_data);
				}
				namespace _directed
				{
					namespace _connectable
					{
						void _mode (BLE_LowLevel* pLowLevel, mac_address* address, uint8 addr_type);
					}
				}
			}
			void _discover (BLE_LowLevel* pLowLevel, uint8 mode);
			namespace _connect
			{
				void _direct (BLE_LowLevel* pLowLevel, mac_address* address, uint8 addr_type, uint16 conn_interval_min, uint16 conn_interval_max, uint16 timeout, uint16 latency);
				void _selective (BLE_LowLevel* pLowLevel, uint16 conn_interval_min, uint16 conn_interval_max, uint16 timeout, uint16 latency);
			}
			namespace _end
			{
				void _procedure (BLE_LowLevel* pLowLevel);
			}
		}
		namespace _hardware
		{
			namespace _io
			{
				namespace _port
				{
					namespace _config
					{
						void _irq (BLE_LowLevel* pLowLevel, uint8 port, uint8 enable_bits, uint8 falling_edge);
						void _direction (BLE_LowLevel* pLowLevel, uint8 port, uint8 direction);
						void _function (BLE_LowLevel* pLowLevel, uint8 port, uint8 function);
						void _pull (BLE_LowLevel* pLowLevel, uint8 port, uint8 tristate_mask, uint8 pull_up);
					}
					void _write (BLE_LowLevel* pLowLevel, uint8 port, uint8 mask, uint8 data);
					void _read (BLE_LowLevel* pLowLevel, uint8 port, uint8 mask);
				}
			}
			namespace _set
			{
				namespace _soft
				{
					void _timer (BLE_LowLevel* pLowLevel, uint32 time, uint8 handle, uint8 single_shot);
				}
				void _txpower (BLE_LowLevel* pLowLevel, uint8 power);
			}
			namespace _adc
			{
				void _read (BLE_LowLevel* pLowLevel, uint8 input, uint8 decimation, uint8 reference_selection);
			}
			namespace _spi
			{
				void _config (BLE_LowLevel* pLowLevel, uint8 channel, uint8 polarity, uint8 phase, uint8 bit_order, uint8 baud_e, uint8 baud_m);
				void _transfer (BLE_LowLevel* pLowLevel, uint8 channel, uint8 data_len, const uint8* data_data);
			}
			namespace _i2c
			{
				void _read (BLE_LowLevel* pLowLevel, uint8 address, uint8 stop, uint8 length);
				void _write (BLE_LowLevel* pLowLevel, uint8 address, uint8 stop, uint8 data_len, const uint8* data_data);
			}
			namespace _timer
			{
				void _comparator (BLE_LowLevel* pLowLevel, uint8 timer, uint8 channel, uint8 mode, uint16 comparator_value);
			}
		}
		namespace _test
		{
			namespace _phy
			{
				void _tx (BLE_LowLevel* pLowLevel, uint8 channel, uint8 length, uint8 type);
				void _rx (BLE_LowLevel* pLowLevel, uint8 channel);
				void _end (BLE_LowLevel* pLowLevel);
				void _reset (BLE_LowLevel* pLowLevel);
			}
			namespace _get
			{
				namespace _channel
				{
					void _map (BLE_LowLevel* pLowLevel);
				}
			}
			void _debug (BLE_LowLevel* pLowLevel, uint8 input_len, const uint8* input_data);
		}
		namespace _dfu
		{
			void _reset (BLE_LowLevel* pLowLevel, uint8 dfu);
			namespace _flash
			{
				namespace _set
				{
					void _address (BLE_LowLevel* pLowLevel, uint32 address);
				}
				namespace _upload
				{
					void _finish (BLE_LowLevel* pLowLevel);
				}
			}
		}
	}
}



extern BLE_LowLevel* defaultLowLevel;

#ifdef __cplusplus
extern "C" {
#endif


#include "BLE_API_Types.h"

class ConnectionData;

void BLE_LL_InitBLEDevice();
void BLE_LL_ResetSystem();
void BLE_LL_SetWriteFunction(void (*writeFunction) (uint8, uint8*, uint16, uint8*));
int BLE_LL_ConnectToDevice(mac_address address, void (*connCallback) (ConnectionData* myData, uint8 connectionHandle));
int BLE_LL_DiscoverServicesInitiate(uint8 connectionID, uint16 start, uint16 end);
int BLE_LL_FindInformationOnService(uint8 connectionHandle, uint16 startAddress, uint16 endAddress);
int BLE_LL_AttributeWrite(uint8 connectionID, uint16 characteristicHandle, uint8 length, const uint8* data);
int BLE_LL_Disconnect(uint8 connectionHandle);
int BLE_LL_RestartScanner();
int BLE_LL_PollRSSI(uint8 connectionHandle);
int BLE_LL_ExtendedAttributeWrite(uint8 connectionID, uint16 attributeHandle, uint16 offset, uint8 length, const uint8* data);
int BLE_LL_ExecuteWrite(uint8 connectionID, uint8 commit);
int BLE_LL_ReadAttribute(uint8 connectionID, uint16 handle);
int BLE_LL_EndProcedure();
int BLE_LL_LongReadAttribute(uint8 connectionID, uint16 handle);

#ifdef __cplusplus
}
#endif

#endif