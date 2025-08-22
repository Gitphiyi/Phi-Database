#include "compiler/QueryProcessor.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace DB;

// class QueryProcessorTest : public testing::Test {
//   protected:
//   // QueryProcessor qp = QueryProcessor();
// };
TEST(QueryProcessorTest, NullTokenTest) {
  string query = "";
  std::vector<Token> tokens = tokenize_query(query);
  EXPECT_EQ(tokens.size(), 0);
}

TEST(QueryProcessorTest, BasicSQLTest) {
  string query = "SELECT * from table Where 1 = 1;";
  std::vector<Token> tokens = tokenize_query(query);
  std::cout<< "stuff: ";
  for(Token t : tokens) {
    std::cout << t.value << ", ";
  }
  std::cout << std::endl;
  EXPECT_EQ(tokens.size(), 9);
}