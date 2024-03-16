#include "BLE_LowLevel.h"
#include "BLE_IO.h"
#include "cmd_def.h"
#include "apitypes.h"
#include <string>



#include "DebugLogging.h"

BLE_LowLevel* defaultLowLevel = nullptr;

BLE_LowLevel::BLE_LowLevel()
{
	this->_BLE_IO = nullptr;
}

void BLE_LowLevel::SetIO(IBLE_IO* BLE_IO)
{
	_BLE_IO = BLE_IO;
}
void BLE_LowLevel::InitBLEDevice()
{
	RestartScanner();
}
void BLE_LowLevel::ResetSystem()
{
	_ble::_cmd::_system::_reset(this, 0);
}
BLE_API_RET BLE_LowLevel::ConnectToDevice(mac_address address)
{
	_ble::_cmd::_gap::_connect::_direct(this, &address, gap_address_type_public, 8, 16, 50, 0);
	return BLE_API_SUCCESS;
}

uint8 primaryServiceUUID[] = { 0x00, 0x28 };
BLE_API_RET BLE_LowLevel::DiscoverServicesInitiate(uint8 connectionID, uint16 start, uint16 end)
{
	_ble::_cmd::_attclient::_read::_by::_group::_type(this, connectionID, start, end, sizeof(primaryServiceUUID), primaryServiceUUID);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::FindInformationOnService(uint8 connectionHandle, uint16 startAddress, uint16 endAddress)
{
	_ble::_cmd::_attclient::_find::_information(this, connectionHandle, startAddress, endAddress);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::AttributeWrite(uint8 connectionID, uint16 characteristicHandle, uint8 length, const uint8* data)
{
	_ble::_cmd::_attclient::_attribute::_write(this, connectionID, characteristicHandle, length, data);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::Disconnect(uint8 connectionHandle)
{
	_ble::_cmd::_connection::_disconnect(this, connectionHandle);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::RestartScanner()
{
	//stop previous operation
	_ble::_cmd::_gap::_end::_procedure(this);

	//get connection status, current command will be handled in response
	_ble::_cmd::_connection::_get::_status(this, 0);

	_ble::_cmd::_gap::_set::_scan::_parameters(this, 50, 50, 1);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::PollRSSI(uint8 connectionHandle)
{
	_ble::_cmd::_connection::_get::_rssi(this, connectionHandle);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::ExtendedAttributeWrite(uint8 connectionID, uint16 attributeHandle, uint16 offset, uint8 length, const uint8* data)
{
	_ble::_cmd::_attclient::_prepare::_write(this, connectionID, attributeHandle, offset, length, data);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::ExecuteWrite(uint8 connectionID, uint8 commit)
{
	_ble::_cmd::_attclient::_execute::_write(this, connectionID, commit);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::ReadAttribute(uint8 connectionID, uint16 handle)
{
	_ble::_cmd::_attclient::_read::_by::_handle(this, connectionID, handle);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::EndProcedure()
{
	_ble::_cmd::_gap::_end::_procedure(this);
	return BLE_API_SUCCESS;
}
BLE_API_RET BLE_LowLevel::LongReadAttribute(uint8 connectionID, uint16 handle)
{
	_ble::_cmd::_attclient::_read::_long(this, connectionID, handle);
	return BLE_API_SUCCESS;
}

BLE_API_RET BLE_LowLevel::GapDiscover(uint8 mode)
{
	_ble::_cmd::_gap::_discover(this, mode);
	return BLE_API_SUCCESS;
}







void BLE_LowLevel::ble_send_message(uint8 msgid, ...)
{
	uint32 i;
	uint32 u32;
	uint16 u16;
	uint8  u8;
	struct ble_cmd_packet packet;
	uint8 *b = (uint8 *)&packet.payload;
	uint8 *hw;
	uint8 *data_ptr = 0;
	uint16 data_len = 0;
	va_list va;
	va_start(va, msgid);
	
	i = apis[msgid].params;
	packet.header = apis[msgid].hdr;
	while (i)
	{

		switch (i & 0xF)
		{

		case 7://int32
		case 6://uint32
			u32 = va_arg(va, uint32);
			*b++ = u32 & 0xff; u32 >>= 8;
			*b++ = u32 & 0xff; u32 >>= 8;
			*b++ = u32 & 0xff; u32 >>= 8;
			*b++ = u32 & 0xff;
			break;
		case 5://int16
		case 4://uint16
			u16 = va_arg(va, unsigned);
			*b++ = u16 & 0xff; u16 >>= 8;
			*b++ = u16 & 0xff;
			break;
		case 3://int8
		case 2://uint8
			u8 = va_arg(va, int);
			*b++ = u8 & 0xff;
			break;

		case 9://string
		case 8://uint8 array
			data_len = va_arg(va, int);
			*b++ = data_len;

			//assuming default packet<256
			u16 = data_len + packet.header.lolen;
			packet.header.lolen = u16 & 0xff;
			packet.header.type_hilen |= u16 >> 8;

			data_ptr = va_arg(va, uint8*);
			break;
		case 10://hwaddr
			hw = va_arg(va, uint8*);
			*b++ = *hw++;
			*b++ = *hw++;
			*b++ = *hw++;
			*b++ = *hw++;
			*b++ = *hw++;
			*b++ = *hw++;

			break;
		}
		i = i >> 4;
	}
	va_end(va);  
	
	if (_BLE_IO != nullptr)
	{
		_BLE_IO->output(sizeof(struct ble_header) + apis[msgid].hdr.lolen, (uint8*)&packet, data_len, (uint8*)data_ptr);
	}
}

#ifdef MiTokenBLE_ClasslessMode
void BLE_LL_InitBLEDevice()
{	
	
	DEBUG_ENTER_FUNCTION;

	BLE_LL_RestartScanner();

	DEBUG_END_FUNCTION;
}

void BLE_LL_ResetSystem()
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_system_reset(0);

	DEBUG_END_FUNCTION;
}

void BLE_LL_SetWriteFunction(void (*writeFunction) (uint8, const uint8*, uint16, const uint8*))
{
	DEBUG_ENTER_FUNCTION;

	bglib_output = writeFunction;

	DEBUG_END_FUNCTION;
}



int BLE_LL_ConnectToDevice(mac_address address, void (*connCallback) (ConnectionData* myData, uint8 connectionHandle))
{
	DEBUG_ENTER_FUNCTION;

	connectionCallback = connCallback;

	if(connCallback == NULL)
	{
		DEBUG_RETURN -1;
	}

	ble_cmd_gap_connect_direct(&address, gap_address_type_public, 40, 60, 100, 0);

	DEBUG_RETURN 0;
}

uint8 primaryServiceUID[] = {0x00, 0x28};

int BLE_LL_DiscoverServicesInitiate(uint8 connectionID, uint16 start, uint16 end)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_attclient_read_by_group_type(connectionID, start, end, sizeof(primaryServiceUID), primaryServiceUID);

	DEBUG_RETURN 0;
}

int BLE_LL_FindInformationOnService(uint8 connectionHandle, uint16 startAddress, uint16 endAddress)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_attclient_find_information(connectionHandle, startAddress, endAddress);

	DEBUG_RETURN 0;
}

int BLE_LL_AttributeWrite(uint8 connectionID, uint16 characteristicHandle, uint8 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_attclient_attribute_write(connectionID, characteristicHandle, length, data);

	DEBUG_RETURN 0;
}

int BLE_LL_Disconnect(uint8 connectionHandle)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_connection_disconnect(connectionHandle);

	DEBUG_RETURN 0;
}

int BLE_LL_RestartScanner()
{
	DEBUG_ENTER_FUNCTION;

	//stop previous operation
	ble_cmd_gap_end_procedure();
	//get connection status, current command will be handled in response
	ble_cmd_connection_get_status(0);

	ble_cmd_gap_set_scan_parameters(50, 50, 1); //50, 25, 1);
	DEBUG_RETURN 0;
}

int BLE_LL_PollRSSI(uint8 connectionHandle)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_connection_get_rssi(connectionHandle);

	DEBUG_RETURN 0;
}

int BLE_LL_ExtendedAttributeWrite(uint8 connectionID, uint16 attributeHandle, uint16 offset, uint8 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_attclient_prepare_write(connectionID, attributeHandle, offset, length, data);
	DEBUG_RETURN 0;
}

int BLE_LL_ExecuteWrite(uint8 connectionID, uint8 commit)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_attclient_execute_write(connectionID, commit);
	DEBUG_RETURN 0;
}

int BLE_LL_ReadAttribute(uint8 connectionID, uint16 handle)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_attclient_read_by_handle(connectionID, handle);
	DEBUG_RETURN 0;
}

int BLE_LL_EndProcedure()
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_gap_end_procedure();
	DEBUG_RETURN 0;
}

int BLE_LL_LongReadAttribute(uint8 connectionID, uint16 handle)
{
	DEBUG_ENTER_FUNCTION;

	ble_cmd_attclient_read_long(connectionID, handle);
	DEBUG_RETURN 0;
}

#else
void BLE_LL_InitBLEDevice()
{	
	return defaultLowLevel->InitBLEDevice();	
}

void BLE_LL_ResetSystem()
{
	return defaultLowLevel->ResetSystem();
}

void BLE_LL_SetWriteFunction(void (*writeFunction) (uint8, uint8*, uint16, uint8*))
{
	return;
}



int BLE_LL_ConnectToDevice(mac_address address, void (*connCallback) (ConnectionData* myData, uint8 connectionHandle))
{
	return defaultLowLevel->ConnectToDevice(address);
}

int BLE_LL_DiscoverServicesInitiate(uint8 connectionID, uint16 start, uint16 end)
{
	return defaultLowLevel->DiscoverServicesInitiate(connectionID, start, end);
}

int BLE_LL_FindInformationOnService(uint8 connectionHandle, uint16 startAddress, uint16 endAddress)
{
	return defaultLowLevel->FindInformationOnService(connectionHandle, startAddress, endAddress);
}

int BLE_LL_AttributeWrite(uint8 connectionID, uint16 characteristicHandle, uint8 length, const uint8* data)
{
	return defaultLowLevel->AttributeWrite(connectionID, characteristicHandle, length, data);
}

int BLE_LL_Disconnect(uint8 connectionHandle)
{
	return defaultLowLevel->Disconnect(connectionHandle);
}

int BLE_LL_RestartScanner()
{
	return defaultLowLevel->RestartScanner();
}

int BLE_LL_PollRSSI(uint8 connectionHandle)
{
	return defaultLowLevel->PollRSSI(connectionHandle);
}

int BLE_LL_ExtendedAttributeWrite(uint8 connectionID, uint16 attributeHandle, uint16 offset, uint8 length, const uint8* data)
{
	return defaultLowLevel->ExtendedAttributeWrite(connectionID, attributeHandle, offset, length, data);
}

int BLE_LL_ExecuteWrite(uint8 connectionID, uint8 commit)
{
	return defaultLowLevel->ExecuteWrite(connectionID, commit);
}

int BLE_LL_ReadAttribute(uint8 connectionID, uint16 handle)
{
	return defaultLowLevel->ReadAttribute(connectionID, handle);
}

int BLE_LL_EndProcedure()
{
	return defaultLowLevel->EndProcedure();
}

int BLE_LL_LongReadAttribute(uint8 connectionID, uint16 handle)
{
	return defaultLowLevel->LongReadAttribute(connectionID, handle);
}


#endif



#ifndef MiTokenBLE_ClasslessMode
void _ble::_cmd::_system::_reset(BLE_LowLevel* pLowLevel, uint8 boot_in_dfu)
{
	pLowLevel->ble_send_message(ble_cmd_system_reset_idx, boot_in_dfu);
}

void _ble::_cmd::_system::_hello(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_system_hello_idx);
}

