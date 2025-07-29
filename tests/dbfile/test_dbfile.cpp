#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>      
#include "Page.hpp"
#include "DbFile.hpp"

using namespace DB;

//bazel test --cxxopt=-std=c++17 --test_output=all //tests/dbfile:dbfile_test

class DbFileTest : public testing::Test {
    protected:
      std::string testPath = "test_dbfile.db";
      Page testPage{Page(-1)};
      DbFile* testDbFile = nullptr;

      void SetUp() override {
        DbFile::initialize(testPath, true, testPage);
        testDbFile = &DbFile::getInstance();
      }

      // void TearDown() override {
      //   // Cleanup the test file
      //   std::remove(testFile.c_str());
      // }
};

TEST(DbFileTest, InitializeDbFileSuccessfully) {
  std::string path = "test.db";
  std::cout << "Creating dbfile at: " << path << std::endl;
  Page testPage{Page(-1)};
  DbFile dbFile = DbFile(path, true, testPage);
}

// TEST_F(DbFileTest, CreateFileSuccessfully) {
//     EXPECT_NO_THROW({
//         DbFile db(testFile, true, 0644);
//     });
// }

// TEST_F(DbFileTest, ThrowIfCannotOpenFile) {
//     // Try to open a file in read/write mode that doesn't exist
//     EXPECT_THROW({
//         DbFile db("non_existent.db", false, 0644);
//     }, std::system_error);
// }

// TEST_F(DbFileTest, WriteAndReadData) {
//     DbFile db(testFile, true, 0644);

//     const char writeData[] = "HelloDbFile";
//     char readData[20] = {0};

//     // Write data
//     ssize_t written = db.write_at((void*)writeData, 0, sizeof(writeData));
//     ASSERT_EQ(written, sizeof(writeData));

//     // Read data
//     ssize_t read = db.read_at(readData, 0, sizeof(writeData));
//     ASSERT_EQ(read, sizeof(writeData));
//     EXPECT_STREQ(readData, writeData);
// }

// TEST_F(DbFileTest, ShortReadReportsError) {
//     DbFile db(testFile, true, 0644);

//     char buffer[10];
//     ssize_t read = db.read_at(buffer, 0, 10);  // reading from empty file
//     EXPECT_EQ(read, -1);
// }

// TEST_F(DbFileTest, CloseDoesNotThrow) {
//     DbFile db(testFile, true, 0644);
//     EXPECT_NO_THROW(db.close());
// }