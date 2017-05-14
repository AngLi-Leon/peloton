//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// create_executor.cpp
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
  LOG_TRACE("Initializing Create Executer...");
  LOG_TRACE("Create Executer initialized!");
  return true;
}

bool AlterTableExecutor::DExecute() {
  LOG_TRACE("Executing Alter Table...");
  const planner::CreatePlan &node = GetPlanNode<planner::CreatePlan>();
  auto current_txn = context->GetTransaction();

  // Check if query was for creating table
  switch (node.GetAlterTableType()) {
    case AlterTableType::AT_AddColumn:
    case AlterTableType::AT_DropColumn:
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
  return false;
}
}
}
