#include "Database.hpp"
#include "Table.hpp"
#include "Types.hpp"
#include "Page.hpp"

namespace db {
    Database::Database(string name) :
        theName(name),
        theHeaderPage(new Page(0)),
        theFreePages(),
        theTables(),
        theLogger(logger),
        thePageCache(cache)
    {}

    void update_header_page(Table& table) {
        std::cout << "print table path and name in the stuff for indexing." << std::endl;
    }

    Database::create_table(const string tableName, Orientation type, const Schema& schema) {
        Table table = Table(tableName, type, Schema);
        theTables.push_back();
    }

    Database::write_header_page() {
        return 1;
        //thePageCache.write_through();
    }
   
}

