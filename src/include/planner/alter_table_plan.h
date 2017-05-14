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
// namespace parser {
// class AlterTableStatement;
// }

namespace planner {

class AlterTablePlan : public AbstractPlan {
 public:
  AlterTablePlan() = delete;

  explicit AlterTablePlan(std::string &database_name, std::string &table_name,
                          std::unique_ptr<catalog::Column> schema_delta,
                          AlterTableType c_type);

  // explicit AlterTablePlan(parser::AlterTableStatement *parse_tree);

  inline PlanNodeType GetPlanNodeType() const {
    return PlanNodeType::AlterTable;
  }

  const std::string GetInfo() const { return "AlterTable Plan"; }

  std::unique_ptr<AbstractPlan> Copy() const {
    return std::unique_ptr<AbstractPlan>(new AlterTablePlan(target_table_));
  }

  std::string GetTableName() const { return table_name; }

  std::string GetDatabaseName() const { return database_name; }

  catalog::Schema *GetSchema() const { return table_schema; }

  AlterTableType GetAlterTableType() const { return AlterTable_type; }

  bool IsUnique() const { return unique; }

  IndexType GetIndexType() const { return index_type; }

  std::vector<std::string> GetIndexAttributes() const { return index_attrs; }

 private:
  // Target Table
  storage::DataTable *target_table_ = nullptr;

  // Table Name
  std::string table_name;

  // Database Name
  std::string database_name;

  // Table Schema
  catalog::Column *schema_delta;

  // Check to either AlterTable Table or INDEX
  AlterTableType AlterTable_type;

 private:
  DISALLOW_COPY_AND_MOVE(AlterTablePlan);
};

}  // namespace planner
}  // namespace peloton