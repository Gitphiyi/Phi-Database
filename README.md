# Phi-Database

## Build System
This project is using Bazel as a build system, and thus, every component is in a subfolder. The folder structure of tests is exactly the same as the folder structure in src. To run any tests on a component run the shell command 
<br>
bazel test --cxxopt=-std=c++17 --test_output=all //tests/**component name**:**component name**_test

For example, the shell command to run the dbfile tests is:
<br>
bazel test --cxxopt=-std=c++17 --test_output=all //tests/dbfile:dbfile_test

