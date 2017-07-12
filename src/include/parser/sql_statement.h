//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// sql_statement.h
//
// Identification: src/include/parser/sql_statement.h
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

/*
 * SQLStatement.h
 * Definition of the structure used to build the syntax tree.
 */

#pragma once

#include <iostream>
#include <vector>

#include "common/sql_node_visitor.h"
#include "common/macros.h"
#include "common/printable.h"
#include "type/types.h"
#include "expression/abstract_expression.h"

namespace peloton {

namespace parser {

struct TableInfo {
  ~TableInfo() {
    if (table_name != nullptr) delete[] table_name;
    if (database_name != nullptr) delete[] database_name;
  }
  char* table_name = nullptr;
  ;
  char* database_name = nullptr;
};

/**
 * @struct ColumnDefinition
 * @brief Represents definition of a table column
 */
struct ColumnDefinition {
  enum DataType {
    INVALID,

    PRIMARY,
    FOREIGN,

    CHAR,
    INT,
    INTEGER,
    TINYINT,
    SMALLINT,
    BIGINT,
    DOUBLE,
    FLOAT,
    DECIMAL,
    BOOLEAN,
    ADDRESS,
    TIMESTAMP,
    TEXT,

    VARCHAR,
    VARBINARY
  };

  enum FKConstrActionType {
    NOACTION,
    RESTRICT,
    CASCADE,
    SETNULL,
    SETDEFAULT
  };

  enum FKConstrMatchType {
    SIMPLE,
    PARTIAL,
    FULL
  };

  ColumnDefinition(DataType type) : type(type) {
    // Set varlen to TEXT_MAX_LENGTH if the data type is TEXT
    if (type == TEXT)
      varlen = type::PELOTON_TEXT_MAX_LEN;
  }

  ColumnDefinition(char* name, DataType type) : name(name), type(type) {
    // Set varlen to TEXT_MAX_LENGTH if the data type is TEXT
    if (type == TEXT)
      varlen = type::PELOTON_TEXT_MAX_LEN;
  }

  virtual ~ColumnDefinition() {
    if (primary_key) {
      for (auto key : *primary_key) delete[] (key);
      delete primary_key;
    }

    if (foreign_key_source) {
      for (auto key : *foreign_key_source) delete[] (key);
      delete foreign_key_source;
    }
    if (foreign_key_sink) {
      for (auto key : *foreign_key_sink) delete[] (key);
      delete foreign_key_sink;
    }
    delete[] name;
    if (table_info_ != nullptr)
      delete table_info_;
    if (default_value != nullptr)
      delete default_value;
    if (check_expression != nullptr)
      delete check_expression;
  }

  static type::TypeId GetValueType(DataType type) {
    switch (type) {
      case INT:
      case INTEGER:
        return type::TypeId::INTEGER;
        break;

      case TINYINT:
        return type::TypeId::TINYINT;
        break;
      case SMALLINT:
        return type::TypeId::SMALLINT;
        break;
      case BIGINT:
        return type::TypeId::BIGINT;
        break;

      // case DOUBLE:
      // case FLOAT:
      //  return type::Type::DOUBLE;
      //  break;

      case DECIMAL:
      case DOUBLE:
      case FLOAT:
        return type::TypeId::DECIMAL;
        break;

      case BOOLEAN:
        return type::TypeId::BOOLEAN;
        break;

      // case ADDRESS:
      //  return type::Type::ADDRESS;
      //  break;

      case TIMESTAMP:
        return type::TypeId::TIMESTAMP;
        break;

      case CHAR:
      case TEXT:
      case VARCHAR:
        return type::TypeId::VARCHAR;
        break;

      case VARBINARY:
        return type::TypeId::VARBINARY;
        break;

      case INVALID:
      case PRIMARY:
      case FOREIGN:
      default:
        return type::TypeId::INVALID;
        break;
    }
  }

  char* name = nullptr;

  // The name of the table and its database
  TableInfo* table_info_ = nullptr;

  DataType type;
  size_t varlen = 0;
  bool not_null = false;
  bool primary = false;
  bool unique = false;
  expression::AbstractExpression* default_value = nullptr;
  expression::AbstractExpression* check_expression = nullptr;

  std::vector<char*>* primary_key = nullptr;
  std::vector<char*>* foreign_key_source = nullptr;
  std::vector<char*>* foreign_key_sink = nullptr;

  char* foreign_key_table_name = nullptr;
  FKConstrActionType foreign_key_delete_action;
  FKConstrActionType foreign_key_update_action;
  FKConstrMatchType foreign_key_match_type;
};

// Base class for every SQLStatement
class SQLStatement : public Printable {
 public:
  SQLStatement(StatementType type) : stmt_type(type){};

  virtual ~SQLStatement() {}

  virtual StatementType GetType() { return stmt_type; }

  // Get a string representation for debugging
  const std::string GetInfo() const;

  // Visitor Pattern used for the optimizer to access statements
  // This allows a facility outside the object itself to determine the type of
  // class using the built-in type system.
  virtual void Accept(SqlNodeVisitor* v) const = 0;

 private:
  StatementType stmt_type;
};

class TableRefStatement : public SQLStatement {
 public:
  TableRefStatement(StatementType type) : SQLStatement(type) {}

  virtual ~TableRefStatement() {
    if (table_info_ != nullptr) {
      delete table_info_;
    }
  }

  virtual inline std::string GetTableName() const {
    return table_info_->table_name;
  }

  // Get the name of the database of this table
  virtual inline std::string GetDatabaseName() const {
    if (table_info_->database_name == nullptr) {
      return DEFAULT_DB_NAME;
    }
    return table_info_->database_name;
  }

  TableInfo* table_info_ = nullptr;
};

// Represents the result of the SQLParser.
// If parsing was successful it is a list of SQLStatement.
class SQLStatementList : public Printable {
 public:
  SQLStatementList()
      : is_valid(true), parser_msg(NULL), error_line(0), error_col(0){};

  SQLStatementList(SQLStatement* stmt) : is_valid(true), parser_msg(NULL) {
    AddStatement(stmt);
  };

  virtual ~SQLStatementList() {
    // clean up statements
    for (auto stmt : statements) delete stmt;

    free((char*)parser_msg);
  }

  void AddStatement(SQLStatement* stmt) { statements.push_back(stmt); }

  SQLStatement* GetStatement(int id) const { return statements[id]; }

  const std::vector<SQLStatement*>& GetStatements() const { return statements; }

  size_t GetNumStatements() const { return statements.size(); }

  // Get a string representation for debugging
  const std::string GetInfo() const;

  std::vector<SQLStatement*> statements;
  bool is_valid;
  const char* parser_msg;
  int error_line;
  int error_col;
};

}  // End parser namespace
}  // End peloton namespace
