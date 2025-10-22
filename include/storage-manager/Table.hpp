#pragma once

#include "general/Types.hpp"
#include "general/Page.hpp"
#include "general/Structs.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "storage-manager/HeapFile.hpp"
#include <vector>
#include <string>

namespace DB {
    struct Table {
        string              name;
        Schema*             schema;
        HeapFile*           heapfile;
        std::vector<u64>    theFreePages; 
        
    };

    //create stuff i.e. write into heapfile
    void create_table();

    void load_table();
    void load_schema();
    


    class Table {
        public:
            //initialize new Table
            Table(const string& name, Schema& schema, HeapFile& heapfile);
            //destructor for  removing table FD and closing pipe
            Table*          get_table(const string& name, HeapFile& bufPool); //get table metadata from disk
            //return page number where row is placed. row byte size must equal schema
            RowId               insert_row();
            Row*                read_row();
            std::vector<Row*>   scan() const;

            // void            write_page(u64 pageNum);
            u64             read(u64 pageNum, u16 rowNum);
            string          print_metadata();
            Schema*         get_record(int rid); //takes record id and returns the record

        private:
            const string            theFileName;
            const string            thePath;
            //bool                  theIndex;
            std::vector<u64>        theFreePages; 
            Schema                  theSchema;    
            u64  allocPage(); // grab a free page from file

    }; 
}