void _ble::_cmd::_system::_address::_get(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_system_address_get_idx);
}

void _ble::_cmd::_system::_reg::_write(BLE_LowLevel* pLowLevel, uint16 address, uint8 value)
{
	pLowLevel->ble_send_message(ble_cmd_system_reg_write_idx, address, value);
}

void _ble::_cmd::_system::_reg::_read(BLE_LowLevel* pLowLevel, uint16 address)
{
	pLowLevel->ble_send_message(ble_cmd_system_reg_read_idx, address);
}

void _ble::_cmd::_system::_get::_counters(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_system_get_counters_idx);
}

void _ble::_cmd::_system::_get::_connections(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_system_get_connections_idx);
}

void _ble::_cmd::_system::_get::_info(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_system_get_info_idx);
}

void _ble::_cmd::_system::_read::_memory(BLE_LowLevel* pLowLevel, uint32 address, uint8 length)
{
	pLowLevel->ble_send_message(ble_cmd_system_read_memory_idx, address, length);
}

void _ble::_cmd::_system::_endpoint::_tx(BLE_LowLevel* pLowLevel, uint8 endpoint, uint8 data_len, const uint8* data_data)
{
	pLowLevel->ble_send_message(ble_cmd_system_endpoint_tx_idx, endpoint, data_len, data_data);
}

