# Understanding a Database

## High Level Overview
- Tokenizer
- Parser
- Code Generator
- Virtual Machine
- B+ Tree
- Pager
    - Loads pages of data into disk
    - Controls transactions
    - maintaining logging to prevent database corruption
- OS Interface
    - abstraction layer to allow database to be run on different OSes
