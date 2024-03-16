//
// Bluegiga’s Bluetooth Smart Demo Application
// Contact: support@bluegiga.com.
//
// This is free software distributed under the terms of the MIT license reproduced below.
//
// Copyright (c) 2012, Bluegiga Technologies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmd_def.h"
#include "BLELib.h"
#include "BLE_Structs.h"

#include <time.h>
#include "DebugLogging.h"
#include "MiTokenBLE.h"
#include "MiTokenBLEConnection.h"

struct _user_init_data user_init_data;

int IIR_Percent = 0;
void (*connectionCallback) (ConnectionData* myData, uint8 connectionHandle) = NULL;

#if USE_MEM_CHECK
#include "MainLib.h"

#define V2_2InterfaceSetup \
	MemLeak_SetLocationID(500); \
	MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender); \
	MemLeak_SetLocationID(501);

#define ProcessChain(type, section, function, passThrough) \
	MemLeak_SetLocationID(502); \
	const IBLE_COMMAND_CHAIN_LINK* chain = v2_2Interface->GetBaseCommandChain(); \
	while(chain != nullptr) \
	{ \
		MemLeak_SetLocationID(504); \
		if(chain->isLinkModular()) \
		{ \
			const BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD* chainHead = dynamic_cast<const BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD*>(chain); \
			if((chainHead != nullptr) && (chainHead-> type != nullptr) && (chainHead-> type -> section != nullptr) && (chainHead-> type -> section -> function != nullptr)) \
			{ \
				chainHead-> type -> section -> function(v2_2Interface, passThrough); \
			} \
		} \
		else \
		{ \
			const BLE_COMMAND_CHAIN_LINK_FULL* chainFull = dynamic_cast<const BLE_COMMAND_CHAIN_LINK_FULL*>(chain); \
			if(chainFull-> type . section . function != nullptr) \
			{ \
				chainFull-> type . section. function(v2_2Interface, passThrough); \
			} \
		} \
		MemLeak_SetLocationID(505); \
		chain = chain->getNextLink(); \
	} \
	MemLeak_SetLocationID(1000 + __LINE__);

#else
#define V2_2InterfaceSetup \
	MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender);


#define ProcessChain(type, section, function, passThrough) \
	const IBLE_COMMAND_CHAIN_LINK* chain = v2_2Interface->GetBaseCommandChain(); \
	while(chain != nullptr) \
	{ \
		if(chain->isLinkModular()) \
		{ \
			const BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD* chainHead = dynamic_cast<const BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD*>(chain); \
			if((chainHead != nullptr) && (chainHead-> type != nullptr) && (chainHead-> type -> section != nullptr) && (chainHead-> type -> section -> function != nullptr)) \
			{ \
				chainHead-> type -> section -> function(v2_2Interface, passThrough); \
			} \
		} \
		else \
		{ \
			const BLE_COMMAND_CHAIN_LINK_FULL* chainFull = dynamic_cast<const BLE_COMMAND_CHAIN_LINK_FULL*>(chain); \
			if(chainFull-> type . section . function != nullptr) \
			{ \
				chainFull-> type . section. function(v2_2Interface, passThrough); \
			} \
		} \
		chain = chain->getNextLink(); \
	}

#endif
/*
void defaultSetExtraData(struct ConnectionData* connData)
{
	connData->extraData = NULL;
}

void defaultFreeExtraData(struct ConnectionData* connData)
{
	connData->extraData = NULL;
}
*/
//void (*setExtraData) (struct ConnectionData* connData) = &defaultSetExtraData;
//void (*freeExtraData) (struct ConnectionData* connData) = &defaultFreeExtraData;

/*
void (*serviceFoundCallback) (uint8 serviceHandleLength, const uint8* serviceHandleData, uint16 start, uint16 end) = NULL;
void (*serviceSearchFinishedCallback) () = NULL;

void (*serviceInformationFoundCallback) (uint16 characteristicsHandle, uint8 dataLen, const uint8* data) = NULL;
void (*serviceInformationCompletedCallback) () = NULL;

void (*handleValueCallback) (uint8 type, uint8 dataLen, const uint8* data) = NULL;
void (*writeAttributeValueCallback) () = NULL;
*/


