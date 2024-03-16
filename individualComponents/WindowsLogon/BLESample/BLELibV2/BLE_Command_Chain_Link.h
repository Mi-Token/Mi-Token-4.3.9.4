#pragma once

#ifndef BLE_COMMAND_CHAIN_LINK_H_
#define BLE_COMMAND_CHAIN_LINK_H_

#include "BLE_API.h"

class IMiTokenBLE;
class IMiTokenBLEV2_2;



class BLE_COMMAND_RSP_SYSTEM
{
public:
	void (*hello)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*address_get)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_address_get_rsp_t *msg);
	void (*reg_write)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_reg_write_rsp_t *msg);
	void (*reg_read)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_reg_read_rsp_t *msg);
	void (*get_counters)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_get_counters_rsp_t *msg);
	void (*get_connections)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_get_connections_rsp_t *msg);
	void (*read_memory)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_read_memory_rsp_t *msg);
	void (*get_info)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_get_info_rsp_t *msg);
	void (*endpoint_tx)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_endpoint_tx_rsp_t *msg);
	void (*whitelist_append)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_whitelist_append_rsp_t *msg);
	void (*whitelist_remove)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_whitelist_remove_rsp_t *msg);
	void (*whitelist_clear)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*endpoint_rx)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_endpoint_rx_rsp_t *msg);
	void (*endpoint_set_watermarks)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_endpoint_set_watermarks_rsp_t *msg);
};
class BLE_COMMAND_RSP_FLASH
{
public:
	void (*ps_defrag)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*ps_dump)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*ps_erase_all)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*ps_save)(IMiTokenBLEV2_2* sender, const struct ble_msg_flash_ps_save_rsp_t *msg);
	void (*ps_load)(IMiTokenBLEV2_2* sender, const struct ble_msg_flash_ps_load_rsp_t *msg);
	void (*ps_erase)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*erase_page)(IMiTokenBLEV2_2* sender, const struct ble_msg_flash_erase_page_rsp_t *msg);
	void (*write_words)(IMiTokenBLEV2_2* sender, const void *nul);
};
class BLE_COMMAND_RSP_ATTRIBUTES
{
public:
	void (*write)(IMiTokenBLEV2_2* sender, const struct ble_msg_attributes_write_rsp_t *msg);
	void (*read)(IMiTokenBLEV2_2* sender, const struct ble_msg_attributes_read_rsp_t *msg);
	void (*read_type)(IMiTokenBLEV2_2* sender, const struct ble_msg_attributes_read_type_rsp_t *msg);
	void (*user_read_response)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*user_write_response)(IMiTokenBLEV2_2* sender, const void *nul);
};
class BLE_COMMAND_RSP_CONNECTION
{
public:
	void (*disconnect)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_disconnect_rsp_t *msg);
	void (*get_rssi)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_get_rssi_rsp_t *msg);
	void (*update)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_update_rsp_t *msg);
	void (*version_update)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_version_update_rsp_t *msg);
	void (*channel_map_get)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_channel_map_get_rsp_t *msg);
	void (*channel_map_set)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_channel_map_set_rsp_t *msg);
	void (*features_get)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_features_get_rsp_t *msg);
	void (*get_status)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_get_status_rsp_t *msg);
	void (*raw_tx)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_raw_tx_rsp_t *msg);
};
class BLE_COMMAND_RSP_ATTCLIENT
{
public:
	void (*find_by_type_value)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_find_by_type_value_rsp_t *msg);
	void (*read_by_group_type)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_read_by_group_type_rsp_t *msg);
	void (*read_by_type)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_read_by_type_rsp_t *msg);
	void (*find_information)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_find_information_rsp_t *msg);
	void (*read_by_handle)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_read_by_handle_rsp_t *msg);
	void (*attribute_write)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_attribute_write_rsp_t *msg);
	void (*write_command)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_write_command_rsp_t *msg);
	void (*indicate_confirm)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_indicate_confirm_rsp_t *msg);
	void (*read_long)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_read_long_rsp_t *msg);
	void (*prepare_write)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_prepare_write_rsp_t *msg);
	void (*execute_write)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_execute_write_rsp_t *msg);
	void (*read_multiple)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_read_multiple_rsp_t *msg);
};
class BLE_COMMAND_RSP_SM
{
public:
	void (*encrypt_start)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_encrypt_start_rsp_t *msg);
	void (*set_bondable_mode)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*delete_bonding)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_delete_bonding_rsp_t *msg);
	void (*set_parameters)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*passkey_entry)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_passkey_entry_rsp_t *msg);
	void (*get_bonds)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_get_bonds_rsp_t *msg);
	void (*set_oob_data)(IMiTokenBLEV2_2* sender, const void *nul);
};
class BLE_COMMAND_RSP_GAP
{
public:
	void (*set_privacy_flags)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*set_mode)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_set_mode_rsp_t *msg);
	void (*discover)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_discover_rsp_t *msg);
	void (*connect_direct)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_connect_direct_rsp_t *msg);
	void (*end_procedure)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_end_procedure_rsp_t *msg);
	void (*connect_selective)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_connect_selective_rsp_t *msg);
	void (*set_filtering)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_set_filtering_rsp_t *msg);
	void (*set_scan_parameters)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_set_scan_parameters_rsp_t *msg);
	void (*set_adv_parameters)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_set_adv_parameters_rsp_t *msg);
	void (*set_adv_data)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_set_adv_data_rsp_t *msg);
	void (*set_directed_connectable_mode)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_set_directed_connectable_mode_rsp_t *msg);
};
class BLE_COMMAND_RSP_HARDWARE
{
public:
	void (*io_port_config_irq)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_io_port_config_irq_rsp_t *msg);
	void (*set_soft_timer)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_set_soft_timer_rsp_t *msg);
	void (*adc_read)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_adc_read_rsp_t *msg);
	void (*io_port_config_direction)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_io_port_config_direction_rsp_t *msg);
	void (*io_port_config_function)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_io_port_config_function_rsp_t *msg);
	void (*io_port_config_pull)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_io_port_config_pull_rsp_t *msg);
	void (*io_port_write)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_io_port_write_rsp_t *msg);
	void (*io_port_read)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_io_port_read_rsp_t *msg);
	void (*spi_config)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_spi_config_rsp_t *msg);
	void (*spi_transfer)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_spi_transfer_rsp_t *msg);
	void (*i2c_read)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_i2c_read_rsp_t *msg);
	void (*i2c_write)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_i2c_write_rsp_t *msg);
	void (*set_txpower)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*timer_comparator)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_timer_comparator_rsp_t *msg);
};
class BLE_COMMAND_RSP_TEST
{
public:
	void (*phy_tx)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*phy_rx)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*phy_end)(IMiTokenBLEV2_2* sender, const struct ble_msg_test_phy_end_rsp_t *msg);
	void (*phy_reset)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*get_channel_map)(IMiTokenBLEV2_2* sender, const struct ble_msg_test_get_channel_map_rsp_t *msg);
	void (*debug)(IMiTokenBLEV2_2* sender, const struct ble_msg_test_debug_rsp_t *msg);
};
class BLE_COMMAND_RSP_DFU
{
public:
	void (*reset)(IMiTokenBLEV2_2* sender, const void *nul);
	void (*flash_set_address)(IMiTokenBLEV2_2* sender, const struct ble_msg_dfu_flash_set_address_rsp_t *msg);
	void (*flash_upload)(IMiTokenBLEV2_2* sender, const struct ble_msg_dfu_flash_upload_rsp_t *msg);
	void (*flash_upload_finish)(IMiTokenBLEV2_2* sender, const struct ble_msg_dfu_flash_upload_finish_rsp_t *msg);
};


