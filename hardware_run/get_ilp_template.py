import argparse
import json
from collections import OrderedDict
import os
import sys

def print_tbl2stage(mau_json_path):
  if not os.path.isabs(mau_json_path):
    print("ERR: ", mau_json_path) 
  print(mau_json_path)

  with open(mau_json_path, "r") as f:
    j_out = OrderedDict()
    j = json.loads(f.read())
    tbl_stage = [] 
    tbl2gress = {}
    tbl2actions = {}
    for tbl in j["tables"]:
      tbl_stage.append((tbl["name"], tbl["stage_allocation"][0]["stage_number"]))
      tbl2gress[tbl["name"]] = tbl["gress"]
      tbl2actions[tbl["name"]] = []
      for act in tbl["action_parameters"]:
        tbl2actions[tbl["name"]].append(act["action_name"])
    tbl_stage.sort(key=lambda x : x[1])

    act_pool = []
    dup_act_pool = []
    skip_act = ["NoAction"]
    skip_tbl = ["tbl_act"]
    for tbl,stage in tbl_stage:
      for act in tbl2actions[tbl]:
        if act in act_pool:
          dup_act_pool.append(act)
        act_pool.append(act)

    for tbl,stage in tbl_stage:
      if tbl in skip_tbl:
        continue
      if tbl2gress[tbl] == "ingress":
        for act in tbl2actions[tbl]:
          if act in skip_act:
            continue
          if act not in dup_act_pool:
            j_out[act] = {"field": -1, "increment": -1}
          else:
            j_out[act+"_fp4_"+tbl] = {"field": -1, "increment": -1}
          act_pool.append(act)
    for tbl,stage in tbl_stage:
      if tbl in skip_tbl:
        continue
      if tbl2gress[tbl] == "egress":
        for act in tbl2actions[tbl]:
          if act in skip_act:
            continue
          if act not in dup_act_pool:
            j_out[act] = {"field": -1, "increment": -1}
          else:
            j_out[act+"_fp4_"+tbl] = {"field": -1, "increment": -1}
          act_pool.append(act)
  print(json.dumps(j_out, indent=2))  


if __name__ == '__main__':
  parser = argparse.ArgumentParser()

  parser.add_argument("-i", "--input_path", type=str, required=True, help="Program name")

  args = parser.parse_args()

  print_tbl2stage("/home/leoyu/bf-sde-9.2.0/pkgsrc/p4-build/tofino/"+args.input_path+"/logs/mau.json")

