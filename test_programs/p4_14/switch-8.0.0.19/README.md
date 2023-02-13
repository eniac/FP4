Switch
=========

The switch repository contains the switch.p4 sample P4 program along with all
the library repos to manipulate the switch using SAI, SwitchAPI.

        +-----+   +-----+   +-----+   +-----+
        |App a|   |App j|   |App n|   |App z|
        |     |...|     |...|     |...|     |
        +-----+   +-----+   +-----+   +-----+
           |         |         |         |
           |         |    +----|         |
    +------------------+  |
    |     SAI          |  |
    |                  |  |
    +-----------------------+
    |      Switch API       |
    |                       |
    +-----------------------+---------+
    |      Resource Mgmt. API         |
    | (auto-gen. from switch.p4)      |
    +---------------------------------+
    |        Soft Switch              |
    |  (compiled from switch.p4)      |
    +---------------------------------+

Directory Structure
------------------
p4src - P4 sources  
switchsai - SAI library  
switchapi - SwitchAPI  
tests/ptf-tests - P4 dependent(PD), SAI and API tests  
tests/of-tests - Openflow tests  

