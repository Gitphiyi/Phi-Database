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
