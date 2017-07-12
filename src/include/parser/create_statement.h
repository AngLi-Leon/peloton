//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// statement_create.h
//
// Identification: src/include/parser/statement_create.h
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "type/types.h"
#include "common/sql_node_visitor.h"
#include "parser/sql_statement.h"
#include "expression/abstract_expression.h"

namespace peloton {
namespace parser {


/**
 * @struct CreateStatement
 * @brief Represents "CREATE TABLE students (name TEXT, student_number INTEGER,
 * city TEXT, grade DOUBLE)"
 */
struct CreateStatement : TableRefStatement {
  enum CreateType { kTable, kDatabase, kIndex };

  CreateStatement(CreateType type)
      : TableRefStatement(StatementType::CREATE),
        type(type),
        if_not_exists(false),
        columns(nullptr){};

  virtual ~CreateStatement() {
    if (columns != nullptr) {
      for (auto col : *columns) delete col;
      delete columns;
    }

    if (index_attrs != nullptr) {
      for (auto attr : *index_attrs) delete[](attr);
      delete index_attrs;
    }

    if (index_name != nullptr) {
      delete[](index_name);
    }
    if (database_name != nullptr) {
      delete[](database_name);
    }
  }

  virtual void Accept(SqlNodeVisitor* v) const override { v->Visit(this); }

  CreateType type;
  bool if_not_exists;

  std::vector<ColumnDefinition*>* columns;
  std::vector<char*>* index_attrs = nullptr;

  IndexType index_type;

  char* index_name = nullptr;
  char* database_name = nullptr;

  bool unique = false;
};

}  // End parser namespace
}  // End peloton namespace