//struct ConnectionData* rootConnectionData = NULL;

/*
struct ConnectionData* getConnectionDataIfExists(int connectionID, ConnectionData** retLastNode)
{
	struct ConnectionData* retValue = NULL;
	struct ConnectionData* currentSearch = rootConnectionData;
	struct ConnectionData* lastNode = NULL;

	while((currentSearch != NULL) && (retValue == NULL))
	{
		lastNode = currentSearch;
		if(currentSearch->connectionID == connectionID)
		{
			retValue = currentSearch;
		}
		currentSearch = currentSearch->nextNode;
	}

	//set *retLastNode to the lastNode we checked if retLastNode is non-null
	if(retLastNode)
	{
		*retLastNode = lastNode;
	}

	return retValue;
}
*/

ConnectionData* getConnectionData(int connectionID)
{
	
	ConnectionData* retValue = NULL;
	ConnectionData* lastNode = NULL;

	//will return NULL if connectionID doesn't exist, otherwise will return a pointer to the struct
	
	retValue = ConnectionData::findNodeWithID(connectionID, true);
	return retValue;
}

void cleanConnectionData(int connectionID)
{
	ConnectionData::removeNodeWithID(connectionID);
}

//Same as cleanConnectionData but with every single node
void cleanupAllConnectionData()
{
	ConnectionData::CleanupData();	
}

void ble_default(HANDLER_START const void*v)
{
}

void ble_evt_system_no_license_key(HANDLER_START const void*nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, system, no_license_key, nul);
	}
}

void ble_evt_attributes_status(HANDLER_START const struct ble_msg_attributes_status_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attributes, status, msg);
	}
}

void ble_rsp_attclient_indicate_confirm(HANDLER_START const struct ble_msg_attclient_indicate_confirm_rsp_t*msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, indicate_confirm, msg);
	}
}

void ble_rsp_test_debug(HANDLER_START const struct ble_msg_test_debug_rsp_t*msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, test, debug, msg);
	}
}

void ble_rsp_hardware_set_txpower(HANDLER_START const void *nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, set_txpower, nul);
	}
}

void ble_rsp_attclient_read_multiple(HANDLER_START const struct ble_msg_attclient_read_multiple_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, read_multiple, msg);
	}
}

void ble_evt_attclient_read_multiple_response(HANDLER_START const struct ble_msg_attclient_read_multiple_response_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attclient, read_multiple_response, msg);
	}
}

void ble_rsp_attclient_prepare_write(HANDLER_START const struct ble_msg_attclient_prepare_write_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, prepare_write, msg);
	}
	else
	{
		MiTokenBLEConnection* conn = dynamic_cast<MiTokenBLEConnection*>(sender->GetConnectionWithConnectionID(msg->connection));
		if(conn != nullptr)
		{
			conn->CALLBACK_PrepareWriteResponse(msg->result);
		}

		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);
		if(connData)
		{
			if(connData->callbacks.prepareWriteResponse != NULL)
			{
				connData->callbacks.prepareWriteResponse(connData, msg->result);
			}
		}
	}
}

void ble_rsp_attclient_execute_write(HANDLER_START const struct ble_msg_attclient_execute_write_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, execute_write, msg);
	}
}

void ble_evt_attributes_user_read_request(HANDLER_START const struct ble_msg_attributes_user_read_request_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attributes, user_read_request, msg);
	}
}

void ble_rsp_attributes_user_read_response(HANDLER_START const void * nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attributes, user_read_response, nul);
	}
}

void ble_rsp_attributes_user_write_response(HANDLER_START const void * nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attributes, user_write_response, nul);
	}
}

void ble_rsp_system_whitelist_clear(HANDLER_START const void *nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, whitelist_clear, nul);
	}
}

void ble_rsp_attclient_write_command(HANDLER_START const struct ble_msg_attclient_write_command_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, write_command, msg);
	}
}

void ble_rsp_attclient_reserved(HANDLER_START const void *nul)
{

}

void ble_rsp_attclient_read_long(HANDLER_START const struct ble_msg_attclient_read_long_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, read_long, msg);
	}
}

void ble_rsp_system_whitelist_append(HANDLER_START const struct ble_msg_system_whitelist_append_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, whitelist_append, msg);
	}
}

