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
- `runme.sh`: Script to run everything from a single place
- `install.sh`: Script to install the required dependencies


## Installation
We have provided 2 vms for easily running the system: server (for instrumentation) and switch (For running switch control-plane and data-plane).

If you don't want to run in VMs, just call `./install_server.sh` at a server and `./install_switch.sh` at the switch to install the required dependencies at both places.


## How to run

`runme.sh` is the main entry point for running different parts of the systems.
There are three main modules: instrumentation, Switch Under Test (SUT) and Switch Doing Test (SDT)

First, we will need to instrument the input P4 program at the server. 
Use the commands 
```
./instrument ...
```  
It will generate `<program_name>_ut_hw.p4`, `<program_name>_dt_hw.p4`.

Move the files to ...

Run the switch under test. Rules must be installed to output from ports xyz
```
```

Run the switch doing test. Rules will be auto-populated

### Steps
1. Compile the instrumentation 