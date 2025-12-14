# Phi-Database: Building a Relational Database from Scratch

## Abstract

Phi-Database is a relational database I built in C++20 to support SQL-92 operations. This report goes over the development process, what I built, the problems I ran into, and what I learned. The project covers storage management, query processing (lexing, parsing, execution), and transaction processing.

## 1. Introduction

### 1.1 Why I Built This

I wanted to actually understand how databases work under the hood. PostgreSQL, MySQL, SQLite - they all hide the complexity of what's actually happening when you run a query. I wanted to know:

- How do SQL queries get turned into something executable?
- How is data actually stored on disk?
- How does everything connect together?
- How do transactions maintain consistency?

Beyond databases, I also intended to use this project to improve my system design skills and get deeper into C++ implementation details. I never actually had the chance to build an end-to-end system before, as I have always spent my time improving on existing systems. Because I already had experience taking CS 316 and spending quite a lot of time working with databases at internships, I figured that a database would be a great project to actually design out. Secondly, I wanted to improve my C++ skills. I spent most of my time at Duke programming in memory-managed languages like Go, Java, and Python. While they are great for programming velocity, I believe it is essential for me to become much more acquainted with a memory-unsafe language like C/C++. This ties into my final desire to dive deeper into Linux. I want to understand what happens at the kernel level and read the manpages to gain a better understanding of the implementation underlying all the syscalls I make. In essence, by getting a first-principles view on how to create a database, I was hoping to improve my memory management skills, system design abilities, and gain a deeper understanding of Linux.

### 1.2 What I Built

- SQL Compiler (lexer + recursive descent parser that outputs Relational Algebra trees)
- Storage Manager (heap files with page directories, table operations)
- Page Manager (file I/O with caching)
- Query Executor (RA Tree to storage operations translator)
- Transaction Processor (Scheduler)

### 1.3 Tech Stack

- C++20
- CMake 3.24+
- Address Sanitizer for catching memory bugs
- Linux/GCC 11

## 2. Architecture

The architecture of my database is a much simpler version than how difficult databases are. After building this database, the problem can be split into two: processing millions of transactions at once or improving latency once a query is received. There are of course a plethora of problems that are involved when addressing these two problems, and I chose to tackle the second one. Even within the second problem, there are various sections that can massively improve the system. I initially attempted to introduce a more intelligent SQL Query Optimizer given information about the tables, but I did not have enough time to do so. Thus, I focused a majority of my efforts on simply building a working system and some latency improvements on the storage operation side.

The system can be split into four main layers. SQL Querying at the top which gets tokenized and parsed into a Relational Algebra tree. Then the query executor builds an operator tree from that and runs it against the storage layer, which handles all the disk I/O through fixed-size pages. Transaction processing sits alongside to handle operation scheduling and logging.

### Component Breakdown

The SQL Compiler spans 4 files (lexer, parser, AST definitions). The Query Executor handles the RA tree to operator translation. Storage Manager covers tables, heap files, and rows. Page Manager handles file I/O and caching. Transaction processing handles scheduling and logging. The rest is shared types and utilities.

## 3. Implementation

### 3.1 SQL Compiler

#### Lexer

The lexer tokenizes SQL into a stream of tokens. Handles 40+ keywords, identifiers, literals (int, float, string, bool), operators, and both comment styles.

```cpp
std::vector<Token> tokenize_query(string& query);
```

#### Parser

The parser is a recursive descent parser. Took forever to get right.

Supports:
- SELECT with DISTINCT, WHERE, GROUP BY, HAVING, ORDER BY, LIMIT
- INSERT (single and multi-row)
- UPDATE with SET and WHERE
- DELETE with WHERE
- CREATE TABLE with constraints
- DROP TABLE

Expression precedence goes OR (lowest), AND, NOT, comparisons, add/sub, mult/div (highest).

The parser outputs RA nodes directly instead of an intermediate SQL AST:

```cpp
enum class RANodeType {
    TABLE_SCAN,
    PROJECT,        // π
    SELECT_OP,      // σ
    INNER_JOIN,     // ⋈
    LEFT_JOIN, RIGHT_JOIN, FULL_JOIN,
    CROSS_PRODUCT,  // ×
    GROUP_BY,       // γ
    SORT,           // τ
    LIMIT_OP,
    // ...
};
```

#### Query Optimization

The system includes an OptimizerCache structure for caching optimization decisions. The main optimization currently implemented is converting the SQL directly to a Relational Algebra tree that can be traversed top-down for execution. Future work includes cost-based optimization for join ordering.

### 3.2 Storage Manager

#### Heap Files

Storage uses heap file organization with page directories:

