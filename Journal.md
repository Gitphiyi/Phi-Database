# Implementation Journal

# Day 1 7/16/2025
- Today begins my journey on building my database from scratch. Why do I want to create a database? I think it mostly has to do with the fact that I am not patient enough to build an OS right now, I don't have that much time for personal projects after work, and I enjoy seeing very tangible progress. Thus, building a Database seems to fulfill my desire to learn, implement a finished product, and have fun.
- I have attempted to tackle this project last summer with zero knowledge of systems, but now I have come back with a bit more experience. I think I will avoid the first three steps of Tokenizer, Parser, and Code Generator, and instead work on the Pager. I believe the Pager is the most core element of a database, and thus, I will naturally find the need for the other subsytems of a database.
- My first goal for today is to create writes/reads/deletes to persistent storage.

# Day 2 7/19/2025
- Finished up the POSIX Wrapper. Wrote it to add additional error/exception handling
- Created folder structure. First time working with C++ so I learned to create an include folder with a src folder
- Created a Table.hpp and PageCache.hpp. The Table.hpp will basically be a file that only gets concatenated by a page at a time. The PageCache.hpp will be used to store pages from disk in memory so for people who want to read parts they can do so in memory
- Decided to allow tables to be row oriented or column oriented

# Day 3 7/25/2025
- Worked on figuring out how to persist tables to storage. Considered writing directly to Disk Driver File Descriptor, but that seemed like a bad idea since I would have no idea if I am overwriting any important information. Instead, I will trust the OS and let it allocate pages for me using the file system. If this become a problem with performance I will figure it out then, but for now I think this will more than suffice
- I now am going to work on the central core of the Database which is the Database.hpp file. This will allow people to configure the database and then I can actually test out some reads/writes
- I also need to start writing test cases for everything that I write especially making sure the file writes work

# Day 4 7/27/2025
- Including the work that I did yesterday and today since I forgot to document the work I was doing yesterday. I got tired of writing and adding .cpp files to my shell commands to test out components, so I decided to use a build system namely Bazel. Why choose Bazel? There isn't much logic to this other than I want to learn how to use Bazel, it is the build tool currently used at my internship, and it looks much easier to start than writing MakeFiles as I have done previously. Unfortunately it also turned out to be quite a pain in the butt to set up and learn. A key point I realized is that I wanted to restructure my codebase so that components each got their own Build file and brought in the dependencies it needed in its own Build file. Then a single larger Build file could simply add each component as a dependency itself. This also led me to realize the extremely trivial but important point that I should build out the components that have no dependencies first. While this should've been obvious, I didn't really want to write code for some of the basic things, but now I realize I should've done that first. Thus, I am now working on finishing up my Page caching component and Posix File API Wraper DbFile.