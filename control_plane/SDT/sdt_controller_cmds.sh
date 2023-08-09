sudo -E python -u SDTcontroller.py -p basic_routing_dt_hw -r basic_routing_ut_hw_rules.txt | tee log_basic_routing.txt
sudo -E python -u SDTcontroller.py -p dv_router_dt_hw -r dv_router_ut_hw_rules.txt | tee log_dv_router.txt
sudo -E python -u SDTcontroller.py -p firewall_dt_hw -r firewall_ut_hw_rules.txt | tee log_firewall.txt
sudo -E python -u SDTcontroller.py -p load_balance_dt_hw -r load_balance_ut_hw_rules.txt | tee log_load_balance.txt
sudo -E python -u SDTcontroller.py -p mirror_clone_dt_hw -r mirror_clone_ut_hw_rules.txt | tee log_mirror_clone.txt
sudo -E python -u SDTcontroller.py -p netchain_dt_hw -r netchain_ut_hw_rules.txt | tee log_netchain.txt
sudo -E python -u SDTcontroller.py -p rate_limiter_dt_hw -r rate_limiter_ut_hw_rules.txt | tee log_rate_limiter.txt
sudo -E python -u SDTcontroller.py -p mac_learning_dt_hw -r mac_learning_ut_hw_rules.txt | tee log_mac_learning.txt