class BLE_COMMAND_EVT_SYSTEM
{
public:
	void (*boot)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_boot_evt_t *msg);
	void (*debug)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_debug_evt_t *msg);
	void (*endpoint_watermark_rx)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_endpoint_watermark_rx_evt_t *msg);
	void (*endpoint_watermark_tx)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_endpoint_watermark_tx_evt_t *msg);
	void (*script_failure)(IMiTokenBLEV2_2* sender, const struct ble_msg_system_script_failure_evt_t *msg);
	void (*no_license_key)(IMiTokenBLEV2_2* sender, const void *nul);
};
class BLE_COMMAND_EVT_FLASH
{
public:
	void (*ps_key)(IMiTokenBLEV2_2* sender, const struct ble_msg_flash_ps_key_evt_t *msg);
};
class BLE_COMMAND_EVT_ATTRIBUTES
{
public:
	void (*value)(IMiTokenBLEV2_2* sender, const struct ble_msg_attributes_value_evt_t *msg);
	void (*user_read_request)(IMiTokenBLEV2_2* sender, const struct ble_msg_attributes_user_read_request_evt_t *msg);
	void (*status)(IMiTokenBLEV2_2* sender, const struct ble_msg_attributes_status_evt_t *msg);
};
class BLE_COMMAND_EVT_CONNECTION
{
public:
	void (*status)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_status_evt_t *msg);
	void (*version_ind)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_version_ind_evt_t *msg);
	void (*feature_ind)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_feature_ind_evt_t *msg);
	void (*raw_rx)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_raw_rx_evt_t *msg);
	void (*disconnected)(IMiTokenBLEV2_2* sender, const struct ble_msg_connection_disconnected_evt_t *msg);
};
class BLE_COMMAND_EVT_ATTCLIENT
{
public:
	void (*indicated)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_indicated_evt_t *msg);
	void (*procedure_completed)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_procedure_completed_evt_t *msg);
	void (*group_found)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_group_found_evt_t *msg);
	void (*attribute_found)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_attribute_found_evt_t *msg);
	void (*find_information_found)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_find_information_found_evt_t *msg);
	void (*attribute_value)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_attribute_value_evt_t *msg);
	void (*read_multiple_response)(IMiTokenBLEV2_2* sender, const struct ble_msg_attclient_read_multiple_response_evt_t *msg);
};
class BLE_COMMAND_EVT_SM
{
public:
	void (*smp_data)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_smp_data_evt_t *msg);
	void (*bonding_fail)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_bonding_fail_evt_t *msg);
	void (*passkey_display)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_passkey_display_evt_t *msg);
	void (*passkey_request)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_passkey_request_evt_t *msg);
	void (*bond_status)(IMiTokenBLEV2_2* sender, const struct ble_msg_sm_bond_status_evt_t *msg);
};
class BLE_COMMAND_EVT_GAP
{
public:
	void (*scan_response)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_scan_response_evt_t *msg);
	void (*mode_changed)(IMiTokenBLEV2_2* sender, const struct ble_msg_gap_mode_changed_evt_t *msg);
};
class BLE_COMMAND_EVT_HARDWARE
{
public:
	void (*io_port_status)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_io_port_status_evt_t *msg);
	void (*soft_timer)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_soft_timer_evt_t *msg);
	void (*adc_result)(IMiTokenBLEV2_2* sender, const struct ble_msg_hardware_adc_result_evt_t *msg);
};
class BLE_COMMAND_EVT_DFU
{
public:
	void (*boot)(IMiTokenBLEV2_2* sender, const struct ble_msg_dfu_boot_evt_t *msg);
};

