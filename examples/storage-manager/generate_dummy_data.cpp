#include "page-manager/DbFile.hpp"
#include "storage-manager/HeapFile.hpp"
#include "general/Structs.hpp"
#include <iostream>
#include <random>
#include <ctime>

using namespace DB;

// List of country names
const std::vector<std::string> COUNTRIES = {
    "USA", "Canada", "Mexico", "Brazil", "Argentina",
    "UK", "France", "Germany", "Italy", "Spain",
    "Russia", "China", "Japan", "India", "Australia",
    "Egypt", "Nigeria", "Kenya", "Morocco", "Ghana",
    "Sweden", "Norway", "Finland", "Denmark", "Poland",
    "Turkey", "Iran", "Iraq", "Israel", "UAE",
    "Thailand", "Vietnam", "Indonesia", "Malaysia", "Singapore",
    "Chile", "Peru", "Colombia", "Venezuela", "Cuba"
};

int main() {
    std::cout << "=== Generating Dummy Table Data ===\n\n";

    // Initialize random number generator
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> random_value(1, 10000);
    std::uniform_int_distribution<size_t> random_country(0, COUNTRIES.size() - 1);

    // Initialize database
    DbFile::initialize(false);

    // Create the heapfile for our dummy table
    std::string tablename = "DUMMY_TABLE";
    std::cout << "Creating heapfile for table: " << tablename << "\n";
    HeapFile* heapfile = create_heapfile(tablename);

    // Generate and insert rows
    // Schema: <id: INT, random_value: INT, country: STRING>
    const int NUM_ROWS = 150; // Enough to span multiple pages

    std::cout << "\nInserting " << NUM_ROWS << " rows...\n";
    std::cout << "Schema: <id: INT, random_value: INT, country: STRING>\n\n";

    u32 current_page = 1;
    int rows_on_current_page = 0;

    for (int i = 1; i <= NUM_ROWS; i++) {
        // Create row values: id, random int, random country
        std::vector<datatype> values;
        values.push_back(i);                                    // id (INT)
        values.push_back(random_value(rng));                    // random value (INT)
        values.push_back(COUNTRIES[random_country(rng)]);       // country name (STRING)

        Row* row = create_row(3, std::move(values));

        // Insert the row
        RowId rid = insert_row(heapfile, row, current_page);

        // Track page changes for multi-page filling
        if (rid.pageId.page_num != current_page) {
            std::cout << "  Page " << current_page << " filled with " << rows_on_current_page << " rows\n";
            current_page = rid.pageId.page_num;
            rows_on_current_page = 1;
        } else {
            rows_on_current_page++;
        }

        // Print progress every 25 rows
        if (i % 25 == 0) {
            std::cout << "  Inserted " << i << " rows...\n";
        }

        delete row;
    }

    std::cout << "  Page " << current_page << " filled with " << rows_on_current_page << " rows\n";
    std::cout << "\nTotal rows inserted: " << NUM_ROWS << "\n";
    std::cout << "Total pages used: " << current_page << "\n";

    // Print heapfile metadata
    std::cout << "\n=== Heapfile Metadata ===\n";
    print_heapfile_metadata(heapfile);

    // Verify by scanning and printing some rows
    std::cout << "\n=== Verifying Data (First 10 rows) ===\n";
    std::vector<Row*> all_rows = scan_heap(heapfile);
    std::cout << "Scanned " << all_rows.size() << " rows from heapfile\n\n";

    int count = 0;
    for (Row* r : all_rows) {
        if (count >= 10) break;

        int id = std::get<int>(r->values[0]);
        int val = std::get<int>(r->values[1]);
        std::string country = std::get<std::string>(r->values[2]);

        std::cout << "  Row " << count + 1 << ": id=" << id
                  << ", value=" << val
                  << ", country=" << country << "\n";
        count++;
    }

    // Cleanup
    for (Row* r : all_rows) {
        delete r;
    }
    delete heapfile;

    std::cout << "\n=== Done! ===\n";
    std::cout << "Heapfile created at: database-files/heapfiles/" << tablename << ".db\n";

    return 0;
}
