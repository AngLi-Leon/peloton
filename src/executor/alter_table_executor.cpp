//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// alter_table_executor.cpp
//
// Identification: src/executor/alter_table_executor.cpp
//
// Copyright (c) 2015-17, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "executor/alter_table_executor.h"
#include "catalog/catalog.h"
#include "common/logger.h"
#include "executor/executor_context.h"

#include <vector>

namespace peloton {
namespace executor {

// Constructor for drop executor
AlterTableExecutor::AlterTableExecutor(const planner::AbstractPlan *node,
                                       ExecutorContext *executor_context)
    : AbstractExecutor(node, executor_context) {
  context = executor_context;
}

// Initialize executer
// Nothing to initialize now
bool AlterTableExecutor::DInit() {
  LOG_TRACE("Initializing Alter Table Executer...");
  LOG_TRACE("Alter Table Executer initialized!");
  return true;
}

bool AlterTableExecutor::DExecute() {
  LOG_TRACE("Executing Alter Table...");
  const planner::AlterTablePlan &node = GetPlanNode<planner::AlterTablePlan>();
  auto current_txn = context->GetTransaction();

  // Get old schema
  try {
    auto old_table = catalog::Catalog::GetInstance()->GetTableWithName(
        node.GetDatabaseName(), node.GetTableName(), current_txn);
    auto old_schema = old_table->GetSchema();

    // Check which type of alter table it is
    if (node.GetAlterTableType() == AlterTableType::COLUMN) {
      /*
       * DROP COLUMN
       */
      // Construct new schema
      std::vector<oid_t> new_column_ids;
      for (oid_t i = 0; i < old_schema->GetColumnCount(); i++) {
        bool is_found = false;
        for (auto drop_column : node.GetDroppedColumns()) {
          if (old_schema->GetColumn(i).GetName() == drop_column) {
            is_found = true;
            // delete record in pg_attribute
            // catalog::ColumnCatalog::GetInstance()->DeleteColumn(
            //     old_table->GetOid(), drop_column, current_txn);
          }
        }
        if (!is_found) {
          new_column_ids.push_back(i);
        }
      }
      // Check if dropped column exists
      if (new_column_ids.size() + node.GetDroppedColumns().size() !=
          old_schema->GetColumnCount()) {
        current_txn->SetResult(ResultType::FAILURE);
        return false;
      }
      std::unique_ptr<catalog::Schema> temp_schema(
          catalog::Schema::CopySchema(old_schema, new_column_ids));

      /*
       * ADD COLUMN
       */
      size_t column_offset = old_schema->GetColumnCount();
      // Check if column already exists
      for (auto new_column : node.GetAddedColumns()->GetColumns()) {
        for (auto old_column : old_schema->GetColumns()) {
          if (new_column.GetName() == old_column.GetName()) {
            LOG_TRACE("Add Column FAILURE: Column %s already exists",
                      new_column.GetName().c_str());
            current_txn->SetResult(ResultType::FAILURE);
            return false;
          }
        }
        // catalog::ColumnCatalog::GetInstance()->InsertColumn(
        //     old_table->GetOid(), new_column.GetName(), column_offset,
        //     new_column.GetOffset(), new_column.GetType(),
        //     new_column.IsInlined(), new_column.GetConstraints(), nullptr,
        //     current_txn);
        column_offset++;
      }

      // Construct new schema
      std::unique_ptr<catalog::Schema> new_schema(catalog::Schema::AppendSchema(
          temp_schema.get(), node.GetAddedColumns()));

      // Copy and replace table content to new schema
      auto result = catalog::Catalog::GetInstance()->AlterTable(
          old_table->GetDatabaseOid(), old_table->GetOid(),
          std::move(new_schema), current_txn);
      current_txn->SetResult(result);
    } else {
      LOG_TRACE("Alter table type %d not implemented",
                (int)node.GetAlterTableType());
    }

    if (current_txn->GetResult() == ResultType::SUCCESS) {
      LOG_TRACE("Alter table succeeded!");
    } else if (current_txn->GetResult() == ResultType::FAILURE) {
      LOG_TRACE("Alter table failed!");
    } else {
      LOG_TRACE("Result is: %s",
                ResultTypeToString(current_txn->GetResult()).c_str());
    }
  } catch (CatalogException &e) {
    LOG_TRACE("Can't found table %s. Return RESULT_FAILURE",
              node.GetTableName().c_str());
    return false;
  }
  return false;
}
}
}
