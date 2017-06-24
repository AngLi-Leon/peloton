//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// schema.cpp
//
// Identification: src/catalog/schema.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "catalog/schema.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "common/logger.h"
#include "common/macros.h"

namespace peloton {
namespace catalog {

// Construct schema from vector of Column
Schema::Schema(const std::vector<Column> &columns) : tuple_is_inlined(true) {
  oid_t column_offset = 0;
  for (oid_t physical_id = 0; physical_id < columns.size(); physical_id++) {
    auto column = columns[physical_id];

    // handle uninlined column
    if (column.IsInlined() == false) {
      tuple_is_inlined = false;
      uninlined_columns.push_back(physical_id);
    }

    // set logical id to physical id map, check logical id
    auto logical_id = column.logical_id;
    if (logical_id == INVALID_OID) {
      LOG_ERROR("Invalid oid when creating schema!");
    }

    auto it = logic2physic.find(logical_id);
    if (it == logic2physic.end()) {
      logic2physic[logical_id] = physical_id;
    } else {
      LOG_ERROR("Duplicated oid %d when creating schema!", (int)logical_id);
    }

    // set column offset
    column.column_offset = column_offset;
    column_offset += column.GetFixedLength();

    // add column
    this->columns.push_back(column);
  }
}

// Copy schema
std::shared_ptr<const Schema> Schema::CopySchema(
    const std::shared_ptr<const Schema> &schema) {
  oid_t column_count = schema->GetColumnCount();
  std::vector<oid_t> physical_ids;

  for (oid_t physical_id = 0; physical_id < column_count; physical_id++) {
    physical_ids.push_back(physical_id);
  }

  return CopySchema(schema, physical_ids);
}

// Copy subset of columns in the given schema
std::shared_ptr<const Schema> Schema::CopySchema(
    const std::shared_ptr<const Schema> &schema,
    const std::vector<oid_t> &physical_ids) {
  std::vector<Column> columns;
  columns.reserve(physical_ids.size());

  for (auto physical_id : physical_ids) {
    PL_ASSERT(physical_id < schema->columns.size());
    columns.push_back(schema->columns[physical_id]);
  }

  // preserve max logical id when copying subset of schema
  return std::shared_ptr<Schema>(new Schema(columns));
}

// Backward compatible for raw pointers
// Copy schema
Schema *Schema::CopySchema(const Schema *schema) {
  oid_t column_count = schema->GetColumnCount();
  std::vector<oid_t> physical_ids;

  for (oid_t physical_id = 0; physical_id < column_count; physical_id++) {
    physical_ids.push_back(physical_id);
  }

  return CopySchema(schema, physical_ids);
}

/*
 * CopySchema() - Copies the schema into a new schema object with index_list
 *                as indices to copy
 *
 * This function essentially does a "Gathering" operation, in a sense that it
 * collects columns indexed by elements inside index_list in the given order
 * and store them inside a newly created schema object
 *
 * If there are duplicates in index_list then the columns will be duplicated
 * (i.e. no dup checking will be done)
 *
 * If the indices inside index_list >= the size of the column list then behavior
 * is undefined (and is likely to crash)
 *
 * The returned schema is created by new operator, and the caller is responsible
 * for destroying it.
 */
Schema *Schema::CopySchema(const Schema *schema,
                           const std::vector<oid_t> &physical_ids) {
  std::vector<Column> column_list;

  // Reserve some space to avoid multiple ma110c() calls
  // But for future push_back() this is not optimized since the
  // memory chunk may not be properly sized and aligned
  column_list.reserve(physical_ids.size());

  // For each column index, push the column
  for (oid_t physical_id : physical_ids) {
    // Make sure the index does not refer to invalid element
    PL_ASSERT(physical_id < schema->columns.size());

    column_list.push_back(schema->columns[physical_id]);
  }

  Schema *ret_schema = new Schema(column_list);

  return ret_schema;
}

// Append two schema objects
Schema *Schema::AppendSchema(Schema *first, Schema *second) {
  return AppendSchemaPtrList({first, second});
}

// Append subset of columns in the two given schemas
Schema *Schema::AppendSchema(Schema *first, std::vector<oid_t> &first_set,
                             Schema *second, std::vector<oid_t> &second_set) {
  const std::vector<Schema *> schema_list({first, second});
  const std::vector<std::vector<oid_t>> subsets({first_set, second_set});
  return AppendSchemaPtrList(schema_list, subsets);
}

// Append given schemas.
Schema *Schema::AppendSchemaList(std::vector<Schema> &schema_list) {
  // All we do here is convert vector<Schema> to vector<Schema *>.
  // This is a convenience function.
  std::vector<Schema *> schema_ptr_list;
  for (unsigned int i = 0; i < schema_list.size(); i++) {
    schema_ptr_list.push_back(&schema_list[i]);
  }
  return AppendSchemaPtrList(schema_ptr_list);
}

// Append given schemas.
Schema *Schema::AppendSchemaPtrList(const std::vector<Schema *> &schema_list) {
  std::vector<std::vector<oid_t>> subsets;

  for (unsigned int i = 0; i < schema_list.size(); i++) {
    oid_t column_count = schema_list[i]->GetColumnCount();
    std::vector<oid_t> subset;
    for (oid_t column_itr = 0; column_itr < column_count; column_itr++) {
      subset.push_back(column_itr);
    }
    subsets.push_back(subset);
  }

  return AppendSchemaPtrList(schema_list, subsets);
}

// Append subsets of columns in the given schemas.
Schema *Schema::AppendSchemaPtrList(
    const std::vector<Schema *> &schema_list,
    const std::vector<std::vector<oid_t>> &subsets) {
  PL_ASSERT(schema_list.size() == subsets.size());

  std::vector<Column> columns;
  for (unsigned int i = 0; i < schema_list.size(); i++) {
    Schema *schema = schema_list[i];
    const std::vector<oid_t> &subset = subsets[i];

    for (oid_t physical_id : subset) {
      PL_ASSERT(physical_id < schema->columns.size());
      columns.push_back(schema->columns[physical_id]);
    }
  }

  Schema *ret_schema = new Schema(columns);

  return ret_schema;
}

const std::string Schema::GetInfo() const {
  std::ostringstream os;

  os << "Schema["
     << "NumColumns:" << columns.size() << ", "
     << "IsInlined:" << tuple_is_inlined << ", "
     << "Length:" << length << ", "
     << "UninlinedCount:" << uninlined_columns.size() << "]";

  bool first = true;
  os << " :: (";
  for (oid_t i = 0; i < columns.size(); i++) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    os << columns[i].GetInfo();
  }
  os << ")";

  return os.str();
}

// Compare two schemas
bool Schema::operator==(const Schema &other) const {
  if (other.GetColumnCount() != GetColumnCount() ||
      other.GetUninlinedColumnCount() != GetUninlinedColumnCount() ||
      other.IsInlined() != IsInlined()) {
    return false;
  }

  for (oid_t column_itr = 0; column_itr < other.GetColumnCount();
       column_itr++) {
    const Column &column_info = other.GetColumn(column_itr);
    const Column &other_column_info = GetColumn(column_itr);

    if (column_info != other_column_info) {
      return false;
    }
  }

  return true;
}

bool Schema::operator!=(const Schema &other) const { return !(*this == other); }

}  // namespace catalog
}  // namespace peloton