void ble_rsp_sm_set_parameters(HANDLER_START const void *nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, sm, set_parameters, nul);
	}
}

void ble_rsp_sm_passkey_entry(HANDLER_START const struct ble_msg_sm_passkey_entry_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, sm, passkey_entry, msg);
	}
}

void ble_rsp_sm_get_bonds(HANDLER_START const struct ble_msg_sm_get_bonds_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, sm, get_bonds, msg);
	}
}

void ble_rsp_sm_set_oob_data(HANDLER_START const void *nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, sm, set_oob_data, nul);
	}
}

void ble_rsp_sm_set_security_mode(HANDLER_START const void *nul)
{
}

void ble_rsp_gap_set_filtering(HANDLER_START const struct ble_msg_gap_set_filtering_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, set_filtering, msg);
	}
}

void ble_rsp_gap_set_adv_parameters(HANDLER_START const struct ble_msg_gap_set_adv_parameters_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, set_adv_parameters, msg);
	}
}

void ble_rsp_hardware_io_port_write(HANDLER_START const struct ble_msg_hardware_io_port_write_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, io_port_write, msg);
	}
}

void ble_rsp_hardware_io_port_read(HANDLER_START const struct ble_msg_hardware_io_port_read_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, io_port_read, msg);
	}
}

void ble_rsp_hardware_timer_comparator(HANDLER_START const struct ble_msg_hardware_timer_comparator_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, timer_comparator, msg);
	}
}

void ble_rsp_hardware_spi_config(HANDLER_START const struct ble_msg_hardware_spi_config_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, spi_config, msg);
	}
}

void ble_rsp_hardware_spi_transfer(HANDLER_START const struct ble_msg_hardware_spi_transfer_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, spi_transfer, msg);
	}
}

void ble_rsp_hardware_i2c_read(HANDLER_START const struct ble_msg_hardware_i2c_read_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, i2c_read, msg);
	}
}

void ble_rsp_hardware_i2c_write(HANDLER_START const struct ble_msg_hardware_i2c_write_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, i2c_write, msg);
	}
}

void ble_rsp_test_get_channel_map(HANDLER_START const struct ble_msg_test_get_channel_map_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, test, get_channel_map, msg);
	}
}

void ble_evt_attributes_value(HANDLER_START const struct ble_msg_attributes_value_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attributes, value, msg);
	}
}

void ble_evt_sm_bonding_fail(HANDLER_START const struct ble_msg_sm_bonding_fail_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, sm, bonding_fail, msg);
	}
}

void ble_evt_sm_passkey_display(HANDLER_START const struct ble_msg_sm_passkey_display_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, sm, passkey_display, msg);
	}
}

void ble_evt_sm_passkey_request(HANDLER_START const struct ble_msg_sm_passkey_request_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, sm, passkey_request, msg);
	}
}

void ble_evt_sm_bond_status(HANDLER_START const struct ble_msg_sm_bond_status_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, sm, bond_status, msg);
	}
}

void ble_rsp_gap_set_adv_data(HANDLER_START const struct ble_msg_gap_set_adv_data_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, set_adv_data, msg);
	}
}

void ble_rsp_gap_set_scan_parameters(HANDLER_START const struct ble_msg_gap_set_scan_parameters_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, set_scan_parameters, msg);
	}
}

void ble_rsp_gap_set_directed_connectable_mode(HANDLER_START const struct ble_msg_gap_set_directed_connectable_mode_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, set_directed_connectable_mode, msg);
	}
}

void ble_rsp_hardware_io_port_config_direction(HANDLER_START const struct ble_msg_hardware_io_port_config_direction_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, io_port_config_direction, msg);
	}
}

void ble_rsp_hardware_io_port_config_pull(HANDLER_START const struct ble_msg_hardware_io_port_config_pull_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, io_port_config_pull, msg);
	}
}

void ble_rsp_hardware_io_port_config_function(HANDLER_START const struct ble_msg_hardware_io_port_config_function_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, io_port_config_function, msg);
	}
}

void ble_rsp_gap_set_privacy_flags(HANDLER_START const void *nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, set_privacy_flags, nul);
	}
}

