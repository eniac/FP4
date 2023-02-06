import argparse
import json
import os
import sys

def print_tbl2stage(mau_json_path):
  if not os.path.isabs(mau_json_path):
    print("ERR: ", mau_json_path) 
  print(mau_json_path)

  with open(mau_json_path, "r") as f:
    j = json.loads(f.read())
    tbl_stage = [] 
    tbl2gress = {} 
    for tbl in j["tables"]:
      tbl_stage.append((tbl["name"], tbl["stage_allocation"][0]["stage_number"]))
      tbl2gress[tbl["name"]] = tbl["gress"]
    tbl_stage.sort(key=lambda x : x[1])
    print("------ Ingress ------")
    for tbl,stage in tbl_stage:
      if tbl2gress[tbl] == "ingress":
        print("{0}:{1}".format(tbl, stage))
    print("------ Egress ------")
    for tbl,stage in tbl_stage:
      if tbl2gress[tbl] == "egress":
        print("{0}:{1}".format(tbl, stage))


if __name__ == '__main__':
  parser = argparse.ArgumentParser()

  parser.add_argument("-i", "--input_path", type=str, required=True, help="Program name")

  args = parser.parse_args()

  print_tbl2stage("/home/leoyu/bf-sde-9.2.0/pkgsrc/p4-build/tofino/"+args.input_path+"/logs/mau.json")

