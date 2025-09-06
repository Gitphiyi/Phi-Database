#pragma once

#include "storage-manager/Table.hpp"

#include <tuple>
#include <vector>

namespace DB {
    enum class RAOps {
        Selection,
        Projection,
        Renaming,
        Union,
        Difference,
        Intersection,
        Cross_Product,
        Join,
        Natural_Join
    };

    // Vectorized model of Operations
    // Operations act on the return of next() from other operations
    struct StorageOps {
        virtual ~StorageOps() = default;
        //set to 0 means pure virtual function
        virtual void open() = 0; // initializes resources
        virtual std::vector<Row*> next() = 0; // produces block of tuples (size of cacheline and does operation on it)
        virtual void close() = 0 ;// closes all the resources
        void print(std::vector<Row*>& output);
    };

    class SeqScan : public StorageOps {
        public:
            SeqScan(const Table& table, size_t batchSize);

            void open() override;
            std::vector<Row*> next() override;
            void close() override;

        private:
            const Table& table;
            size_t batchSize;
            size_t cursor;
    };


    class Selection : public StorageOps {
        public: 
            Selection(StorageOps* child, std::function<bool(const Row*)> cond);
            void open() override;
            void close() override;
        protected:
            StorageOps*                         childOp;
            std::function<bool(const Row&)>     condition;
    };

    // struct Join : StorageOps {
    //     Join(Table);
    // };
}
