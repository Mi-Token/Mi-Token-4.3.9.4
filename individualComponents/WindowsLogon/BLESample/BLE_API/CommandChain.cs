using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;


//AUTOMATICALLY GENERATED FILE. DO NOT CHANGE!

//File last generated at 2015-01-20 16:54:17

namespace BLE_API
{
	public partial class BLE
	{
		public partial class CommandChain
		{
			[UnmanagedFunctionPointer(CallingConvention.StdCall)]
			public delegate void CommandChainCall(IntPtr sender, IntPtr data);

			internal static int pointerInitialOffset = -1;

			public struct RSP
			{
				public struct SYSTEM 
				{
					public CommandChainCall hello, address_get, reg_write, reg_read, get_counters, get_connections, read_memory, get_info, endpoint_tx, whitelist_append, whitelist_remove, whitelist_clear, endpoint_rx, endpoint_set_watermarks;
				}
				public SYSTEM system;

				public struct FLASH 
				{
					public CommandChainCall ps_defrag, ps_dump, ps_erase_all, ps_save, ps_load, ps_erase, erase_page, write_words;
				}
				public FLASH flash;

				public struct ATTRIBUTES 
				{
					public CommandChainCall write, read, read_type, user_read_response, user_write_response;
				}
				public ATTRIBUTES attributes;

				public struct CONNECTION 
				{
					public CommandChainCall disconnect, get_rssi, update, version_update, channel_map_get, channel_map_set, features_get, get_status, raw_tx;
				}
				public CONNECTION connection;

				public struct ATTCLIENT 
				{
					public CommandChainCall find_by_type_value, read_by_group_type, read_by_type, find_information, read_by_handle, attribute_write, write_command, indicate_confirm, read_long, prepare_write, execute_write, read_multiple;
				}
				public ATTCLIENT attclient;

				public struct SM 
				{
					public CommandChainCall encrypt_start, set_bondable_mode, delete_bonding, set_parameters, passkey_entry, get_bonds, set_oob_data;
				}
				public SM sm;

				public struct GAP 
				{
					public CommandChainCall set_privacy_flags, set_mode, discover, connect_direct, end_procedure, connect_selective, set_filtering, set_scan_parameters, set_adv_parameters, set_adv_data, set_directed_connectable_mode;
				}
				public GAP gap;

				public struct HARDWARE 
				{
					public CommandChainCall io_port_config_irq, set_soft_timer, adc_read, io_port_config_direction, io_port_config_function, io_port_config_pull, io_port_write, io_port_read, spi_config, spi_transfer, i2c_read, i2c_write, set_txpower, timer_comparator;
				}
				public HARDWARE hardware;

				public struct TEST 
				{
					public CommandChainCall phy_tx, phy_rx, phy_end, phy_reset, get_channel_map, debug;
				}
				public TEST test;

				public struct DFU 
				{
					public CommandChainCall reset, flash_set_address, flash_upload, flash_upload_finish;
				}
				public DFU dfu;

			}
			public RSP rsp;

			public struct EVT
			{
				public struct SYSTEM 
				{
					public CommandChainCall boot, debug, endpoint_watermark_rx, endpoint_watermark_tx, script_failure, no_license_key;
				}
				public SYSTEM system;

				public struct FLASH 
				{
					public CommandChainCall ps_key;
				}
				public FLASH flash;

				public struct ATTRIBUTES 
				{
					public CommandChainCall value, user_read_request, status;
				}
				public ATTRIBUTES attributes;

				public struct CONNECTION 
				{
					public CommandChainCall status, version_ind, feature_ind, raw_rx, disconnected;
				}
				public CONNECTION connection;

				public struct ATTCLIENT 
				{
					public CommandChainCall indicated, procedure_completed, group_found, attribute_found, find_information_found, attribute_value, read_multiple_response;
				}
				public ATTCLIENT attclient;

				public struct SM 
				{
					public CommandChainCall smp_data, bonding_fail, passkey_display, passkey_request, bond_status;
				}
				public SM sm;

