using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BLE_API;


namespace BLE_API_Debugger
{
	public class Debugger
	{
		public delegate void Debugger_SendDebugMessage(IntPtr sender, string message);
		public static event Debugger_SendDebugMessage Debugger_SendDebugMessageCallback;

		internal static void _debugSendMessage(IntPtr sender, string message)
		{
			if(Debugger_SendDebugMessageCallback != null)
			{
				Debugger_SendDebugMessageCallback(sender, message);
			}
		}

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_address_get = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ADDRESS_GET msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ADDRESS_GET(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_reg_write = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.REG_WRITE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.REG_WRITE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_reg_read = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.REG_READ msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.REG_READ(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_get_counters = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.GET_COUNTERS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.GET_COUNTERS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_get_connections = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.GET_CONNECTIONS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.GET_CONNECTIONS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_read_memory = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.READ_MEMORY msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.READ_MEMORY(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_get_info = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.GET_INFO msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.GET_INFO(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_endpoint_tx = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ENDPOINT_TX msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ENDPOINT_TX(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_whitelist_append = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.WHITELIST_APPEND msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.WHITELIST_APPEND(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_whitelist_remove = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.WHITELIST_REMOVE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.WHITELIST_REMOVE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_endpoint_rx = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ENDPOINT_RX msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ENDPOINT_RX(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_system_endpoint_set_watermarks = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ENDPOINT_SET_WATERMARKS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SYSTEM.ENDPOINT_SET_WATERMARKS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_system_boot = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.BOOT msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.BOOT(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_system_debug = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.DEBUG msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.DEBUG(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_system_endpoint_watermark_rx = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.ENDPOINT_WATERMARK_RX msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.ENDPOINT_WATERMARK_RX(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_system_endpoint_watermark_tx = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.ENDPOINT_WATERMARK_TX msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.ENDPOINT_WATERMARK_TX(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_system_script_failure = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.SCRIPT_FAILURE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SYSTEM.SCRIPT_FAILURE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_flash_ps_save = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.FLASH.PS_SAVE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.FLASH.PS_SAVE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_flash_ps_load = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.FLASH.PS_LOAD msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.FLASH.PS_LOAD(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_flash_erase_page = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.FLASH.ERASE_PAGE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.FLASH.ERASE_PAGE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_flash_ps_key = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.FLASH.PS_KEY msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.FLASH.PS_KEY(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attributes_write = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTRIBUTES.WRITE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTRIBUTES.WRITE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attributes_read = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTRIBUTES.READ msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTRIBUTES.READ(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attributes_read_type = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTRIBUTES.READ_TYPE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTRIBUTES.READ_TYPE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attributes_value = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTRIBUTES.VALUE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTRIBUTES.VALUE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attributes_user_read_request = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTRIBUTES.USER_READ_REQUEST msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTRIBUTES.USER_READ_REQUEST(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attributes_status = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTRIBUTES.STATUS msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTRIBUTES.STATUS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_disconnect = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.DISCONNECT msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.DISCONNECT(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_get_rssi = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.GET_RSSI msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.GET_RSSI(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_update = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.UPDATE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.UPDATE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_version_update = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.VERSION_UPDATE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.VERSION_UPDATE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_channel_map_get = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.CHANNEL_MAP_GET msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.CHANNEL_MAP_GET(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_channel_map_set = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.CHANNEL_MAP_SET msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.CHANNEL_MAP_SET(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_features_get = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.FEATURES_GET msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.FEATURES_GET(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_get_status = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.GET_STATUS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.GET_STATUS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_connection_raw_tx = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.RAW_TX msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.RAW_TX(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_connection_status = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.STATUS msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.STATUS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_connection_version_ind = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.VERSION_IND msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.VERSION_IND(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_connection_feature_ind = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.FEATURE_IND msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.FEATURE_IND(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_connection_raw_rx = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.RAW_RX msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.RAW_RX(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_connection_disconnected = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.DISCONNECTED msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.DISCONNECTED(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_find_by_type_value = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.FIND_BY_TYPE_VALUE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.FIND_BY_TYPE_VALUE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_read_by_group_type = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_BY_GROUP_TYPE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_BY_GROUP_TYPE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_read_by_type = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_BY_TYPE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_BY_TYPE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_find_information = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.FIND_INFORMATION msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.FIND_INFORMATION(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_read_by_handle = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_BY_HANDLE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_BY_HANDLE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_attribute_write = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.ATTRIBUTE_WRITE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.ATTRIBUTE_WRITE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_write_command = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.WRITE_COMMAND msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.WRITE_COMMAND(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_indicate_confirm = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.INDICATE_CONFIRM msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.INDICATE_CONFIRM(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_read_long = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_LONG msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_LONG(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_prepare_write = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.PREPARE_WRITE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.PREPARE_WRITE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_execute_write = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.EXECUTE_WRITE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.EXECUTE_WRITE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_attclient_read_multiple = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_MULTIPLE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.ATTCLIENT.READ_MULTIPLE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attclient_indicated = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.INDICATED msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.INDICATED(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attclient_procedure_completed = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.PROCEDURE_COMPLETED msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.PROCEDURE_COMPLETED(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attclient_group_found = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.GROUP_FOUND msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.GROUP_FOUND(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attclient_attribute_found = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_FOUND msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_FOUND(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attclient_find_information_found = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.FIND_INFORMATION_FOUND msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.FIND_INFORMATION_FOUND(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attclient_attribute_value = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_VALUE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.ATTRIBUTE_VALUE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_attclient_read_multiple_response = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.READ_MULTIPLE_RESPONSE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.READ_MULTIPLE_RESPONSE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_sm_encrypt_start = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SM.ENCRYPT_START msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SM.ENCRYPT_START(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_sm_delete_bonding = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SM.DELETE_BONDING msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SM.DELETE_BONDING(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_sm_passkey_entry = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SM.PASSKEY_ENTRY msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SM.PASSKEY_ENTRY(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_sm_get_bonds = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.SM.GET_BONDS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.SM.GET_BONDS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_sm_smp_data = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SM.SMP_DATA msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SM.SMP_DATA(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_sm_bonding_fail = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SM.BONDING_FAIL msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SM.BONDING_FAIL(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_sm_passkey_display = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SM.PASSKEY_DISPLAY msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SM.PASSKEY_DISPLAY(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_sm_passkey_request = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SM.PASSKEY_REQUEST msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SM.PASSKEY_REQUEST(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_sm_bond_status = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.SM.BOND_STATUS msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.SM.BOND_STATUS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_set_mode = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_MODE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_MODE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_discover = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.DISCOVER msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.DISCOVER(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_connect_direct = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.CONNECT_DIRECT msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.CONNECT_DIRECT(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_end_procedure = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.END_PROCEDURE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.END_PROCEDURE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_connect_selective = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.CONNECT_SELECTIVE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.CONNECT_SELECTIVE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_set_filtering = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_FILTERING msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_FILTERING(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_set_scan_parameters = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_SCAN_PARAMETERS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_SCAN_PARAMETERS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_set_adv_parameters = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_ADV_PARAMETERS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_ADV_PARAMETERS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_set_adv_data = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_ADV_DATA msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_ADV_DATA(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_gap_set_directed_connectable_mode = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_DIRECTED_CONNECTABLE_MODE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.GAP.SET_DIRECTED_CONNECTABLE_MODE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_gap_scan_response = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.SCAN_RESPONSE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_gap_mode_changed = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.MODE_CHANGED msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.GAP.MODE_CHANGED(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_io_port_config_irq = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_IRQ msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_IRQ(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_set_soft_timer = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.SET_SOFT_TIMER msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.SET_SOFT_TIMER(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_adc_read = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.ADC_READ msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.ADC_READ(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_io_port_config_direction = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_DIRECTION msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_DIRECTION(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_io_port_config_function = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_FUNCTION msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_FUNCTION(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_io_port_config_pull = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_PULL msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_CONFIG_PULL(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_io_port_write = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_WRITE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_WRITE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_io_port_read = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_READ msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.IO_PORT_READ(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_spi_config = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.SPI_CONFIG msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.SPI_CONFIG(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_spi_transfer = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.SPI_TRANSFER msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.SPI_TRANSFER(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_i2c_read = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.I2C_READ msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.I2C_READ(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_i2c_write = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.I2C_WRITE msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.I2C_WRITE(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_hardware_timer_comparator = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.TIMER_COMPARATOR msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.HARDWARE.TIMER_COMPARATOR(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_hardware_io_port_status = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.HARDWARE.IO_PORT_STATUS msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.HARDWARE.IO_PORT_STATUS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_hardware_soft_timer = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.HARDWARE.SOFT_TIMER msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.HARDWARE.SOFT_TIMER(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_hardware_adc_result = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.HARDWARE.ADC_RESULT msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.HARDWARE.ADC_RESULT(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_test_phy_end = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.TEST.PHY_END msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.TEST.PHY_END(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_test_get_channel_map = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.TEST.GET_CHANNEL_MAP msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.TEST.GET_CHANNEL_MAP(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_test_debug = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.TEST.DEBUG msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.TEST.DEBUG(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_dfu_flash_set_address = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.DFU.FLASH_SET_ADDRESS msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.DFU.FLASH_SET_ADDRESS(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_dfu_flash_upload = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.DFU.FLASH_UPLOAD msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.DFU.FLASH_UPLOAD(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_rsp_dfu_flash_upload_finish = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.RSP.DFU.FLASH_UPLOAD_FINISH msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.DFU.FLASH_UPLOAD_FINISH(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static BLE.CommandChain.CommandChainCall ccc_evt_dfu_boot = (IntPtr sender, IntPtr data) =>
			{
				BLE.CommandChain.CommandChainMessageTypes.EVT.DFU.BOOT msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.DFU.BOOT(data);
				_debugSendMessage(sender, msg.__debugString);
			};

		static public void AttachDebugger(BLE.V2_2.MiTokenBLE instance)
		{
			BLE.CommandChain CC = new BLE.CommandChain();
			CC.rsp.system.address_get = ccc_rsp_system_address_get;
			CC.rsp.system.reg_write = ccc_rsp_system_reg_write;
			CC.rsp.system.reg_read = ccc_rsp_system_reg_read;
			CC.rsp.system.get_counters = ccc_rsp_system_get_counters;
			CC.rsp.system.get_connections = ccc_rsp_system_get_connections;
			CC.rsp.system.read_memory = ccc_rsp_system_read_memory;
			CC.rsp.system.get_info = ccc_rsp_system_get_info;
			CC.rsp.system.endpoint_tx = ccc_rsp_system_endpoint_tx;
			CC.rsp.system.whitelist_append = ccc_rsp_system_whitelist_append;
			CC.rsp.system.whitelist_remove = ccc_rsp_system_whitelist_remove;
			CC.rsp.system.endpoint_rx = ccc_rsp_system_endpoint_rx;
			CC.rsp.system.endpoint_set_watermarks = ccc_rsp_system_endpoint_set_watermarks;
			CC.evt.system.boot = ccc_evt_system_boot;
			CC.evt.system.debug = ccc_evt_system_debug;
			CC.evt.system.endpoint_watermark_rx = ccc_evt_system_endpoint_watermark_rx;
			CC.evt.system.endpoint_watermark_tx = ccc_evt_system_endpoint_watermark_tx;
			CC.evt.system.script_failure = ccc_evt_system_script_failure;
			CC.rsp.flash.ps_save = ccc_rsp_flash_ps_save;
			CC.rsp.flash.ps_load = ccc_rsp_flash_ps_load;
			CC.rsp.flash.erase_page = ccc_rsp_flash_erase_page;
			CC.evt.flash.ps_key = ccc_evt_flash_ps_key;
			CC.rsp.attributes.write = ccc_rsp_attributes_write;
			CC.rsp.attributes.read = ccc_rsp_attributes_read;
			CC.rsp.attributes.read_type = ccc_rsp_attributes_read_type;
			CC.evt.attributes.value = ccc_evt_attributes_value;
			CC.evt.attributes.user_read_request = ccc_evt_attributes_user_read_request;
			CC.evt.attributes.status = ccc_evt_attributes_status;
			CC.rsp.connection.disconnect = ccc_rsp_connection_disconnect;
			CC.rsp.connection.get_rssi = ccc_rsp_connection_get_rssi;
			CC.rsp.connection.update = ccc_rsp_connection_update;
			CC.rsp.connection.version_update = ccc_rsp_connection_version_update;
			CC.rsp.connection.channel_map_get = ccc_rsp_connection_channel_map_get;
			CC.rsp.connection.channel_map_set = ccc_rsp_connection_channel_map_set;
			CC.rsp.connection.features_get = ccc_rsp_connection_features_get;
			CC.rsp.connection.get_status = ccc_rsp_connection_get_status;
			CC.rsp.connection.raw_tx = ccc_rsp_connection_raw_tx;
			CC.evt.connection.status = ccc_evt_connection_status;
			CC.evt.connection.version_ind = ccc_evt_connection_version_ind;
			CC.evt.connection.feature_ind = ccc_evt_connection_feature_ind;
			CC.evt.connection.raw_rx = ccc_evt_connection_raw_rx;
			CC.evt.connection.disconnected = ccc_evt_connection_disconnected;
			CC.rsp.attclient.find_by_type_value = ccc_rsp_attclient_find_by_type_value;
			CC.rsp.attclient.read_by_group_type = ccc_rsp_attclient_read_by_group_type;
			CC.rsp.attclient.read_by_type = ccc_rsp_attclient_read_by_type;
			CC.rsp.attclient.find_information = ccc_rsp_attclient_find_information;
			CC.rsp.attclient.read_by_handle = ccc_rsp_attclient_read_by_handle;
			CC.rsp.attclient.attribute_write = ccc_rsp_attclient_attribute_write;
			CC.rsp.attclient.write_command = ccc_rsp_attclient_write_command;
			CC.rsp.attclient.indicate_confirm = ccc_rsp_attclient_indicate_confirm;
			CC.rsp.attclient.read_long = ccc_rsp_attclient_read_long;
			CC.rsp.attclient.prepare_write = ccc_rsp_attclient_prepare_write;
			CC.rsp.attclient.execute_write = ccc_rsp_attclient_execute_write;
			CC.rsp.attclient.read_multiple = ccc_rsp_attclient_read_multiple;
			CC.evt.attclient.indicated = ccc_evt_attclient_indicated;
			CC.evt.attclient.procedure_completed = ccc_evt_attclient_procedure_completed;
			CC.evt.attclient.group_found = ccc_evt_attclient_group_found;
			CC.evt.attclient.attribute_found = ccc_evt_attclient_attribute_found;
			CC.evt.attclient.find_information_found = ccc_evt_attclient_find_information_found;
			CC.evt.attclient.attribute_value = ccc_evt_attclient_attribute_value;
			CC.evt.attclient.read_multiple_response = ccc_evt_attclient_read_multiple_response;
			CC.rsp.sm.encrypt_start = ccc_rsp_sm_encrypt_start;
			CC.rsp.sm.delete_bonding = ccc_rsp_sm_delete_bonding;
			CC.rsp.sm.passkey_entry = ccc_rsp_sm_passkey_entry;
			CC.rsp.sm.get_bonds = ccc_rsp_sm_get_bonds;
			CC.evt.sm.smp_data = ccc_evt_sm_smp_data;
			CC.evt.sm.bonding_fail = ccc_evt_sm_bonding_fail;
			CC.evt.sm.passkey_display = ccc_evt_sm_passkey_display;
			CC.evt.sm.passkey_request = ccc_evt_sm_passkey_request;
			CC.evt.sm.bond_status = ccc_evt_sm_bond_status;
			CC.rsp.gap.set_mode = ccc_rsp_gap_set_mode;
			CC.rsp.gap.discover = ccc_rsp_gap_discover;
			CC.rsp.gap.connect_direct = ccc_rsp_gap_connect_direct;
			CC.rsp.gap.end_procedure = ccc_rsp_gap_end_procedure;
			CC.rsp.gap.connect_selective = ccc_rsp_gap_connect_selective;
			CC.rsp.gap.set_filtering = ccc_rsp_gap_set_filtering;
			CC.rsp.gap.set_scan_parameters = ccc_rsp_gap_set_scan_parameters;
			CC.rsp.gap.set_adv_parameters = ccc_rsp_gap_set_adv_parameters;
			CC.rsp.gap.set_adv_data = ccc_rsp_gap_set_adv_data;
			CC.rsp.gap.set_directed_connectable_mode = ccc_rsp_gap_set_directed_connectable_mode;
			CC.evt.gap.scan_response = ccc_evt_gap_scan_response;
			CC.evt.gap.mode_changed = ccc_evt_gap_mode_changed;
			CC.rsp.hardware.io_port_config_irq = ccc_rsp_hardware_io_port_config_irq;
			CC.rsp.hardware.set_soft_timer = ccc_rsp_hardware_set_soft_timer;
			CC.rsp.hardware.adc_read = ccc_rsp_hardware_adc_read;
			CC.rsp.hardware.io_port_config_direction = ccc_rsp_hardware_io_port_config_direction;
			CC.rsp.hardware.io_port_config_function = ccc_rsp_hardware_io_port_config_function;
			CC.rsp.hardware.io_port_config_pull = ccc_rsp_hardware_io_port_config_pull;
			CC.rsp.hardware.io_port_write = ccc_rsp_hardware_io_port_write;
			CC.rsp.hardware.io_port_read = ccc_rsp_hardware_io_port_read;
			CC.rsp.hardware.spi_config = ccc_rsp_hardware_spi_config;
			CC.rsp.hardware.spi_transfer = ccc_rsp_hardware_spi_transfer;
			CC.rsp.hardware.i2c_read = ccc_rsp_hardware_i2c_read;
			CC.rsp.hardware.i2c_write = ccc_rsp_hardware_i2c_write;
			CC.rsp.hardware.timer_comparator = ccc_rsp_hardware_timer_comparator;
			CC.evt.hardware.io_port_status = ccc_evt_hardware_io_port_status;
			CC.evt.hardware.soft_timer = ccc_evt_hardware_soft_timer;
			CC.evt.hardware.adc_result = ccc_evt_hardware_adc_result;
			CC.rsp.test.phy_end = ccc_rsp_test_phy_end;
			CC.rsp.test.get_channel_map = ccc_rsp_test_get_channel_map;
			CC.rsp.test.debug = ccc_rsp_test_debug;
			CC.rsp.dfu.flash_set_address = ccc_rsp_dfu_flash_set_address;
			CC.rsp.dfu.flash_upload = ccc_rsp_dfu_flash_upload;
			CC.rsp.dfu.flash_upload_finish = ccc_rsp_dfu_flash_upload_finish;
			CC.evt.dfu.boot = ccc_evt_dfu_boot;

			instance.AppendCommandChain(CC, true);
		}

	}
}
