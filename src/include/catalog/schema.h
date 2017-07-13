//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// schema.h
//
// Identification: src/include/catalog/schema.h
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <unordered_map>
#include "boost/algorithm/string.hpp"
#include "catalog/column.h"
#include "common/printable.h"
#include "type/type.h"

namespace peloton {
namespace catalog {

//===--------------------------------------------------------------------===//
// Schema
//===--------------------------------------------------------------------===//

class Schema : public Printable {
 public:
  //===--------------------------------------------------------------------===//
  // Static factory methods to construct schema objects
  //===--------------------------------------------------------------------===//

  // Construct schema from vector of Column
  Schema(const std::vector<Column> &columns);

  // Copy schema
  static std::shared_ptr<const Schema> CopySchema(
      const std::shared_ptr<const Schema> &schema);

  // Copy subset of columns in the given schema
  static std::shared_ptr<const Schema> CopySchema(
      const std::shared_ptr<const Schema> &schema,
      const std::vector<oid_t> &set);

  // Backward compatible for raw pointers
  // Copy schema
  static Schema *CopySchema(const Schema *schema);

  // Copy subset of columns in the given schema
  static Schema *CopySchema(const Schema *schema,
                            const std::vector<oid_t> &index_list);

  // Append two schema objects
  static Schema *AppendSchema(Schema *first, Schema *second);

  // Append subset of columns in the two given schemas
  static Schema *AppendSchema(Schema *first, std::vector<oid_t> &first_set,
                              Schema *second, std::vector<oid_t> &second_set);

  // Append given schemas.
  static Schema *AppendSchemaList(std::vector<Schema> &schema_list);

  // Append given schemas.
  static Schema *AppendSchemaPtrList(const std::vector<Schema *> &schema_list);

  // Append subsets of columns in the given schemas.
  static Schema *AppendSchemaPtrList(
      const std::vector<Schema *> &schema_list,
      const std::vector<std::vector<oid_t>> &subsets);

  // Compare two schemas
  bool operator==(const Schema &other) const;
  bool operator!=(const Schema &other) const;

  //===--------------------------------------------------------------------===//
  // Schema accessors
  //===--------------------------------------------------------------------===//

  inline size_t GetOffset(const oid_t column_id) const {
    return columns[column_id].GetOffset();
  }

  inline type::TypeId GetType(const oid_t column_id) const {
    return columns[column_id].GetType();
  }

  // Return appropriate length based on whether column is inlined
  inline size_t GetAppropriateLength(const oid_t column_id) const {
    auto is_inlined = columns[column_id].IsInlined();
    size_t column_length;

    if (is_inlined) {
      column_length = GetLength(column_id);
    } else {
      column_length = GetVariableLength(column_id);
    }

    return column_length;
  }

  // Returns fixed length
  inline size_t GetLength(const oid_t column_id) const {
    return columns[column_id].GetFixedLength();
  }

  inline size_t GetVariableLength(const oid_t column_id) const {
    return columns[column_id].GetVariableLength();
  }

  inline bool IsInlined(const oid_t column_id) const {
    return columns[column_id].IsInlined();
  }

  inline const Column GetColumn(const oid_t column_id) const {
    return columns[column_id];
  }

  inline oid_t GetColumnID(std::string col_name) const {
    oid_t index = -1;
    for (oid_t i = 0; i < columns.size(); ++i) {
      if (columns[i].GetName() == col_name) {
        index = i;
        break;
      }
    }
    return index;
  }

  inline oid_t GetUninlinedColumn(const oid_t column_id) const {
    return uninlined_columns[column_id];
  }

  inline const std::vector<Column> &GetColumns() const { return columns; }

  // Return the number of columns in the schema for the tuple.
  inline size_t GetColumnCount() const { return columns.size(); }

  inline oid_t GetUninlinedColumnCount() const {
    return uninlined_columns.size();
  }

  // Return the number of bytes used by one tuple.
  inline oid_t GetLength() const { return length; }

  // Returns a flag indicating whether all columns are inlined
  inline bool IsInlined() const { return tuple_is_inlined; }

  inline void SetIndexedColumns(const std::vector<oid_t> &indexed_columns) {
    indexed_columns_ = indexed_columns;
  }

  inline const std::vector<oid_t> GetIndexedColumns() const {
    return indexed_columns_;
  }

  // Get the nullability of the column at a given index.
  inline bool AllowNull(const oid_t column_id) const {
    for (auto constraint : columns[column_id].GetConstraints()) {
      if (constraint.GetType() == ConstraintType::NOTNULL) return false;
    }
    return true;
  }

  // Add constraint for column by id
  inline void AddConstraint(oid_t column_id,
                            const catalog::Constraint &constraint) {
    columns[column_id].AddConstraint(constraint);
  }

  // Add constraint for column by name
  inline void AddConstraint(std::string column_name,
                            const catalog::Constraint &constraint) {
    for (size_t column_itr = 0; column_itr < columns.size(); column_itr++) {
      if (columns[column_itr].GetName() == column_name) {
        columns[column_itr].AddConstraint(constraint);
      }
    }
  }

  // Convert logical id to physical id
  inline size_t LogicToPhysic(oid_t column_id) {
    auto it = logic2physic.find(column_id);
    if (it == logic2physic.end()) {
      return -1;
    } else {
      return it->second;
    }
  }

  // Get a string representation for debugging
  const std::string GetInfo() const;

 private:
  // size of fixed length columns
  size_t length;

  // all inlined and uninlined columns in the tuple, indexed by (physical id)
  std::vector<Column> columns;

  // keeps track of unlined columns (physical id)
  std::vector<oid_t> uninlined_columns;

  // Deprecated, this seems to duplicated information as columns.size()
  // // keep these in sync with the vectors above
  // oid_t column_count = INVALID_OID;

  // oid_t uninlined_column_count = INVALID_OID;

  // are all columns inlined
  bool tuple_is_inlined;

  // keeps track of indexed columns in original table (physical id)
  std::vector<oid_t> indexed_columns_;

  // logical id to physical id mapping
  std::unordered_map<oid_t, oid_t> logic2physic;
};

}  // namespace catalog
}  // namespace peloton