				public struct GAP 
				{
					public CommandChainCall scan_response, mode_changed;
				}
				public GAP gap;

				public struct HARDWARE 
				{
					public CommandChainCall io_port_status, soft_timer, adc_result;
				}
				public HARDWARE hardware;

				public struct DFU 
				{
					public CommandChainCall boot;
				}
				public DFU dfu;

			}
			public EVT evt;



			internal struct PointerLocations
			{
				internal const int evt_system_boot = 0;
				internal const int evt_system_debug = 1;
				internal const int evt_system_endpoint_watermark_rx = 2;
				internal const int evt_system_endpoint_watermark_tx = 3;
				internal const int evt_system_script_failure = 4;
				internal const int evt_system_no_license_key = 5;
				internal const int evt_flash_ps_key = 6;
				internal const int evt_attributes_value = 7;
				internal const int evt_attributes_user_read_request = 8;
				internal const int evt_attributes_status = 9;
				internal const int evt_connection_status = 10;
				internal const int evt_connection_version_ind = 11;
				internal const int evt_connection_feature_ind = 12;
				internal const int evt_connection_raw_rx = 13;
				internal const int evt_connection_disconnected = 14;
				internal const int evt_attclient_indicated = 15;
				internal const int evt_attclient_procedure_completed = 16;
				internal const int evt_attclient_group_found = 17;
				internal const int evt_attclient_attribute_found = 18;
				internal const int evt_attclient_find_information_found = 19;
				internal const int evt_attclient_attribute_value = 20;
				internal const int evt_attclient_read_multiple_response = 21;
				internal const int evt_sm_smp_data = 22;
				internal const int evt_sm_bonding_fail = 23;
				internal const int evt_sm_passkey_display = 24;
				internal const int evt_sm_passkey_request = 25;
				internal const int evt_sm_bond_status = 26;
				internal const int evt_gap_scan_response = 27;
				internal const int evt_gap_mode_changed = 28;
				internal const int evt_hardware_io_port_status = 29;
				internal const int evt_hardware_soft_timer = 30;
				internal const int evt_hardware_adc_result = 31;
				internal const int evt_dfu_boot = 32;
				internal const int rsp_system_hello = 33;
				internal const int rsp_system_address_get = 34;
				internal const int rsp_system_reg_write = 35;
				internal const int rsp_system_reg_read = 36;
				internal const int rsp_system_get_counters = 37;
				internal const int rsp_system_get_connections = 38;
				internal const int rsp_system_read_memory = 39;
				internal const int rsp_system_get_info = 40;
				internal const int rsp_system_endpoint_tx = 41;
				internal const int rsp_system_whitelist_append = 42;
				internal const int rsp_system_whitelist_remove = 43;
				internal const int rsp_system_whitelist_clear = 44;
				internal const int rsp_system_endpoint_rx = 45;
				internal const int rsp_system_endpoint_set_watermarks = 46;
				internal const int rsp_flash_ps_defrag = 47;
				internal const int rsp_flash_ps_dump = 48;
				internal const int rsp_flash_ps_erase_all = 49;
				internal const int rsp_flash_ps_save = 50;
				internal const int rsp_flash_ps_load = 51;
				internal const int rsp_flash_ps_erase = 52;
				internal const int rsp_flash_erase_page = 53;
				internal const int rsp_flash_write_words = 54;
				internal const int rsp_attributes_write = 55;
				internal const int rsp_attributes_read = 56;
				internal const int rsp_attributes_read_type = 57;
				internal const int rsp_attributes_user_read_response = 58;
				internal const int rsp_attributes_user_write_response = 59;
				internal const int rsp_connection_disconnect = 60;
				internal const int rsp_connection_get_rssi = 61;
				internal const int rsp_connection_update = 62;
				internal const int rsp_connection_version_update = 63;
				internal const int rsp_connection_channel_map_get = 64;
				internal const int rsp_connection_channel_map_set = 65;
				internal const int rsp_connection_features_get = 66;
				internal const int rsp_connection_get_status = 67;
				internal const int rsp_connection_raw_tx = 68;
				internal const int rsp_attclient_find_by_type_value = 69;
				internal const int rsp_attclient_read_by_group_type = 70;
				internal const int rsp_attclient_read_by_type = 71;
				internal const int rsp_attclient_find_information = 72;
				internal const int rsp_attclient_read_by_handle = 73;
				internal const int rsp_attclient_attribute_write = 74;
				internal const int rsp_attclient_write_command = 75;
				internal const int rsp_attclient_indicate_confirm = 76;
				internal const int rsp_attclient_read_long = 77;
				internal const int rsp_attclient_prepare_write = 78;
				internal const int rsp_attclient_execute_write = 79;
				internal const int rsp_attclient_read_multiple = 80;
				internal const int rsp_sm_encrypt_start = 81;
				internal const int rsp_sm_set_bondable_mode = 82;
				internal const int rsp_sm_delete_bonding = 83;
				internal const int rsp_sm_set_parameters = 84;
				internal const int rsp_sm_passkey_entry = 85;
				internal const int rsp_sm_get_bonds = 86;
				internal const int rsp_sm_set_oob_data = 87;
				internal const int rsp_gap_set_privacy_flags = 88;
				internal const int rsp_gap_set_mode = 89;
				internal const int rsp_gap_discover = 90;
				internal const int rsp_gap_connect_direct = 91;
				internal const int rsp_gap_end_procedure = 92;
				internal const int rsp_gap_connect_selective = 93;
				internal const int rsp_gap_set_filtering = 94;
				internal const int rsp_gap_set_scan_parameters = 95;
				internal const int rsp_gap_set_adv_parameters = 96;
				internal const int rsp_gap_set_adv_data = 97;
				internal const int rsp_gap_set_directed_connectable_mode = 98;
				internal const int rsp_hardware_io_port_config_irq = 99;
				internal const int rsp_hardware_set_soft_timer = 100;
				internal const int rsp_hardware_adc_read = 101;
				internal const int rsp_hardware_io_port_config_direction = 102;
				internal const int rsp_hardware_io_port_config_function = 103;
				internal const int rsp_hardware_io_port_config_pull = 104;
				internal const int rsp_hardware_io_port_write = 105;
				internal const int rsp_hardware_io_port_read = 106;
				internal const int rsp_hardware_spi_config = 107;
				internal const int rsp_hardware_spi_transfer = 108;
				internal const int rsp_hardware_i2c_read = 109;
				internal const int rsp_hardware_i2c_write = 110;
				internal const int rsp_hardware_set_txpower = 111;
				internal const int rsp_hardware_timer_comparator = 112;
				internal const int rsp_test_phy_tx = 113;
				internal const int rsp_test_phy_rx = 114;
				internal const int rsp_test_phy_end = 115;
				internal const int rsp_test_phy_reset = 116;
				internal const int rsp_test_get_channel_map = 117;
				internal const int rsp_test_debug = 118;
				internal const int rsp_dfu_reset = 119;
				internal const int rsp_dfu_flash_set_address = 120;
				internal const int rsp_dfu_flash_upload = 121;
				internal const int rsp_dfu_flash_upload_finish = 122;
			}

