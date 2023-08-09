python SUTcontroller.py -p basic_routing_ut_hw
python SUTcontroller.py -p dv_router_ut_hw
python SUTcontroller.py -p firewall_ut_hw
python SUTcontroller.py -p load_balance_ut_hw
python SUTcontroller.py -p mirror_clone_ut_hw
python SUTcontroller.py -p netchain_ut_hw
python SUTcontroller.py -p rate_limiter_ut_hw
sudo -E python SUTcontroller.py -p dv_router_ut_hw -d dv_router_controller | tee log_sut_dv_router.txt
sudo -E python SUTcontroller.py -p mac_learning_ut_hw -d mac_learning_controller | tee log_sut_mac_learning.txt
sudo -E python SUTcontroller.py -p dos_defense_ut_hw -d dos_defense_controller | tee log_sut_dos_defense.txt
