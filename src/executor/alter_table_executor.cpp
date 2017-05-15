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
#include "executor/executor_context.h"
#include "common/logger.h"
#include "catalog/catalog.h"

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
        node->GetDatabaseName(), node->GetTableName(), current_txn);
    auto old_schema = old_table->GetSchema();

    // Check which type of alter table it is
    switch (node.GetAlterTableType()) {
      /*
       * ADD COLUMN
       */
      case AlterTableType::AT_AddColumn:
        // Check if column already exists
        for (auto new_column : node->GetSchemaDelta()->GetColumns()) {
          for (auto old_column : old_schema->GetColumns()) {
            if (new_column.GetName() == old_column.GetName()) {
              LOG_TRACE("Add Column FAILURE: Column %s already exists",
                        new_column.GetName().c_str());
              return false;
            }
          }
        }

        // Construct new schema
        auto new_schema =
            catalog::Schema::AppendSchema(old_schema, node->GetSchemaDelta());

        // Copy and replace table content to new schema
        auto result = catalog::Catalog->GetInstance()->AlterTable(
            old_table->GetDatabaseOid(), old_table->GetOid(), new_schema,
            current_txn);
        return (result == ResultType::SUCCESS);

      /*
       * DROP COLUMN
       */
      case AlterTableType::AT_DropColumn:
        // Check if dropped column exists
        std::set<oid_t> drop_column_ids;
        for (auto drop_column : node->GetSchemaDelta()->GetColumns()) {
          oid_t drop_column_id =
              catalog::ColumnCatalog::GetInstance()->GetColumnId(
                  old_table->GetOid(), drop_column.GetName(), txn);
          if (drop_column_id == INVALID_OID) {
            LOG_TRACE("Drop Column FAILURE: Column %s does not exist",
                      drop_column.GetName().c_str());
            return false;
          } else {
            drop_column_ids.insert(drop_column_id);
          }
        }

        // Construct new schema
        std::vector<oid_t> new_column_ids;
        for (oid_t i = 0; i < old_schema->GetColumnCount(); i++) {
          if (drop_column_ids.find(i) == drop_column_ids.end()) {
            new_column_ids.push_back(i);
          }
        }
        auto new_schema =
            catalog::Schema::CopySchema(old_schema, new_column_ids);

        // Copy and replace table content to new schema
        auto result = catalog::Catalog->GetInstance()->AlterTable(
            old_table->GetDatabaseOid(), old_table->GetOid(), new_schema,
            current_txn);
        return (result == ResultType::SUCCESS);

      default:
        LOG_DEBUG("Alter table type %d not implemented",
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
    LOG_TRACE("Can't found database %s. Return RESULT_FAILURE",
              database_name.c_str());
    return false;
  }
  return false;
}
}
}
