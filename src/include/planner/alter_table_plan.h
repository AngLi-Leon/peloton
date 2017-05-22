//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// AlterTable_plan.h
//
// Identification: src/include/planner/alter_table_plan.h
//
// Copyright (c) 2015-17, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "planner/abstract_plan.h"

namespace peloton {
namespace catalog {
class Schema;
}
namespace storage {
class DataTable;
}
namespace parser {
class AlterTableStatement;
}

namespace planner {

class AlterTablePlan : public AbstractPlan {
 public:
  AlterTablePlan() = delete;

  explicit AlterTablePlan(const std::string &database_name,
                          const std::string &table_name,
                          std::unique_ptr<catalog::Schema> added_columns,
                          const std::vector<std::string> &dropped_columns,
                          AlterTableType a_type);

  explicit AlterTablePlan(parser::AlterTableStatement *parse_tree);

  inline PlanNodeType GetPlanNodeType() const {
    return PlanNodeType::ALTER_TABLE;
  }

  const std::string GetInfo() const { return "AlterTable Plan"; }

  std::unique_ptr<AbstractPlan> Copy() const {
    return std::unique_ptr<AbstractPlan>(
        new AlterTablePlan(database_name, table_name,
                           std::unique_ptr<catalog::Schema>(
                               catalog::Schema::CopySchema(added_columns)),
                           dropped_columns, altertable_type));
  }

  std::string GetTableName() const { return table_name; }

  std::string GetDatabaseName() const { return database_name; }

  catalog::Schema *GetAddedColumns() const { return added_columns; }

  const std::vector<std::string> &GetDroppedColumns() const {
    return dropped_columns;
  }

  AlterTableType GetAlterTableType() const { return altertable_type; }

 private:
  // Target Table
  storage::DataTable *target_table_ = nullptr;

  // Table Name
  std::string table_name;

  // Database Name
  std::string database_name;

  // Schema delta, define the column txn want to add
  catalog::Schema *added_columns;
  // dropped_column, define the column you want to drop
  std::vector<std::string> dropped_columns;

  // Check to either AlterTable Table or INDEX
  AlterTableType altertable_type;

 private:
  DISALLOW_COPY_AND_MOVE(AlterTablePlan);
};

}  // namespace planner
}  // namespace peloton
