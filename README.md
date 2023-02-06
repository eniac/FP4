# FP4: Line rate Greybox Fuzz Testing for P4 Switches

## Description

FP4 is a fuzz-testing framework for P4 switches that achieves high expressiveness, coverage, and scalability.

## Main Contents

- `control_plane/`: Contains the various control-planes
    - `SDT/`: Control-plane of SDT
    - `SUT/`: Static SUT controller
- `instrumentation/`: Instruments the input P4 program and also output the program doing test.
    - `frontend.l`: Lexer for input P4 program
    - `frontend.y`: Parser for the input P4 program and main C program
    - `src/ast/`: Syntax tree for the P4 programs
    - `src/dt.cpp`: Class to generate the `.p4` code for fuzzing engine. It also generates `.json` files for the control-plane.
    - `src/ut.cpp`: Class to instrument the input program
- `mininet/`: Mininet setup
- `test_programs/`: Contains the programs we tested
    - `simulation`: Programs to test in mininet
    - `p4_14`: Programs to run in tofino p4_14
- `install_<machine>.sh`: Script to install the required dependencies


## Installation
We have provided 2 vms for easily running the system: server (for instrumentation) and switch (For running switch control-plane and data-plane).

If you don't want to run in VMs, just call `./install_server.sh` at a server and `./install_switch.sh` at the switch to install the required dependencies at both places.

## How to run
There are three main modules: instrumentation, Switch Under Test (SUT) and Switch Doing Test (SDT)

First, we will need to instrument the input P4 program at the server. 
Use the commands 
```
cd $HOME/FP4/instrumentation
./instrument.sh -t hw -r <rules_file> <p4 program>

# Example
./instrument.sh -t hw -r $HOME/FP4/test_programs/p4_14/dv_router/hardware_rules.txt $HOME/FP4/test_programs/p4_14/dv_router/dv_router.p4
```  
It will generate `<program_name>_ut_hw.p4`, `<program_name>_dt_hw.p4`, `<program_name>_ut_hw_rules.txt` in `$HOME/FP4/instrumentation/sample_out/` and also generate the `.json` files in `$HOME/FP4/instrumentation/out/`.

Alternatively, `instrument_all_hw.sh` compiles a batch of test_programs and prepares the files under `$HOME/FP4/hardware_run` and `$HOME/FP4/control_plane`.

### SUT switch
1. Move the files `<program_name>_ut_hw.p4` in `$HOME/FP4/hardware_run/`. 
1. Compile the program using `sudo -E ./compile_p4_14.sh <program_name>_ut_hw.p4>`.
1. Move the file `<program_name>_ut_hw_rules.txt` and any other controller extension (More on this later) in `$HOME/FP4/control_plane/SUT/`.
1. On separate terminals, run the hardware program and the control-plane program.

```
# Run switch - terminal 1
cd $HOME/FP4/hardware_run/
sudo -E ./launch.sh <program_name>_ut_hw config_sut.ini

# Run control plane - terminal 2
cd $HOME/FP4/control_plane/SUT
python SUTcontroller.py -p <program_name>_ut_hw 
```

### SDT switch
1. Move the files `<program_name>_dt_hw.p4` in `$HOME/FP4/hardware_run/`. 
1. Compile the program using `sudo -E ./compile_p4_14.sh <program_name>_dt_hw.p4>`.
1. Move the file `<program_name>_ut_hw_rules.txt`, and all the `.json` files in `$HOME/FP4/control_plane/SDT/`.
1. On separate terminals, run the hardware program and the control-plane program.

```
# Run switch - terminal 1
cd $HOME/FP4/hardware_run/
sudo ./launch.sh <program_name>_dt_hw config_sdt.ini

# Run control plane - terminal 2
cd $HOME/FP4/control_plane/SDT
sudo -E python SDTcontroller.py -p <program_name>_dt_hw -r <program_name>_ut_hw_rules.txt 
```

