sudo -E ./launch.sh basic_routing_p4testgen config.ini
sudo -E python controller.py -p basic_routing_p4testgen | tee log_basic_routing
sudo -E ./launch.sh firewall_p4testgen config.ini
sudo -E python controller.py -p firewall_p4testgen | tee log_firewall
sudo -E ./launch.sh rate_limiter_p4testgen config.ini
sudo -E python controller.py -p rate_limiter_p4testgen | tee log_rate_limiter
sudo -E ./launch.sh mac_learning_p4testgen config.ini
sudo -E python controller.py -p mac_learning_p4testgen | tee log_mac_learning
sudo -E ./launch.sh load_balance_p4testgen config.ini
sudo -E python controller.py -p load_balance_p4testgen | tee log_load_balance
sudo -E ./launch.sh dos_defense_p4testgen config.ini
sudo -E python controller.py -p dos_defense_p4testgen | tee log_dos_defense
sudo -E ./launch.sh mirror_clone_p4testgen config.ini
sudo -E python controller.py -p mirror_clone_p4testgen | tee log_mirror_clone
sudo -E ./launch.sh netchain_p4testgen config.ini
sudo -E python controller.py -p netchain_p4testgen | tee log_netchain
sudo -E ./launch.sh cheetah_groupby_p4testgen config.ini
sudo -E python controller.py -p cheetah_groupby_p4testgen | tee log_cheetah_groupby
sudo -E ./launch.sh dv_router_p4testgen config.ini
sudo -E python controller.py -p dv_router_p4testgen | tee log_dv_router