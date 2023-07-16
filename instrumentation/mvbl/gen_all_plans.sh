SDE_PREFIX=/home/leoyu/bf-sde-9.2.0/pkgsrc/p4-build/tofino

program_list=(
#    "firewall"
#    "load_balance"
#    "rate_limiter"
   "basic_routing"
#    "dv_router"
#    "mirror_clone"
#    "netchain"
)

for program in "${program_list[@]}"
do
    python3 gen_mvbl_plan.py -d $SDE_PREFIX/$program/graphs/ingress.dot -c $SDE_PREFIX/$program/context.json
done

# python3 gen_mvbl_plan.py -d test_examples/switch_80019/ingress.dot -c test_examples/switch_80019/context.json
# python3 gen_mvbl_plan.py -d test_examples/basic_routing/ingress.dot -c test_examples/basic_routing/context.json
# {'dst': 'ipv4_fib_lpm', 'src': 'ipv4_fib__NoAction', 'weight': 1}
# {'dst': 'ipv4_fib_lpm', 'src': 'ipv4_fib__on_miss', 'weight': 1}
# {'dst': 'tbl_act', 'src': 'hdripv4isValid', 'weight': 8}
# {'dst': 'ipv4_fib__NoAction', 'src': 'ipv4_fib', 'weight': 2}
# {'dst': 'ipv4_fib__on_miss', 'src': 'ipv4_fib', 'weight': 4}
# {'dst': 'nexthop', 'src': 'ipv4_fib', 'weight': 6}
# {'dst': 'ipv4_fib_lpm', 'src': 'ipv4_fib', 'weight': 7}
# {'dst': 'ipv4_fib_lpm', 'src': 'ipv4_fib__fib_hit_nexthop', 'weight': 1}
# ----- Variable 1 additions------
# {'dst': 'nexthop__set_egress_details', 'src': 'nexthop', 'weight': 1}
# {'dst': 'nexthop__on_miss', 'src': 'nexthop', 'weight': 2}
# {'dst': 'nexthop__NoAction', 'src': 'nexthop', 'weight': 3}
# {'dst': 'bd__set_vrf', 'src': 'bd', 'weight': 20}
# {'dst': 'ipv4_fib', 'src': 'bd', 'weight': 40}
# {'dst': 'ipv4_fib_lpm', 'src': 'ipv4_fib', 'weight': 4}
# {'dst': 'bd', 'src': 'port_mapping', 'weight': 60}
# {'dst': 'port_mapping__set_bd', 'src': 'port_mapping', 'weight': 120}
# {'dst': 'tbl_act', 'src': 'hdripv4isValid', 'weight': 180}
# {'dst': 'ipv4_fib_lpm__on_miss', 'src': 'ipv4_fib_lpm', 'weight': 4}
# {'dst': 'nexthop', 'src': 'ipv4_fib_lpm', 'weight': 8}
# {'dst': 'ipv4_fib_lpm__NoAction', 'src': 'ipv4_fib_lpm', 'weight': 12}