void ble_rsp_gap_connect_selective(HANDLER_START const struct ble_msg_gap_connect_selective_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, connect_selective, msg);
	}
}

void ble_rsp_system_whitelist_remove(HANDLER_START const struct ble_msg_system_whitelist_remove_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, whitelist_remove, msg);
	}
}

void ble_rsp_system_reset(HANDLER_START const void* nul)
{
}

void ble_rsp_system_hello(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, hello, nul);
	}
}

void ble_rsp_system_address_get(HANDLER_START const struct ble_msg_system_address_get_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, address_get, msg);
	}
}

void ble_rsp_system_reg_write(HANDLER_START const struct ble_msg_system_reg_write_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, reg_write, msg);
	}
}

void ble_rsp_system_reg_read(HANDLER_START const struct ble_msg_system_reg_read_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, reg_read, msg);
	}
}

void ble_rsp_system_get_counters(HANDLER_START const struct ble_msg_system_get_counters_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, get_counters, msg);
	}
}

void ble_rsp_system_get_connections(HANDLER_START const struct ble_msg_system_get_connections_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, get_connections, msg);
	}
}

void ble_rsp_system_read_memory(HANDLER_START const struct ble_msg_system_read_memory_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, read_memory, msg);
	}
}

void ble_rsp_system_get_info(HANDLER_START const struct ble_msg_system_get_info_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, get_info, msg);
	}
}

void ble_rsp_system_endpoint_tx(HANDLER_START const struct ble_msg_system_endpoint_tx_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, endpoint_tx, msg);
	}
}

void ble_rsp_flash_ps_defrag(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, ps_defrag, nul);
	}
}

void ble_rsp_flash_ps_dump(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, ps_dump, nul);
	}
}

void ble_rsp_flash_ps_erase_all(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, ps_erase_all, nul);
	}
}

void ble_rsp_flash_ps_save(HANDLER_START const struct ble_msg_flash_ps_save_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, ps_save, msg);
	}
}

void ble_rsp_flash_ps_load(HANDLER_START const struct ble_msg_flash_ps_load_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, ps_load, msg);
	}
}

void ble_rsp_flash_ps_erase(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, ps_erase, nul);
	}
}

void ble_rsp_flash_erase_page(HANDLER_START const struct ble_msg_flash_erase_page_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, erase_page, msg);
	}
}

void ble_rsp_flash_write_words(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, flash, write_words, nul);
	}
}

void ble_rsp_attributes_write(HANDLER_START const struct ble_msg_attributes_write_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attributes, write, msg);
	}
}

void ble_rsp_attributes_read(HANDLER_START const struct ble_msg_attributes_read_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attributes, read, msg);
	}
}

void ble_rsp_attributes_read_type(HANDLER_START const struct ble_msg_attributes_read_type_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attributes, read_type, msg);
	}
}

void ble_rsp_connection_disconnect(HANDLER_START const struct ble_msg_connection_disconnect_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, disconnect, msg);
	}
	else
	{
		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);

		if(connData)
		{
			if(msg->result == 0)
			{
				//if this is successful then connData will be reset to NULL.
	#ifndef _NOPRINT
				printf("Successfully Disconnected\r\n");
	#endif

				ConnectionData::closeConnection(connData);
			}
		}
	}
}

void ble_rsp_connection_get_rssi(HANDLER_START const struct ble_msg_connection_get_rssi_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, get_rssi, msg);
	}
	else
	{
		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);

		if(connData)
		{
			if(connData->callbacks.RSSIValue)
			{
				connData->callbacks.RSSIValue(connData, msg->rssi);
			}
		}
	}
}

void ble_rsp_connection_update(HANDLER_START const struct ble_msg_connection_update_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, update, msg);
	}
}

void ble_rsp_connection_version_update(HANDLER_START const struct ble_msg_connection_version_update_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, version_update, msg);
	}
}

void ble_rsp_connection_channel_map_get(HANDLER_START const struct ble_msg_connection_channel_map_get_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, channel_map_get, msg);
	}
}

void ble_rsp_connection_channel_map_set(HANDLER_START const struct ble_msg_connection_channel_map_set_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, channel_map_set, msg);
	}
}

