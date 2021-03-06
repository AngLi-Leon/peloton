//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// alter_table_plan.cpp
//
// Identification: src/planner/alter_table_plan.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "planner/alter_table_plan.h"

#include "catalog/column.h"
#include "catalog/schema.h"
#include "parser/alter_table_statement.h"
#include "storage/data_table.h"

namespace peloton {
namespace planner {

AlterTablePlan::AlterTablePlan(const std::string &database_name,
                               const std::string &table_name,
                               std::unique_ptr<catalog::Schema> added_columns,
                               const std::vector<std::string> &dropped_columns,
                               AlterTableType a_type)
    : table_name(table_name),
      database_name(database_name),
      added_columns(added_columns.release()),
      dropped_columns(dropped_columns),
      altertable_type(a_type) {}

AlterTablePlan::AlterTablePlan(parser::AlterTableStatement *parse_tree) {
  table_name = std::string(parse_tree->GetTableName());
  database_name = std::string(parse_tree->GetDatabaseName());
  altertable_type = parse_tree->type;
  std::vector<catalog::Column> columns;
  // case 1: add column(column name + column data type)
  if (parse_tree->type == AlterTableType::COLUMN) {
    // Add columns: traverse through vector of ColumnDefinition
    for (auto col : *parse_tree->columns) {
      type::TypeId val = col->GetValueType(col->type);
      LOG_TRACE("Column name: %s", col->name);

      bool is_inline = (val == type::TypeId::VARCHAR) ? false : true;
      auto column = catalog::Column(val, type::Type::GetTypeSize(val),
                                    std::string(col->name), is_inline);
      LOG_TRACE("Column is_line: %d", is_inline);
      // Add not_null constraints
      if (col->not_null) {
        catalog::Constraint constraint(ConstraintType::NOTNULL, "con_not_null");
        column.AddConstraint(constraint);
      }
      columns.push_back(column);
    }
    added_columns = new catalog::Schema(columns);

    // Drop columns: traverse through vector of char*(column name)
    for (auto col : *parse_tree->names) {
      LOG_TRACE("Drooped column name: %s", col);
      dropped_columns.push_back(std::string(col));
    }
  }
}

}  // namespace planner
}  // namespace peloton
