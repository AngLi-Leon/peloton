//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// alter_table_statement.h
//
// Identification: src/include/parser/alter_table_statement.h
//
// Copyright (c) 2015-17, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "type/types.h"
#include "common/sql_node_visitor.h"
#include "parser/sql_statement.h"

namespace peloton {
namespace parser {

/**
 * @struct AlterTableStatement
 * @brief Represents "ALTER TABLE add column COLUMN_NAME COLUMN_TYPE"
 */
struct AlterTableStatement : TableRefStatement {
  AlterTableStatement(AlterTableType type)
      : TableRefStatement(StatementType::ALTER),
        type(type),
        names(new std::vector<char*>),
        columns(new std::vector<ColumnDefinition*>()){};

  virtual ~AlterTableStatement() {
    if (columns != nullptr) {
      for (auto col : *columns) delete col;
      delete columns;
    }
    if (names != nullptr) {
      for (auto name : *names) delete name;
      delete names;
    }
  }

  virtual void Accept(SqlNodeVisitor* v) const override { v->Visit(this); }

  AlterTableType type;

  // Dropped columns
  std::vector<char*>* names;

  // Added columns
  std::vector<ColumnDefinition*>* columns;
};

}  // End parser namespace
}  // End peloton namespace