void _ble::_cmd::_system::_endpoint::_rx(BLE_LowLevel* pLowLevel, uint8 endpoint, uint8 size)
{
	pLowLevel->ble_send_message(ble_cmd_system_endpoint_rx_idx, endpoint, size);
}

void _ble::_cmd::_system::_endpoint::_set::_watermarks(BLE_LowLevel* pLowLevel, uint8 endpoint, uint8 rx, uint8 tx)
{
	pLowLevel->ble_send_message(ble_cmd_system_endpoint_set_watermarks_idx, endpoint, rx, tx);
}

void _ble::_cmd::_system::_whitelist::_append(BLE_LowLevel* pLowLevel, mac_address* address, uint8 address_type)
{
	pLowLevel->ble_send_message(ble_cmd_system_whitelist_append_idx, address, address_type);
}

void _ble::_cmd::_system::_whitelist::_remove(BLE_LowLevel* pLowLevel, mac_address* address, uint8 address_type)
{
	pLowLevel->ble_send_message(ble_cmd_system_whitelist_remove_idx, address, address_type);
}

void _ble::_cmd::_system::_whitelist::_clear(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_system_whitelist_clear_idx);
}

void _ble::_cmd::_flash::_ps::_defrag(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_flash_ps_defrag_idx);
}

void _ble::_cmd::_flash::_ps::_dump(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_flash_ps_dump_idx);
}

