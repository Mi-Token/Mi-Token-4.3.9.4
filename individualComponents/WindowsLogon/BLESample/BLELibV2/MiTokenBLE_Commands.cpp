#include "MiTokenBLE_Commands.h"
#include "MiTokenBLE.h"
#include "MiTokenBLEConnection.h"

#define INIT_INTERFACE MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender);
#define INIT_CONN_FROM_CONNID MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithConnectionID(msg->connection));
#define INIT_CONN_FROM_MACID MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithMACAddress(msg->address));

void EVT_ATTCLIENT_ATTRIBUTE_VALUE(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_attribute_value_evt_t* msg)
{
	MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender);
	MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithConnectionID(msg->connection));
	if(conn != nullptr)
	{
		conn->CALLBACK_HandleValue(msg->type, msg->atthandle, msg->value.len, msg->value.data);
		conn->TriggerSyncCheck();
	}
}

void RSP_ATTCLIENT_PREPARE_WRITE(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_prepare_write_rsp_t* msg)
{
	//MiTokenBLEConnection* conn = ((MiTokenBLEV2_2*)sender)->GetConnectionWithConnectionID(msg->connection);

	//This will be running at least the v2_2 interface
	MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender);
	MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithConnectionID(msg->connection));
	if(conn != nullptr)
	{
		conn->CALLBACK_PrepareWriteResponse(msg->result);
	}
}

void EVT_ATTCLIENT_PROCEDURE_COMPLETED(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_procedure_completed_evt_t* msg)
{
	MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender);
	MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithConnectionID(msg->connection));
	if(conn != nullptr)
	{
		conn->CALLBACK_ProcedureCompleted(msg->chrhandle, msg->result);
		conn->TriggerSyncCheck();
	}
}



void EVT_CONNECTION_STATUS(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_status_evt_t* msg)
{
	MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender);
	if((msg->flags & connection_connected) && (msg->flags & connection_completed))
	{
		v2_2Interface->forceConnectionUpdateRequired();
		MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithMACAddress(msg->address));
		if(conn != nullptr)
		{
			conn->CALLBACK_Connected(msg->connection);

			conn->TriggerSyncCheck();
		}
	}

	if(((msg->flags & (connection_connected | connection_parameters_change)) == (connection_connected | connection_parameters_change)))
	{
		if(v2_2Interface->connectionUpdateRequired(msg->conn_interval, msg->conn_interval, msg->latency, msg->timeout, false))
		{
			_ble::_cmd::_connection::_update(v2_2Interface->getLowLevel(), msg->connection, msg->conn_interval, msg->conn_interval, msg->latency, msg->timeout);
		}
		//ble_cmd_connection_disconnect(msg->connection);
		//ble_cmd_gap_connect_direct(&msg->address, msg->address_type, msg->conn_interval, msg->conn_interval, msg->timeout, msg->latency);
	}

	
	if(msg->flags&connection_connected)
	{
		//printf("#connected -> disconnect\n");
		//ble_cmd_connection_disconnect(msg->connection);
	}else
	{
		MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithConnectionID(msg->connection));
		if(conn != nullptr)
		{
			conn->CALLBACK_ConnectionClosed();

			conn->TriggerSyncCheck();
		}
		
		//printf("#Not connected -> Scan\n");
		v2_2Interface->getLowLevel()->GapDiscover(1);
		//ble_cmd_gap_discover(1);
	}
}


void EVT_CONNECTION_DISCONNECTED(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_disconnected_evt_t* msg)
{
	MiTokenBLEV2_2* v2_2Interface = dynamic_cast<MiTokenBLEV2_2*>(sender);
	
	MiTokenBLEConnectionV2_2* conn = dynamic_cast<MiTokenBLEConnectionV2_2*>(v2_2Interface->GetConnectionWithConnectionID(msg->connection));
	if(conn)
	{
		conn->CALLBACK_ConnectionClosed();
		conn->TriggerSyncCheck();

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

	_ble::_cmd::_connection::_get::_status(v2_2Interface->getLowLevel(), 0);
}

void RSP_CONNECTION_GET_RSSI(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_get_rssi_rsp_t* msg)
{
	INIT_INTERFACE;
	INIT_CONN_FROM_CONNID;
	if(conn != nullptr)
	{
		conn->CALLBACK_RSSIValue(msg->rssi);
	}
}
void EVT_ATTCLIENT_GROUP_FOUND(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_group_found_evt_t* msg)
{
	INIT_INTERFACE;
	INIT_CONN_FROM_CONNID;
	if(conn != nullptr)
	{
		conn->CALLBACK_ServiceFound(msg->uuid.len, msg->uuid.data, msg->start, msg->end);
	}

}
void EVT_ATTCLIENT_FIND_INFORMATION_FOUND(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_find_information_found_evt_t* msg)
{
	INIT_INTERFACE;
	INIT_CONN_FROM_CONNID;

	if(conn != nullptr)
	{
		conn->CALLBACK_ServiceInformationFound(msg->chrhandle, msg->uuid.len, msg->uuid.data);
	}
}
void RSP_CONNECTION_DISCONNECT(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_disconnect_rsp_t* msg)
{
	INIT_INTERFACE;
	INIT_CONN_FROM_CONNID;

	if(conn != nullptr)
	{
		//TODO
	}
}