void ble_rsp_connection_features_get(HANDLER_START const struct ble_msg_connection_features_get_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, features_get, msg);
	}
}

void ble_rsp_connection_get_status(HANDLER_START const struct ble_msg_connection_get_status_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, get_status, msg);
	}
}

void ble_rsp_connection_raw_tx(HANDLER_START const struct ble_msg_connection_raw_tx_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, connection, raw_tx, msg);
	}
}

void ble_rsp_attclient_find_by_type_value(HANDLER_START const struct ble_msg_attclient_find_by_type_value_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, find_by_type_value, msg);
	}
}

void ble_rsp_attclient_read_by_group_type(HANDLER_START const struct ble_msg_attclient_read_by_group_type_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, read_by_group_type, msg);
	}
}

void ble_rsp_attclient_read_by_type(HANDLER_START const struct ble_msg_attclient_read_by_type_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, read_by_type, msg);
	}
}

void ble_rsp_attclient_find_information(HANDLER_START const struct ble_msg_attclient_find_information_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, find_information, msg);
	}
}

void ble_rsp_attclient_read_by_handle(HANDLER_START const struct ble_msg_attclient_read_by_handle_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, read_by_handle, msg);
	}
}

void ble_rsp_attclient_attribute_write(HANDLER_START const struct ble_msg_attclient_attribute_write_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, attclient, attribute_write, msg);
	}
}

void ble_rsp_sm_encrypt_start(HANDLER_START const struct ble_msg_sm_encrypt_start_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, sm, encrypt_start, msg);
	}
}

void ble_rsp_sm_set_bondable_mode(HANDLER_START const void* nul)
{
}

void ble_rsp_sm_delete_bonding(HANDLER_START const struct ble_msg_sm_delete_bonding_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, sm, delete_bonding, msg);
	}
}

void ble_rsp_gap_set_address_mode(HANDLER_START const void* nul)
{
}

void ble_rsp_gap_set_mode(HANDLER_START const struct ble_msg_gap_set_mode_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, set_mode, msg);
	}
}

void ble_rsp_gap_discover(HANDLER_START const struct ble_msg_gap_discover_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, discover, msg);
	}
}

void ble_rsp_gap_connect_direct(HANDLER_START const struct ble_msg_gap_connect_direct_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, connect_direct, msg);
	}
}

void ble_rsp_gap_end_procedure(HANDLER_START const struct ble_msg_gap_end_procedure_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, gap, end_procedure, msg);
	}
}


void ble_rsp_hardware_io_port_config_irq(HANDLER_START const struct ble_msg_hardware_io_port_config_irq_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, io_port_config_irq, msg);
	}
}

void ble_rsp_hardware_set_soft_timer(HANDLER_START const struct ble_msg_hardware_set_soft_timer_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, set_soft_timer, msg);
	}
}

void ble_rsp_hardware_adc_read(HANDLER_START const struct ble_msg_hardware_adc_read_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, hardware, adc_read, msg);
	}
}

void ble_rsp_test_phy_tx(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, test, phy_tx, nul);
	}
}

void ble_rsp_test_phy_rx(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, test, phy_rx, nul);
	}
}

void ble_rsp_test_phy_end(HANDLER_START const struct ble_msg_test_phy_end_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, test, phy_end, msg);
	}
}

void ble_rsp_test_phy_reset(HANDLER_START const void* nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, test, phy_reset, nul);
	}
}

void ble_evt_system_boot(HANDLER_START const struct ble_msg_system_boot_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, system, boot, msg);
	}
}

void ble_evt_system_debug(HANDLER_START const struct ble_msg_system_debug_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, system, debug, msg);
	}
}

void ble_evt_system_endpoint_watermark_rx(HANDLER_START const struct ble_msg_system_endpoint_watermark_rx_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, system, endpoint_watermark_rx, msg);
	}
}

void ble_evt_system_endpoint_watermark_tx(HANDLER_START const struct ble_msg_system_endpoint_watermark_tx_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, system, endpoint_watermark_tx, msg);
	}
}

void ble_rsp_system_endpoint_set_watermarks(HANDLER_START const struct ble_msg_system_endpoint_set_watermarks_rsp_t*msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, endpoint_set_watermarks, msg);
	}
}

