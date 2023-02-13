/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef __SWITCH_PD_COMMON_H__
#define __SWITCH_PD_COMMON_H__

#define switch_pd_log_level_debug() 1

#define SWITCH_PD_HANDLE_VALID(_handle) (_handle != SWITCH_PD_INVALID_HANDLE)

switch_status_t switch_pd_init(switch_device_t device);

switch_status_t switch_pd_free(switch_device_t device);

switch_status_t switch_pd_client_init(switch_device_t device);

char *switch_pd_table_id_to_string(switch_pd_table_id_t table_id);

char *switch_pd_action_id_to_string(switch_pd_action_id_t action_id);

switch_status_t switch_pd_status_to_status(switch_pd_status_t pd_status);

switch_pd_status_t switch_status_to_pd_status(switch_status_t status);

switch_status_t switch_pd_entry_dump(switch_device_t device,
                                     switch_pd_dump_entry_t *pd_entry);

switch_status_t switch_pd_max_ports_get(switch_device_t device,
                                        switch_uint32_t *max_ports);

switch_status_t switch_pd_port_list_get(switch_device_t device,
                                        switch_uint32_t max_ports,
                                        switch_port_t *fp_list,
                                        switch_dev_port_t *dev_port_list);

switch_status_t switch_pd_recirc_port_list_get(
    switch_device_t device,
    switch_uint32_t *max_recirc_ports,
    switch_port_t *recirc_port_list,
    switch_dev_port_t *recirc_dev_port_list);

switch_status_t switch_pd_max_pipes_get(switch_device_t device,
                                        switch_uint32_t *max_pipes);

switch_status_t switch_pd_table_init();

switch_status_t switch_pd_table_entry_count_get(switch_device_t device,
                                                switch_table_id_t table_id,
                                                switch_uint32_t *num_entries);

#endif /* __SWITCH_PD_COMMON_H__ */
