#include <iostream>
#include <cstddef>
#include <sys/stat.h>

#include "general/Types.hpp"
#include "general/Page.hpp"
#include "DbFile.hpp"
#include "Database.hpp"
#include "PageCache.hpp"
#include "scheduling/TScheduler.hpp"
#include "compiler/QueryProcessor.hpp"

using namespace DB;


// void dbfile_test(DbFile& dbFile, Page* buffer) {
//     buffer->valid_bit = 1;
//     for(int i = 0; i < PAGE_SIZE; i += sizeof(int)) {
//         memcpy(buffer->data + i, &i, sizeof(int));
//     }

//     dbFile.db_write_at(0, *buffer);
//     dbFile.read_at(buffer->id, *buffer, 0);
    
//     buffer->print<int>();

//     dbFile.close();
// }

// void pagecache_test(PageCache& cache, const string filepath) {
//     Page* test = new Page(1);
//     Page* p2 = new Page(2);
//     Page* p3 = new Page(3);

//     Page* test_read = new Page(2);
//     for(int i = 0; i < PAGE_DATA_SIZE / sizeof(int); i ++) {
//         memcpy(test->data + sizeof(int) * i, &i, sizeof(int));
//         int temp = i*2;
//         memcpy(p2->data + sizeof(int) * i, &temp, sizeof(int));
//         temp = i*3;
//         memcpy(p3->data + sizeof(int) * i, &temp, sizeof(int));
//     }

//     cache.write_through(*test, filepath);
//     cache.write_through(*p2, filepath);
//     cache.write_through(*p3, filepath);
//     cache.print();

//     cache.read(test->id, *test_read, filepath);
//     // test_read->print<int>();
//     // p2->print<int>();
//     // p3->print<int>();
//     cache.print();
//     delete test;
//     delete p2;
//     delete p3;
//     delete test_read;
// }

// void table_test(PageCache& cache) {
//     int cols = 5;
//     Schema* schema = new Schema(cols);
//     Table table = Table("test_table", *schema, cache);

// }

void test_query_processor(string query) {
    std::vector<Token> tokens = tokenize_query(query);
    std::cout<< "tokens: ";
    for(Token t : tokens) {
        std::cout << "[" << t.type << "] " << t.value << ", ";
    }
    std::cout << std::endl;
}
int main(int argc, char** argv) {
//     const string filename = "db/hi.db";
//     mkdir("db", 0755);
//     mkdir("db/table", 0755);
//     std::cout << "Creating dbfile at: " << filename << std::endl;
//     u32 numPages = 2;
//     DbFile dbFile = DbFile(filename, true);
//     PageCache pgCache(numPages, dbFile);
//     Database db("phi-db");

//     //dbfile_test();
//     //pagecache_test(pgCache, filename);
//     table_test(pgCache);

    string query = "WITH seq AS (SELECT generate_series(1,10) AS n)"
"SELECT u.id, u.jb 'user' 'name' AS uname, SUM(u.ints[2]) FILTER (WHERE u.n % 2=0) AS sum_even" 
"FROM (SELECT n AS id, ARRAY[1,n,n*n]::int[] AS ints FROM seq) u"
"JOIN LATERAL jsonb_array_elements_text t(tag) ON true "
"WHERE u.jb AND u.id IN (SELECT n FROM seq WHERE n>5) "
"ORDER BY u.id DESC LIMIT 5;";
    test_query_processor(query);
    return 0;
}