			internal static void setChainPointerValues(IntPtr __ipChain, CommandChain __chain)
			{
				if(pointerInitialOffset == -1)
				{
					Inner.VerifyDllVersion(BLE.V2_2.MinVersion_ChainOffset);
					pointerInitialOffset = DLLFunctions.V2_2.MiTokenBLE.MiTokenBLEV2_2_Inner_GetChainPointerOffset(__ipChain);
				}

				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_system_boot, __chain.evt.system.boot);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_system_debug, __chain.evt.system.debug);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_system_endpoint_watermark_rx, __chain.evt.system.endpoint_watermark_rx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_system_endpoint_watermark_tx, __chain.evt.system.endpoint_watermark_tx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_system_script_failure, __chain.evt.system.script_failure);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_system_no_license_key, __chain.evt.system.no_license_key);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_flash_ps_key, __chain.evt.flash.ps_key);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attributes_value, __chain.evt.attributes.value);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attributes_user_read_request, __chain.evt.attributes.user_read_request);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attributes_status, __chain.evt.attributes.status);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_connection_status, __chain.evt.connection.status);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_connection_version_ind, __chain.evt.connection.version_ind);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_connection_feature_ind, __chain.evt.connection.feature_ind);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_connection_raw_rx, __chain.evt.connection.raw_rx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_connection_disconnected, __chain.evt.connection.disconnected);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attclient_indicated, __chain.evt.attclient.indicated);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attclient_procedure_completed, __chain.evt.attclient.procedure_completed);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attclient_group_found, __chain.evt.attclient.group_found);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attclient_attribute_found, __chain.evt.attclient.attribute_found);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attclient_find_information_found, __chain.evt.attclient.find_information_found);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attclient_attribute_value, __chain.evt.attclient.attribute_value);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_attclient_read_multiple_response, __chain.evt.attclient.read_multiple_response);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_sm_smp_data, __chain.evt.sm.smp_data);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_sm_bonding_fail, __chain.evt.sm.bonding_fail);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_sm_passkey_display, __chain.evt.sm.passkey_display);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_sm_passkey_request, __chain.evt.sm.passkey_request);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_sm_bond_status, __chain.evt.sm.bond_status);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_gap_scan_response, __chain.evt.gap.scan_response);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_gap_mode_changed, __chain.evt.gap.mode_changed);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_hardware_io_port_status, __chain.evt.hardware.io_port_status);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_hardware_soft_timer, __chain.evt.hardware.soft_timer);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_hardware_adc_result, __chain.evt.hardware.adc_result);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.evt_dfu_boot, __chain.evt.dfu.boot);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_hello, __chain.rsp.system.hello);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_address_get, __chain.rsp.system.address_get);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_reg_write, __chain.rsp.system.reg_write);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_reg_read, __chain.rsp.system.reg_read);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_get_counters, __chain.rsp.system.get_counters);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_get_connections, __chain.rsp.system.get_connections);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_read_memory, __chain.rsp.system.read_memory);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_get_info, __chain.rsp.system.get_info);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_endpoint_tx, __chain.rsp.system.endpoint_tx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_whitelist_append, __chain.rsp.system.whitelist_append);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_whitelist_remove, __chain.rsp.system.whitelist_remove);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_whitelist_clear, __chain.rsp.system.whitelist_clear);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_endpoint_rx, __chain.rsp.system.endpoint_rx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_system_endpoint_set_watermarks, __chain.rsp.system.endpoint_set_watermarks);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_ps_defrag, __chain.rsp.flash.ps_defrag);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_ps_dump, __chain.rsp.flash.ps_dump);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_ps_erase_all, __chain.rsp.flash.ps_erase_all);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_ps_save, __chain.rsp.flash.ps_save);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_ps_load, __chain.rsp.flash.ps_load);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_ps_erase, __chain.rsp.flash.ps_erase);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_erase_page, __chain.rsp.flash.erase_page);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_flash_write_words, __chain.rsp.flash.write_words);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attributes_write, __chain.rsp.attributes.write);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attributes_read, __chain.rsp.attributes.read);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attributes_read_type, __chain.rsp.attributes.read_type);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attributes_user_read_response, __chain.rsp.attributes.user_read_response);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attributes_user_write_response, __chain.rsp.attributes.user_write_response);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_disconnect, __chain.rsp.connection.disconnect);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_get_rssi, __chain.rsp.connection.get_rssi);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_update, __chain.rsp.connection.update);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_version_update, __chain.rsp.connection.version_update);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_channel_map_get, __chain.rsp.connection.channel_map_get);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_channel_map_set, __chain.rsp.connection.channel_map_set);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_features_get, __chain.rsp.connection.features_get);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_get_status, __chain.rsp.connection.get_status);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_connection_raw_tx, __chain.rsp.connection.raw_tx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_find_by_type_value, __chain.rsp.attclient.find_by_type_value);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_read_by_group_type, __chain.rsp.attclient.read_by_group_type);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_read_by_type, __chain.rsp.attclient.read_by_type);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_find_information, __chain.rsp.attclient.find_information);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_read_by_handle, __chain.rsp.attclient.read_by_handle);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_attribute_write, __chain.rsp.attclient.attribute_write);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_write_command, __chain.rsp.attclient.write_command);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_indicate_confirm, __chain.rsp.attclient.indicate_confirm);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_read_long, __chain.rsp.attclient.read_long);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_prepare_write, __chain.rsp.attclient.prepare_write);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_execute_write, __chain.rsp.attclient.execute_write);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_attclient_read_multiple, __chain.rsp.attclient.read_multiple);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_sm_encrypt_start, __chain.rsp.sm.encrypt_start);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_sm_set_bondable_mode, __chain.rsp.sm.set_bondable_mode);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_sm_delete_bonding, __chain.rsp.sm.delete_bonding);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_sm_set_parameters, __chain.rsp.sm.set_parameters);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_sm_passkey_entry, __chain.rsp.sm.passkey_entry);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_sm_get_bonds, __chain.rsp.sm.get_bonds);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_sm_set_oob_data, __chain.rsp.sm.set_oob_data);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_set_privacy_flags, __chain.rsp.gap.set_privacy_flags);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_set_mode, __chain.rsp.gap.set_mode);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_discover, __chain.rsp.gap.discover);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_connect_direct, __chain.rsp.gap.connect_direct);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_end_procedure, __chain.rsp.gap.end_procedure);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_connect_selective, __chain.rsp.gap.connect_selective);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_set_filtering, __chain.rsp.gap.set_filtering);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_set_scan_parameters, __chain.rsp.gap.set_scan_parameters);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_set_adv_parameters, __chain.rsp.gap.set_adv_parameters);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_set_adv_data, __chain.rsp.gap.set_adv_data);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_gap_set_directed_connectable_mode, __chain.rsp.gap.set_directed_connectable_mode);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_io_port_config_irq, __chain.rsp.hardware.io_port_config_irq);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_set_soft_timer, __chain.rsp.hardware.set_soft_timer);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_adc_read, __chain.rsp.hardware.adc_read);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_io_port_config_direction, __chain.rsp.hardware.io_port_config_direction);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_io_port_config_function, __chain.rsp.hardware.io_port_config_function);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_io_port_config_pull, __chain.rsp.hardware.io_port_config_pull);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_io_port_write, __chain.rsp.hardware.io_port_write);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_io_port_read, __chain.rsp.hardware.io_port_read);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_spi_config, __chain.rsp.hardware.spi_config);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_spi_transfer, __chain.rsp.hardware.spi_transfer);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_i2c_read, __chain.rsp.hardware.i2c_read);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_i2c_write, __chain.rsp.hardware.i2c_write);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_set_txpower, __chain.rsp.hardware.set_txpower);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_hardware_timer_comparator, __chain.rsp.hardware.timer_comparator);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_test_phy_tx, __chain.rsp.test.phy_tx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_test_phy_rx, __chain.rsp.test.phy_rx);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_test_phy_end, __chain.rsp.test.phy_end);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_test_phy_reset, __chain.rsp.test.phy_reset);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_test_get_channel_map, __chain.rsp.test.get_channel_map);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_test_debug, __chain.rsp.test.debug);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_dfu_reset, __chain.rsp.dfu.reset);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_dfu_flash_set_address, __chain.rsp.dfu.flash_set_address);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_dfu_flash_upload, __chain.rsp.dfu.flash_upload);
				PointerHandling.writeCallbackToChainPtr(__ipChain, pointerInitialOffset, PointerLocations.rsp_dfu_flash_upload_finish, __chain.rsp.dfu.flash_upload_finish);
			}

		}
	}
}