void _ble::_cmd::_flash::_ps::_erase::_all(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_flash_ps_erase_all_idx);
}

void _ble::_cmd::_flash::_ps::_save(BLE_LowLevel* pLowLevel, uint16 key, uint8 value_len, const uint8* value_data)
{
	pLowLevel->ble_send_message(ble_cmd_flash_ps_save_idx, key, value_len, value_data);
}

void _ble::_cmd::_flash::_ps::_load(BLE_LowLevel* pLowLevel, uint16 key)
{
	pLowLevel->ble_send_message(ble_cmd_flash_ps_load_idx, key);
}

void _ble::_cmd::_flash::_erase::_page(BLE_LowLevel* pLowLevel, uint8 page)
{
	pLowLevel->ble_send_message(ble_cmd_flash_erase_page_idx, page);
}

void _ble::_cmd::_flash::_write::_words(BLE_LowLevel* pLowLevel, uint16 address, uint8 words_len, const uint8* words_data)
{
	pLowLevel->ble_send_message(ble_cmd_flash_write_words_idx, address, words_len, words_data);
}

void _ble::_cmd::_attributes::_write(BLE_LowLevel* pLowLevel, uint16 handle, uint8 offset, uint8 value_len, const uint8* value_data)
{
	pLowLevel->ble_send_message(ble_cmd_attributes_write_idx, handle, offset, value_len, value_data);
}

