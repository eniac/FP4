sudo -E ./launch.sh basic_routing_p4testgen config.ini
sudo -E python controller.py -p basic_routing_p4testgen | tee log_basic_routing.md