void ble_rsp_system_endpoint_rx(HANDLER_START const struct ble_msg_system_endpoint_rx_rsp_t*msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, system, endpoint_rx, msg);
	}
}

void ble_evt_system_script_failure(HANDLER_START const struct ble_msg_system_script_failure_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, system, script_failure, msg);
	}
}

void ble_evt_flash_ps_key(HANDLER_START const struct ble_msg_flash_ps_key_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, flash, ps_key, msg);
	}
}

void ble_evt_connection_status(HANDLER_START const struct ble_msg_connection_status_evt_t *msg)
{
#ifndef _NOPRINT
	if(msg->flags == 9)
	{
	}
	else
	{
		printf("Connection Status : %02lX [%02lX]\r\n", msg->flags, msg->connection);
	}
#endif

	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, connection, status, msg);
	}
	else
	{
		MiTokenBLE* parent = dynamic_cast<MiTokenBLE*>(sender);
		if((msg->flags & connection_connected) && (msg->flags & connection_completed))
		{
			MiTokenBLEConnection* conn = dynamic_cast<MiTokenBLEConnection*>(parent->GetConnectionWithMACAddress(msg->address));
			if(conn != nullptr)
			{
				conn->CALLBACK_Connected(msg->connection);
			}

			if(connectionCallback)
			{
				ConnectionData* connData = getConnectionData(msg->connection);
				connectionCallback(connData, msg->connection);
			}
		}

		if(((msg->flags & (connection_connected | connection_parameters_change)) == (connection_connected | connection_parameters_change)))
		{
			_ble::_cmd::_connection::_update(parent->getLowLevel(), msg->connection, msg->conn_interval, msg->conn_interval, msg->latency, msg->timeout);
			//ble_cmd_connection_disconnect(msg->connection);
			//ble_cmd_gap_connect_direct(&msg->address, msg->address_type, msg->conn_interval, msg->conn_interval, msg->timeout, msg->latency);
		}

	
		if(msg->flags&connection_connected)
		{
			//printf("#connected -> disconnect\n");
			//ble_cmd_connection_disconnect(msg->connection);
		}else
		{
			MiTokenBLEConnection* conn = dynamic_cast<MiTokenBLEConnection*>(parent->GetConnectionWithConnectionID(msg->connection));
			if(conn != nullptr)
			{
				conn->CALLBACK_ConnectionClosed();
			}

			ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);
			if((connData) && (connData->callbacks.connectionClosed))
			{
				connData->callbacks.connectionClosed(connData);
			}
			//printf("#Not connected -> Scan\n");
			parent->getLowLevel()->GapDiscover(1);
			//ble_cmd_gap_discover(1);
		}
	}
	
}

void ble_evt_connection_version_ind(HANDLER_START const struct ble_msg_connection_version_ind_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, connection, version_ind, msg);
	}
}

void ble_evt_connection_feature_ind(HANDLER_START const struct ble_msg_connection_feature_ind_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, connection, feature_ind, msg);
	}
}

void ble_evt_connection_raw_rx(HANDLER_START const struct ble_msg_connection_raw_rx_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, connection, raw_rx, msg);
	}
}

void ble_evt_connection_disconnected(HANDLER_START const struct ble_msg_connection_disconnected_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, connection, disconnected, msg);
	}
	else
	{
#ifndef _NOPRINT
		printf("Connection Disconnected [%04lX]\r\n", msg->reason);
#endif

		MiTokenBLE* parent = dynamic_cast<MiTokenBLE*>(sender);
		MiTokenBLEConnection* conn = dynamic_cast<MiTokenBLEConnection*>(parent->GetConnectionWithConnectionID(msg->connection));
		if(conn)
		{
			conn->CALLBACK_ConnectionClosed();
		}
		/*
		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);
		if(connData)
		{
			if(connData->callbacks.connectionClosed)
			{
				connData->callbacks.connectionClosed(connData);
			}
		}*/

		_ble::_cmd::_connection::_get::_status(parent->getLowLevel(), 0);

	}
}

void ble_evt_attclient_indicated(HANDLER_START const struct ble_msg_attclient_indicated_evt_t *msg)
{
	V2_2InterfaceSetup;
	if (v2_2Interface != nullptr)
	{
		ProcessChain(evt, attclient, indicated, msg);
	}
}