void _ble::_cmd::_attributes::_read::_type(BLE_LowLevel* pLowLevel, uint16 handle)
{
	pLowLevel->ble_send_message(ble_cmd_attributes_read_type_idx, handle);
}

void _ble::_cmd::_attributes::_user::_read::_response(BLE_LowLevel* pLowLevel, uint8 connection, uint8 att_error, uint8 value_len, const uint8* value_data)
{
	pLowLevel->ble_send_message(ble_cmd_attributes_user_read_response_idx, connection, att_error, value_len, value_data);
}

void _ble::_cmd::_attributes::_user::_write::_response(BLE_LowLevel* pLowLevel, uint8 connection, uint8 att_error)
{
	pLowLevel->ble_send_message(ble_cmd_attributes_user_write_response_idx, connection, att_error);
}

void _ble::_cmd::_connection::_disconnect(BLE_LowLevel* pLowLevel, uint8 connection)
{
	pLowLevel->ble_send_message(ble_cmd_connection_disconnect_idx, connection);
}

void _ble::_cmd::_connection::_get::_rssi(BLE_LowLevel* pLowLevel, uint8 connection)
{
	pLowLevel->ble_send_message(ble_cmd_connection_get_rssi_idx, connection);
}

void _ble::_cmd::_connection::_get::_status(BLE_LowLevel* pLowLevel, uint8 connection)
{
	pLowLevel->ble_send_message(ble_cmd_connection_get_status_idx, connection);
}

void _ble::_cmd::_connection::_update(BLE_LowLevel* pLowLevel, uint8 connection, uint16 interval_min, uint16 interval_max, uint16 latency, uint16 timeout)
{
	pLowLevel->ble_send_message(ble_cmd_connection_update_idx, connection, interval_min, interval_max, latency, timeout);
}

void _ble::_cmd::_connection::_version::_update(BLE_LowLevel* pLowLevel, uint8 connection)
{
	pLowLevel->ble_send_message(ble_cmd_connection_version_update_idx, connection);
}

void _ble::_cmd::_connection::_channel::_map::_get(BLE_LowLevel* pLowLevel, uint8 connection)
{
	pLowLevel->ble_send_message(ble_cmd_connection_channel_map_get_idx, connection);
}

void _ble::_cmd::_connection::_channel::_map::_set(BLE_LowLevel* pLowLevel, uint8 connection, uint8 map_len, const uint8* map_data)
{
	pLowLevel->ble_send_message(ble_cmd_connection_channel_map_set_idx, connection, map_len, map_data);
}

void _ble::_cmd::_connection::_features::_get(BLE_LowLevel* pLowLevel, uint8 connection)
{
	pLowLevel->ble_send_message(ble_cmd_connection_features_get_idx, connection);
}

void _ble::_cmd::_connection::_raw::_tx(BLE_LowLevel* pLowLevel, uint8 connection, uint8 data_len, const uint8* data_data)
{
	pLowLevel->ble_send_message(ble_cmd_connection_raw_tx_idx, connection, data_len, data_data);
}

void _ble::_cmd::_attclient::_find::_by::_type::_value(BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end, uint16 uuid, uint8 value_len, const uint8* value_data)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_find_by_type_value_idx, connection, start, end, uuid, value_len, value_data);
}

void _ble::_cmd::_attclient::_find::_information(BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_find_information_idx, connection, start, end);
}

void _ble::_cmd::_attclient::_read::_by::_group::_type(BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end, uint8 uuid_len, const uint8* uuid_data)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_read_by_group_type_idx, connection, start, end, uuid_len, uuid_data);
}

