#pragma once

#ifndef _MITOKEN_BLE_COMMANDS_H_
#define _MITOKEN_BLE_COMMANDS_H_

#include "MiTokenBLE.h"
#include "BLE_Command_Chain_Link.h"

#include "cmd_def.h"


void EVT_ATTCLIENT_ATTRIBUTE_VALUE(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_attribute_value_evt_t* msg);
void EVT_ATTCLIENT_FIND_INFORMATION_FOUND(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_find_information_found_evt_t* msg);
void EVT_ATTCLIENT_GROUP_FOUND(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_group_found_evt_t* msg);
void EVT_ATTCLIENT_PROCEDURE_COMPLETED(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_procedure_completed_evt_t* msg);

void EVT_CONNECTION_STATUS(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_status_evt_t* msg);
void EVT_CONNECTION_DISCONNECTED(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_disconnected_evt_t* msg);


void RSP_ATTCLIENT_PREPARE_WRITE(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_prepare_write_rsp_t* msg);

void RSP_CONNECTION_DISCONNECT(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_disconnect_rsp_t* msg);
void RSP_CONNECTION_GET_RSSI(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_get_rssi_rsp_t* msg);




#endif