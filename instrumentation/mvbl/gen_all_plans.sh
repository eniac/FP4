SDE_PREFIX=/home/leoyu/bf-sde-9.2.0/pkgsrc/p4-build/tofino

python3 gen_mvbl_plan.py -d $SDE_PREFIX/basic_routing/graphs/ingress.dot -c $SDE_PREFIX/basic_routing/context.json
# python3 gen_mvbl_plan.py -d test_examples/basic_routing/ingress.dot -c test_examples/basic_routing/context.json