void _ble::_cmd::_attclient::_read::_by::_type(BLE_LowLevel* pLowLevel, uint8 connection, uint16 start, uint16 end, uint8 uuid_len, const uint8* uuid_data)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_read_by_type_idx, connection, start, end, uuid_len, uuid_data);
}

void _ble::_cmd::_attclient::_read::_by::_handle(BLE_LowLevel* pLowLevel, uint8 connection, uint16 chrhandle)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_read_by_handle_idx, connection, chrhandle);
}

void _ble::_cmd::_attclient::_read::_long(BLE_LowLevel* pLowLevel, uint8 connection, uint16 chrhandle)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_read_long_idx, connection, chrhandle);
}

void _ble::_cmd::_attclient::_read::_multiple(BLE_LowLevel* pLowLevel, uint8 connection, uint8 handles_len, const uint8* handles_data)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_read_multiple_idx, connection, handles_len, handles_data);
}

void _ble::_cmd::_attclient::_attribute::_write(BLE_LowLevel* pLowLevel, uint8 connection, uint16 atthandle, uint8 data_len, const uint8* data_data)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_attribute_write_idx, connection, atthandle, data_len, data_data);
}

void _ble::_cmd::_attclient::_write::_command(BLE_LowLevel* pLowLevel, uint8 connection, uint16 atthandle, uint8 data_len, const uint8* data_data)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_write_command_idx, connection, atthandle, data_len, data_data);
}

void _ble::_cmd::_attclient::_indicate::_confirm(BLE_LowLevel* pLowLevel, uint8 connection)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_indicate_confirm_idx, connection);
}

void _ble::_cmd::_attclient::_prepare::_write(BLE_LowLevel* pLowLevel, uint8 connection, uint16 atthandle, uint16 offset, uint8 data_len, const uint8* data_data)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_prepare_write_idx, connection, atthandle, offset, data_len, data_data);
}

void _ble::_cmd::_attclient::_execute::_write(BLE_LowLevel* pLowLevel, uint8 connection, uint8 commit)
{
	pLowLevel->ble_send_message(ble_cmd_attclient_execute_write_idx, connection, commit);
}

void _ble::_cmd::_sm::_encrypt::_start(BLE_LowLevel* pLowLevel, uint8 handle, uint8 bonding)
{
	pLowLevel->ble_send_message(ble_cmd_sm_encrypt_start_idx, handle, bonding);
}

void _ble::_cmd::_sm::_set::_bondable::_mode(BLE_LowLevel* pLowLevel, uint8 bondable)
{
	pLowLevel->ble_send_message(ble_cmd_sm_set_bondable_mode_idx, bondable);
}

void _ble::_cmd::_sm::_set::_parameters(BLE_LowLevel* pLowLevel, uint8 mitm, uint8 min_key_size, uint8 io_capabilities)
{
	pLowLevel->ble_send_message(ble_cmd_sm_set_parameters_idx, mitm, min_key_size, io_capabilities);
}

void _ble::_cmd::_sm::_set::_oob::_data(BLE_LowLevel* pLowLevel, uint8 oob_len, const uint8* oob_data)
{
	pLowLevel->ble_send_message(ble_cmd_sm_set_oob_data_idx, oob_len, oob_data);
}

void _ble::_cmd::_sm::_delete::_bonding(BLE_LowLevel* pLowLevel, uint8 handle)
{
	pLowLevel->ble_send_message(ble_cmd_sm_delete_bonding_idx, handle);
}

void _ble::_cmd::_sm::_passkey::_entry(BLE_LowLevel* pLowLevel, uint8 handle, uint32 passkey)
{
	pLowLevel->ble_send_message(ble_cmd_sm_passkey_entry_idx, handle, passkey);
}

void _ble::_cmd::_sm::_get::_bonds(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_sm_get_bonds_idx);
}

void _ble::_cmd::_gap::_set::_privacy::_flags(BLE_LowLevel* pLowLevel, uint8 peripheral_privacy, uint8 central_privacy)
{
	pLowLevel->ble_send_message(ble_cmd_gap_set_privacy_flags_idx, peripheral_privacy, central_privacy);
}

