import argparse
import json
import os
import sys

def print_tbl2stage(mau_json_path, verbose):
  if not os.path.isabs(mau_json_path):
    print("ERR: ", mau_json_path) 
  print(mau_json_path)

  with open(mau_json_path, "r") as f:
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
    print("------ Ingress ------")
    for tbl,stage in tbl_stage:
      if tbl2gress[tbl] == "ingress":
        print("{0}:{1}".format(tbl, stage))
        if verbose:
          print("  {} actions".format(len(tbl2actions[tbl])))
          for act in tbl2actions[tbl]:
            print("  {}".format(act))
    print("------ Egress ------")
    for tbl,stage in tbl_stage:
      if tbl2gress[tbl] == "egress":
        print("{0}:{1}".format(tbl, stage))
        if verbose:
          print("  {} actions".format(len(tbl2actions[tbl])))
          for act in tbl2actions[tbl]:
            print("  {}".format(act))


if __name__ == '__main__':
  parser = argparse.ArgumentParser()

  parser.add_argument("-i", "--input_path", type=str, required=True, help="Program name")
  parser.add_argument("-v", "--verbose", required=False, action="store_true")

  args = parser.parse_args()

  print_tbl2stage("/home/leoyu/bf-sde-9.2.0/pkgsrc/p4-build/tofino/"+args.input_path+"/logs/mau.json", args.verbose)

