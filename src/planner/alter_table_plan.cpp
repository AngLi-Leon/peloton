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

AlterTablePlan::AlterTablePlan(std::string &database_name,
                               std::string &table_name,
                               std::unique_ptr<catalog::Schema> schema_delta,
                               AlterTableType c_type)
    : table_name(table_name),
      database_name(database_name),
      schema_delta(schema_delta.release()),
      altertable_type(c_type) {}

AlterTablePlan::AlterTablePlan(parser::AlterTableStatement *parse_tree) {
  table_name = parse_tree->GetTableName();
  database_name = parse_tree->GetDatabaseName();
  std::vector<catalog::Column> columns;
  // case 1: add column(column name + column data type)
  if (parse_tree->type == parse_tree->CreateType::kTable) {
    altertable_type = AlterTableType::ADDCOLUMN;
    // tranverse through vector of ColumnDefinition
    for (auto col : *parse_tree->columns) {
      type::Type::TypeId val = col->GetValueType(col->type);
      LOG_TRACE("Column name: %s", col->name);

      bool is_inline = (val == type::Type::VARCHAR) ? false : true;
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
    catalog::Schema *schema = new catalog::Schema(columns);
    schema_delta = schema;
  }
  // case 2: drop column(column name)
  // case 1 and case 2 can exsit within one sql statement
  if (parse_tree->type == parse_tree->CreateType::kIndex) {
    altertable_type = AlterTableType::DROPCOLUMN;
    // tranverse through vector of char*(column name)
    for (auto col : *parse_tree->names) {
      LOG_TRACE("Drooped column name: %s", col.c_str());
      dropped_column.push_back(col);
    }
  }
}

}  // namespace planner
}  // namespace peloton