void _ble::_cmd::_gap::_set::_mode(BLE_LowLevel* pLowLevel, uint8 discover, uint8 connect)
{
	pLowLevel->ble_send_message(ble_cmd_gap_set_mode_idx, discover, connect);
}

void _ble::_cmd::_gap::_set::_filtering(BLE_LowLevel* pLowLevel, uint8 scan_policy, uint8 adv_policy, uint8 scan_duplicate_filtering)
{
	pLowLevel->ble_send_message(ble_cmd_gap_set_filtering_idx, scan_policy, adv_policy, scan_duplicate_filtering);
}

void _ble::_cmd::_gap::_set::_scan::_parameters(BLE_LowLevel* pLowLevel, uint16 scan_interval, uint16 scan_window, uint8 active)
{
	pLowLevel->ble_send_message(ble_cmd_gap_set_scan_parameters_idx, scan_interval, scan_window, active);
}

void _ble::_cmd::_gap::_set::_adv::_parameters(BLE_LowLevel* pLowLevel, uint16 adv_interval_min, uint16 adv_interval_max, uint8 adv_channels)
{
	pLowLevel->ble_send_message(ble_cmd_gap_set_adv_parameters_idx, adv_interval_min, adv_interval_max, adv_channels);
}

void _ble::_cmd::_gap::_set::_adv::_data(BLE_LowLevel* pLowLevel, uint8 set_scanrsp, uint8 adv_data_len, const uint8* adv_data_data)
{
	pLowLevel->ble_send_message(ble_cmd_gap_set_adv_data_idx, set_scanrsp, adv_data_len, adv_data_data);
}

void _ble::_cmd::_gap::_set::_directed::_connectable::_mode(BLE_LowLevel* pLowLevel, mac_address* address, uint8 addr_type)
{
	pLowLevel->ble_send_message(ble_cmd_gap_set_directed_connectable_mode_idx, address, addr_type);
}

void _ble::_cmd::_gap::_discover(BLE_LowLevel* pLowLevel, uint8 mode)
{
	pLowLevel->ble_send_message(ble_cmd_gap_discover_idx, mode);
}

void _ble::_cmd::_gap::_connect::_direct(BLE_LowLevel* pLowLevel, mac_address* address, uint8 addr_type, uint16 conn_interval_min, uint16 conn_interval_max, uint16 timeout, uint16 latency)
{
	pLowLevel->ble_send_message(ble_cmd_gap_connect_direct_idx, address, addr_type, conn_interval_min, conn_interval_max, timeout, latency);
}

void _ble::_cmd::_gap::_connect::_selective(BLE_LowLevel* pLowLevel, uint16 conn_interval_min, uint16 conn_interval_max, uint16 timeout, uint16 latency)
{
	pLowLevel->ble_send_message(ble_cmd_gap_connect_selective_idx, conn_interval_min, conn_interval_max, timeout, latency);
}

void _ble::_cmd::_gap::_end::_procedure(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_gap_end_procedure_idx);
}

void _ble::_cmd::_hardware::_io::_port::_config::_irq(BLE_LowLevel* pLowLevel, uint8 port, uint8 enable_bits, uint8 falling_edge)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_io_port_config_irq_idx, port, enable_bits, falling_edge);
}

void _ble::_cmd::_hardware::_io::_port::_config::_direction(BLE_LowLevel* pLowLevel, uint8 port, uint8 direction)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_io_port_config_direction_idx, port, direction);
}

void _ble::_cmd::_hardware::_io::_port::_config::_function(BLE_LowLevel* pLowLevel, uint8 port, uint8 function)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_io_port_config_function_idx, port, function);
}

void _ble::_cmd::_hardware::_io::_port::_config::_pull(BLE_LowLevel* pLowLevel, uint8 port, uint8 tristate_mask, uint8 pull_up)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_io_port_config_pull_idx, port, tristate_mask, pull_up);
}