void ble_evt_attclient_procedure_completed(HANDLER_START const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attclient, procedure_completed, msg);
	}
	else
	{
		MiTokenBLE* parent = dynamic_cast<MiTokenBLE*>(sender);

		MiTokenBLEConnection* conn = dynamic_cast<MiTokenBLEConnection*>(parent->GetConnectionWithConnectionID(msg->connection));
	
		if(conn != nullptr)
		{
			conn->CALLBACK_ProcedureCompleted(msg->chrhandle, msg->result);
		}


		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);
	
	
		if(connData)
		{
			connData->lastError = msg->result;
			if(connData->callbacks.writeAttributeFinished)
			{
				connData->callbacks.writeAttributeFinished(connData);
			}
			if(connData->callbacks.serviceInformationFinished)
			{
				connData->callbacks.serviceInformationFinished(connData);
			}
			if(connData->callbacks.serviceSearchFinished)
			{
				connData->callbacks.serviceSearchFinished(connData);
			}
			if(connData->callbacks.extendedWriteCompleted)
			{
				connData->callbacks.extendedWriteCompleted(connData);
			}
			if(connData->callbacks.longWriteSectionCompleted)
			{
				connData->callbacks.longWriteSectionCompleted(connData);
			}
			if(connData->callbacks.longReadCompleted)
			{
				connData->callbacks.longReadCompleted(connData, msg->chrhandle);
			}
		}
	}
}

void ble_evt_attclient_group_found(HANDLER_START const struct ble_msg_attclient_group_found_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attclient, group_found, msg);
	}
	else
	{
		
		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);
		if(connData)
		{
			if(connData->callbacks.serviceFound)
			{
				connData->callbacks.serviceFound(connData, msg->uuid.len, msg->uuid.data, msg->start, msg->end);
			}
		}
	}
}

void ble_evt_attclient_attribute_found(HANDLER_START const struct ble_msg_attclient_attribute_found_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attclient, attribute_found, msg);
	}	
}

void ble_evt_attclient_find_information_found(HANDLER_START const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attclient, find_information_found, msg);
	}
	else
	{
		MiTokenBLE* parent = dynamic_cast<MiTokenBLE*>(sender);
		MiTokenBLEConnection * conn = dynamic_cast<MiTokenBLEConnection*>(parent->GetConnectionWithConnectionID(msg->connection));
		if(conn != nullptr)
		{
			conn->CALLBACK_ServiceInformationFound(msg->chrhandle, msg->uuid.len, msg->uuid.data);
		}

		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);
		if(connData)
		{
			if(connData->callbacks.serviceInformationFound)
			{
				connData->callbacks.serviceInformationFound(connData, msg->chrhandle, msg->uuid.len, msg->uuid.data);
			}
		}
	}
}

void ble_evt_attclient_attribute_value(HANDLER_START const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, attclient, attribute_value, msg);
	}
	else
	{
		MiTokenBLE* parent = dynamic_cast<MiTokenBLE*>(sender);
	
		MiTokenBLEConnection* conn = dynamic_cast<MiTokenBLEConnection*>(parent->GetConnectionWithConnectionID(msg->connection));
		if(conn != nullptr)
		{
			conn->CALLBACK_HandleValue(msg->type, msg->atthandle, msg->value.len, msg->value.data);
		}

		ConnectionData* connData = ConnectionData::findNodeWithID(msg->connection, false);
		if(connData)
		{
			if(connData->callbacks.handleValue)
			{
				connData->callbacks.handleValue(connData, msg->type, msg->atthandle, msg->value.len, msg->value.data);
			}
		}
	}
	
}

void ble_evt_sm_smp_data(HANDLER_START const struct ble_msg_sm_smp_data_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, sm, smp_data, msg);
	}
}