```cpp
struct HeapFile_Metadata {
    string  identifier;
    u64     heap_id;
    u64     table_id;
    u64     num_pages;
    u64     num_records;
    u8      next_heapfile;
} __attribute__((packed));
```

Page 0 contains metadata, schema, and the page directory.

#### Pages

Fixed-size pages (128 bytes default for testing):

```cpp
struct Page {
    bool    valid_bit;
    bool    dirty_bit;
    u16     ref_count;
    u32     id;
    u32     used_bytes;
    byte*   data[PAGE_DATA_SIZE];
};
```

#### Table Operations

The Table class handles row-level operations:
- insert_row() - adds records to the heap file
- read_row() - retrieves records by RowId
- scan() - sequential scan returning all rows
- Schema management with column types (INT, FLOAT, STRING, BOOL)

#### Rows

```cpp
using datatype = std::variant<int, float, string, bool, int64_t, double>;

struct Row {
    u8                      numCols;
    std::vector<datatype>   values;
};
```

### 3.3 Query Executor

Uses the Volcano iterator model with batching:

```cpp
struct StorageOps {
    virtual void open() = 0;
    virtual std::vector<Row*> next() = 0;
    virtual void close() = 0;
};
```

Implemented operators: SeqScan, IndexScan, FilterOp, ProjectOp, LimitOp, CrossProductOp, NestedLoopJoin.

### 3.4 Page Manager

DbFile is a singleton that manages all file descriptors. It handles:
- read_at() / write_at() for positioned I/O
- File path management with automatic directory creation
- Locking support (shared/exclusive modes)

PageCache does LRU-style caching with:
- Page eviction when cache is full
- Write-through policy for durability
- Reference counting for pinned pages

### 3.5 Transaction Processing

#### Transaction Scheduler (TScheduler)

The scheduler handles operation ordering for transactions:

```cpp
void TScheduler::schedule_transaction() {
    // Groups operations by table
    // Adds LOCK operations before table access
    // Adds UNLOCK operations after table operations complete
    // Adds COMMIT at end of transaction
}
```

Operations are partitioned by which table they modify, then lock/unlock pairs are inserted to ensure serializability.

#### Transaction Logger (TLogger)

The logger maintains an operation log for recovery:

```cpp
class TLogger {
    void add_ops(std::vector<Operation>& operations);
    void flush_ops();  // Execute logged operations
    void clear_ops();  // Clear after commit
};
```

The logger queues operations and flushes them through the page cache. This enables undo/redo recovery by replaying the log.

### 3.6 Indexing

The storage layer supports B+ tree indexes for efficient key-based lookups:

```cpp
class BPlusTree {
public:
    void insert(const datatype& key, const RowId& rid);
    bool remove(const datatype& key);
    std::optional<RowId> search(const datatype& key) const;
    std::vector<RowId> rangeSearch(const datatype& low, const datatype& high) const;
};
```

The B+ tree uses a configurable order (default 4) and supports:
- Point lookups returning a single RowId
- Range scans for queries like `WHERE x BETWEEN a AND b`
- Duplicate key handling for non-unique indexes

The Index class wraps BPlusTree and associates it with a table column:

```cpp
class Index {
    void insert(const datatype& key, const RowId& rid);
    std::optional<RowId> lookup(const datatype& key) const;
    std::vector<RowId> rangeLookup(const datatype& low, const datatype& high) const;
};
```

Tables can create indexes on any column. When an index exists, the query executor can use IndexScan instead of SeqScan for equality and range predicates.

## 4. Design Tradeoffs

### Performance vs Simplicity

- Chose heap files over more complex storage (like slotted pages) for simplicity
- Batch size of 64 rows balances memory usage with cache efficiency
- Left-deep join trees are suboptimal but simpler to implement than bushy trees

### Correctness vs Performance

- Address Sanitizer enabled for all builds catches memory bugs but adds overhead
- Write-through caching is slower than write-back but simpler to reason about for durability
- Strict two-phase locking in the scheduler ensures serializability but limits concurrency

### Scalability Considerations

- Singleton DbFile centralizes file management but could become a bottleneck
- Page cache size is fixed at startup
- No support for distributed queries (single-node only)

## 5. Problems I Ran Into

### 5.1 Bazel to CMake

Started with Bazel because I thought it would be cool. It wasn't. The biggest issue was Bazel's sandboxing - it creates its own isolated environment for builds, which means you can't easily access the files that get created during the build process. For a database project where I constantly needed to inspect heap files and debug page layouts on disk, this was a nightmare. I couldn't just look at what was being written without jumping through hoops.

Bazel also has a steep learning curve with its BUILD files and dependency management. Way too complex for what I needed. Switched to CMake and everything got simpler - I could just build and run, then inspect the output files directly.

