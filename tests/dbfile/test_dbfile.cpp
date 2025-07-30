#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>      
#include <unistd.h>
#include <filesystem>
#include <cstdlib>

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
  const char* tmp = std::getenv("TEST_TMPDIR");
  std::string path = "test.db";
  std::cout << "Creating dbfile at: " << path << std::endl;
  Page testPage{Page(-1)};

  EXPECT_NO_THROW({
    DbFile dbFile = DbFile(path, true, testPage);
  });

  ASSERT_EQ(access(path.c_str(), F_OK), 0);
}

TEST(DbFileTest, ThrowIfCannotOpenFile) {
  Page testPage{Page(-1)};
  EXPECT_THROW({
    DbFile("non_existent.db", false, testPage);
  }, std::system_error);
}

TEST() {
  
}

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