void ble_evt_gap_scan_response(HANDLER_START const struct ble_msg_gap_scan_response_evt_t *msg)
{
	V2_2InterfaceSetup;
	if (v2_2Interface != nullptr)
	{
		ProcessChain(evt, gap, scan_response, msg);
	}
#if USE_MEM_CHECK
	MemLeak_SetLocationID(510);
#endif
	static time_t lstpck = 0;
	static uint8 lastlbyte = 0;
	
	/*
	DEBUG_LOG("In GapScanResponse, PacketType = %d, RSSI = %d, DataLen = %d\r\n", msg->packet_type, msg->rssi, msg->data.len);
	*/

	PollData** pRootNode = nullptr;

#ifndef MiTokenBLE_ClasslessMode
	MiTokenBLE* parent = dynamic_cast<MiTokenBLE*>(sender);
	pRootNode = parent->GetPRootPoll();
#endif


	if((msg->packet_type & (~(0x02 | 0x04))) == 0x00) //allows through 0, 2 and 4 bits.
	{
		//PollData::GotRSSILessPoll((unsigned char*)msg->sender.addr, msg->data.len, (void*)msg->data.data);
		PollData::GotPoll((unsigned char*)msg->sender.addr, msg->rssi, msg->data.len, (void*)msg->data.data, parent, pRootNode);
	}
	/*
	if(msg->packet_type == 4)
	{
		PollData::GotPoll((unsigned char*)msg->sender.addr, msg->rssi, msg->data.len, (void*)msg->data.data, parent, pRootNode);
	}
	*/

	else if((msg->packet_type != 4) && ((msg->sender.addr[0] == 0x8C) || (msg->sender.addr[5] == 0x8C)))
	{
		if(true)
		{
			return;
		}
		char buf[80];
		time_t now = time(0);
		struct tm tstruct;
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
		printf("%s\r\n", buf);

		time_t diff = now - lstpck;
		printf("Delta Time = %d\r\n", diff);
		lstpck = now;
		printf("Message 0\r\n");
		//address
		printf("Address : " );
		for(int i  = 0 ; i < 6 ; ++i) { printf("%02lX%s", msg->sender.addr[i], (i == 5 ? "\r\n" : ":")); }
		//data
		printf("Data : [%d] ", msg->data.len);
		for(uint8 i = 0 ; i < msg->data.len ; ++i) { printf("%02lX%s", msg->data.data[i], (i == (msg->data.len - 1) ? "\r\n" : " ")); }

		printf("Last Byte : [%s] vs [%s]\r\n\r\n\r\n", ((msg->data.data[msg->data.len - 2] != lastlbyte) ? "Changed" : "Same"), (diff % 8 == 0 ? "Same" : "Changed"));
		lastlbyte = msg->data.data[msg->data.len - 2];

	}


}

void ble_evt_gap_mode_changed(HANDLER_START const struct ble_msg_gap_mode_changed_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, gap, mode_changed, msg);
	}
}

void ble_evt_hardware_io_port_status(HANDLER_START const struct ble_msg_hardware_io_port_status_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, hardware, io_port_status, msg);
	}
}

void ble_evt_hardware_soft_timer(HANDLER_START const struct ble_msg_hardware_soft_timer_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, hardware, soft_timer, msg);
	}
}

void ble_evt_hardware_adc_result(HANDLER_START const struct ble_msg_hardware_adc_result_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, hardware, adc_result, msg);
	}
}


/**Reset system**/
void ble_rsp_dfu_reset(HANDLER_START const void *nul)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, dfu, reset, nul);
	}
}

/**set address for flashing**/
void ble_rsp_dfu_flash_set_address(HANDLER_START const struct ble_msg_dfu_flash_set_address_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, dfu, flash_set_address, msg);
	}
}

/**Upload binary for flashing. Address will be updated automatically.**/
void ble_rsp_dfu_flash_upload(HANDLER_START const struct ble_msg_dfu_flash_upload_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, dfu, flash_upload, msg);
	}
}
/**Uploading is finished.**/
void ble_rsp_dfu_flash_upload_finish(HANDLER_START const struct ble_msg_dfu_flash_upload_finish_rsp_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(rsp, dfu, flash_upload_finish, msg);
	}
}
/**Device booted up in dfu, and is ready to receive commands**/
void ble_evt_dfu_boot(HANDLER_START const struct ble_msg_dfu_boot_evt_t *msg)
{
	V2_2InterfaceSetup;
	if(v2_2Interface != nullptr)
	{
		ProcessChain(evt, dfu, boot, msg);
	}
}
