sudo -E python -u SDTcontroller.py -p basic_routing_dt_hw -r basic_routing_ut_hw_rules.txt | tee log_basic_routing.txt
sudo -E python -u SDTcontroller.py -p dv_router_dt_hw -r dv_router_ut_hw_rules.txt | tee log_dv_router.txt
sudo -E python -u SDTcontroller.py -p Firewall_dt_hw -r Firewall_ut_hw_rules.txt | tee log_Firewall.txt
sudo -E python -u SDTcontroller.py -p LoadBalance_dt_hw -r LoadBalance_ut_hw_rules.txt | tee log_LoadBalance.txt
sudo -E python -u SDTcontroller.py -p mirror_clone_dt_hw -r mirror_clone_ut_hw_rules.txt | tee log_mirror_clone.txt
sudo -E python -u SDTcontroller.py -p netchain_dt_hw -r netchain_ut_hw_rules.txt | tee log_netchain.txt
sudo -E python -u SDTcontroller.py -p RateLimiter_dt_hw -r RateLimiter_ut_hw_rules.txt | tee log_RateLimiter.txt
