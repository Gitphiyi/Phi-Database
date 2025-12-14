# Understanding a Database

## High Level Overview
- Tokenizer
- Parser
- Code Generator
- Virtual Machine
- Pager
    - Loads pages of data into disk
    - Controls transactions
    - maintaining logging to prevent database corruption
- OS Interface
    - abstraction layer to allow database to be run on different OSes

## Resources
- https://gitlab.oit.duke.edu/compsci516/ddb/-/tree/main/src/ddb?ref_type=heads
    - Duke Devils Database. Best resource for SQL Compiler