void _ble::_cmd::_hardware::_io::_port::_write(BLE_LowLevel* pLowLevel, uint8 port, uint8 mask, uint8 data)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_io_port_write_idx, port, mask, data);
}

void _ble::_cmd::_hardware::_io::_port::_read(BLE_LowLevel* pLowLevel, uint8 port, uint8 mask)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_io_port_read_idx, port, mask);
}

void _ble::_cmd::_hardware::_set::_soft::_timer(BLE_LowLevel* pLowLevel, uint32 time, uint8 handle, uint8 single_shot)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_set_soft_timer_idx, time, handle, single_shot);
}

void _ble::_cmd::_hardware::_set::_txpower(BLE_LowLevel* pLowLevel, uint8 power)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_set_txpower_idx, power);
}

void _ble::_cmd::_hardware::_adc::_read(BLE_LowLevel* pLowLevel, uint8 input, uint8 decimation, uint8 reference_selection)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_adc_read_idx, input, decimation, reference_selection);
}

void _ble::_cmd::_hardware::_spi::_config(BLE_LowLevel* pLowLevel, uint8 channel, uint8 polarity, uint8 phase, uint8 bit_order, uint8 baud_e, uint8 baud_m)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_spi_config_idx, channel, polarity, phase, bit_order, baud_e, baud_m);
}

void _ble::_cmd::_hardware::_spi::_transfer(BLE_LowLevel* pLowLevel, uint8 channel, uint8 data_len, const uint8* data_data)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_spi_transfer_idx, channel, data_len, data_data);
}

void _ble::_cmd::_hardware::_i2c::_read(BLE_LowLevel* pLowLevel, uint8 address, uint8 stop, uint8 length)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_i2c_read_idx, address, stop, length);
}

void _ble::_cmd::_hardware::_i2c::_write(BLE_LowLevel* pLowLevel, uint8 address, uint8 stop, uint8 data_len, const uint8* data_data)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_i2c_write_idx, address, stop, data_len, data_data);
}

void _ble::_cmd::_hardware::_timer::_comparator(BLE_LowLevel* pLowLevel, uint8 timer, uint8 channel, uint8 mode, uint16 comparator_value)
{
	pLowLevel->ble_send_message(ble_cmd_hardware_timer_comparator_idx, timer, channel, mode, comparator_value);
}

void _ble::_cmd::_test::_phy::_tx(BLE_LowLevel* pLowLevel, uint8 channel, uint8 length, uint8 type)
{
	pLowLevel->ble_send_message(ble_cmd_test_phy_tx_idx, channel, length, type);
}

void _ble::_cmd::_test::_phy::_rx(BLE_LowLevel* pLowLevel, uint8 channel)
{
	pLowLevel->ble_send_message(ble_cmd_test_phy_rx_idx, channel);
}

void _ble::_cmd::_test::_phy::_end(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_test_phy_end_idx);
}

void _ble::_cmd::_test::_phy::_reset(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_test_phy_reset_idx);
}

void _ble::_cmd::_test::_get::_channel::_map(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_test_get_channel_map_idx);
}

void _ble::_cmd::_test::_debug(BLE_LowLevel* pLowLevel, uint8 input_len, const uint8* input_data)
{
	pLowLevel->ble_send_message(ble_cmd_test_debug_idx, input_len, input_data);
}

void _ble::_cmd::_dfu::_reset(BLE_LowLevel* pLowLevel, uint8 dfu)
{
	pLowLevel->ble_send_message(ble_cmd_dfu_reset_idx, dfu);
}

void _ble::_cmd::_dfu::_flash::_set::_address(BLE_LowLevel* pLowLevel, uint32 address)
{
	pLowLevel->ble_send_message(ble_cmd_dfu_flash_set_address_idx, address);
}

void _ble::_cmd::_dfu::_flash::_upload::_finish(BLE_LowLevel* pLowLevel)
{
	pLowLevel->ble_send_message(ble_cmd_dfu_flash_upload_finish_idx);
}

#endif