### 5.2 Parser Burnout

SQL grammar is brutal. Nested subqueries, all the join types, getting expression precedence right. I hit a wall and had to step away.

Ended up working on heap files for a while, then came back to the parser with fresh eyes. That actually helped a lot. The final parser handles all join types, subqueries, BETWEEN/IN/LIKE/CASE expressions, and aggregates with GROUP BY/HAVING.

### 5.3 Struct Alignment Hell

Spent way too long on this.

Tried to serialize HeapFile_Metadata with memcpy. Doesn't work when you have std::string in the struct - the packed attribute gets ignored for non-POD types.

Had to do manual serialization:

```cpp
inline u8* to_bytes(HeapFile_Metadata* metadata) {
    u8* buffer = new u8[metadata->size];
    size_t offset = 0;
    std::memcpy(buffer + offset, metadata->identifier.data(), ...);
}
```

### 5.4 Join Ordering

When you have `FROM a JOIN b ON ... JOIN c ON ...`, what order do you evaluate that? Went with left-to-right, building a left-deep tree. Good enough for now, bushy trees can come later.

### 5.5 Integration

Built the compiler and storage manager separately. When I tried to connect them, everything broke:

- Missing includes everywhere
- Had both `struct Table` and `class Table` defined
- Empty function implementations I forgot about
- std::format doesn't exist in GCC 11

Created the Query Executor to bridge everything. Should have defined interfaces earlier.

## 6. Results

The database can:

1. Parse complex SQL into RA trees. A query like:
```sql
SELECT department, COUNT(*) as emp_count
FROM employees WHERE salary > 50000
GROUP BY department HAVING COUNT(*) > 5
ORDER BY emp_count DESC LIMIT 10
```

Produces a proper RA tree with Limit -> Sort -> Project -> GroupBy -> Select -> TableScan.

2. Create tables with schemas
3. Execute simple queries through the operator pipeline
4. Schedule transactions with proper lock ordering

Operators return batches of 64 rows for cache efficiency. All disk access goes through fixed-size pages. AddressSanitizer is on for all builds.

## 7. Future Work

Short term:
- Finish row insertion (currently stubbed)
- Implement UPDATE/DELETE execution
- Cost-based query optimization with index selection

Long term:
- Full ACID transactions with WAL (write-ahead logging)
- Deadlock detection in the lock manager
- Recovery manager using the transaction log
- Maybe distribution/sharding

## 8. Problems and Issues

### Build System Growing Pains

The Bazel to CMake migration cost me a week. Bazel's sandboxed execution model made it impossible to quickly iterate on storage code - every time I wanted to check what bytes were actually written to a heap file, I had to figure out where Bazel hid the outputs. For a project that requires constant low-level debugging, this was unworkable.

### Memory Management Bugs

Working in C++ after years of garbage-collected languages meant relearning memory discipline. AddressSanitizer caught several use-after-free bugs in the page cache where I was returning pointers to evicted pages. The fix required proper reference counting and ensuring pages stay pinned while in use.

### Cross-Component Type Conflicts

Building the compiler and storage manager in isolation led to duplicate type definitions. I had `struct Table` in the parser for AST nodes and `class Table` in storage for actual table operations. The linker errors were cryptic. Lesson learned: define shared interfaces early.

### File I/O Edge Cases

Reading Linux manpages for `pread`/`pwrite` revealed subtleties I hadn't considered - partial reads, signal interruptions, file descriptor limits. The DbFile class went through several iterations to handle these correctly.

## 9. What I Learned

### Technical

- Relational Algebra is the key abstraction between SQL and execution
- The Volcano iterator model composes really well
- Don't assume you can memcpy structs with complex types
- CMake > Bazel for projects this size
- Two-phase locking is straightforward to implement but has concurrency limits

### Process

- Switching between components when stuck prevents burnout
- Integration testing matters - things that work alone break together
- Writing things down (like Parser.md) helps clarify thinking

### Database Stuff

- SQL has a lot of edge cases
- Page size and row format affect everything above them
- The catalog (table metadata) is needed everywhere
- Transaction ordering is tricky to get right

## 10. Conclusion

Building a database from scratch is doable and teaches you a lot. Mine isn't production-ready but it works well enough to demonstrate the core concepts. The most valuable part was understanding how a query flows from SQL text all the way down to disk I/O, and how transactions maintain consistency across operations.

## References

1. "Database System Concepts" by Silberschatz, Korth, and Sudarshan
2. Berkeley DB Architecture Orcal White Paper
3. Duke Devils Database (DDB)
4. SQL-92 Specification
5. Phoenix SQL Grammar Documentation
6. https://craftinginterpreters.com/introduction.html

