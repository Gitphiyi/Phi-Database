# Phi-Database
Fully functional relational database with SQL Compiler, Transaction Processing, Transaction Logging, Persistent Storage, Page Caching, and all Database Operations. Optimizing in-memory operations and making it distributed TBD. Currently aiming to support SQL operations specified by SQL-92.

## Build System
This project used to use Bazel. Turns out Bazel was not right fit, so instead I used CMake!  
<br>
Building MakeFile: **cmake -S . -B build** in whatever directory you need to update MakeFile after changing project structure
<br>
Running MakeFile: **make -j**




#TODO: go back to snake case
#TODO: get my implementation of table fixed