class BLE_COMMAND_CHAIN_LINK_FULL : public IBLE_COMMAND_CHAIN_LINK
{
public:
	BLE_COMMAND_CHAIN_LINK_FULL();
	virtual bool isLinkModular() const;
	class
	{
	public:
		void (*default)(IMiTokenBLEV2_2* sender, const void* v); 
	}ble;
	class
	{
	public:
		BLE_COMMAND_EVT_SYSTEM system;
		BLE_COMMAND_EVT_FLASH flash;
		BLE_COMMAND_EVT_ATTRIBUTES attributes;
		BLE_COMMAND_EVT_CONNECTION connection; 
		BLE_COMMAND_EVT_ATTCLIENT attclient;
		BLE_COMMAND_EVT_SM sm;
		BLE_COMMAND_EVT_GAP gap;
		BLE_COMMAND_EVT_HARDWARE hardware;
		BLE_COMMAND_EVT_DFU dfu;
	}evt;
	class
	{
	public:
		BLE_COMMAND_RSP_SYSTEM system;
		BLE_COMMAND_RSP_FLASH flash;
		BLE_COMMAND_RSP_ATTRIBUTES attributes;
		BLE_COMMAND_RSP_CONNECTION connection;
		BLE_COMMAND_RSP_ATTCLIENT attclient;
		BLE_COMMAND_RSP_SM sm;
		BLE_COMMAND_RSP_GAP gap;
		BLE_COMMAND_RSP_HARDWARE hardware;
		BLE_COMMAND_RSP_TEST test;
		BLE_COMMAND_RSP_DFU dfu;
	}rsp;
};

class BLE_COMMAND_CHAIN_LINK_MODULAR_RSP
{
public:
	BLE_COMMAND_RSP_SYSTEM* system;
	BLE_COMMAND_RSP_FLASH* flash;
	BLE_COMMAND_RSP_ATTRIBUTES* attributes;
	BLE_COMMAND_RSP_CONNECTION* connection;
	BLE_COMMAND_RSP_ATTCLIENT* attclient;
	BLE_COMMAND_RSP_SM* sm;
	BLE_COMMAND_RSP_GAP* gap;
	BLE_COMMAND_RSP_HARDWARE* hardware;
	BLE_COMMAND_RSP_TEST* test;
	BLE_COMMAND_RSP_DFU* dfu;
};

class BLE_COMMAND_CHAIN_LINK_MODULAR_EVT
{
public:
	BLE_COMMAND_EVT_SYSTEM* system;
	BLE_COMMAND_EVT_FLASH* flash;
	BLE_COMMAND_EVT_ATTRIBUTES* attributes;
	BLE_COMMAND_EVT_CONNECTION* connection;
	BLE_COMMAND_EVT_ATTCLIENT* attclient;
	BLE_COMMAND_EVT_SM* sm;
	BLE_COMMAND_EVT_GAP* gap;
	BLE_COMMAND_EVT_HARDWARE* hardware;
	BLE_COMMAND_EVT_DFU* dfu;
};

class BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD : public IBLE_COMMAND_CHAIN_LINK
{
public:
	BLE_COMMAND_CHAIN_LINK_MODULAR_HEAD();
	virtual bool isLinkModular() const;

	void(*default)(IMiTokenBLEV2_2* sender, const void* v);
	BLE_COMMAND_CHAIN_LINK_MODULAR_EVT* evt;
	BLE_COMMAND_CHAIN_LINK_MODULAR_RSP* rsp;
};

#endif