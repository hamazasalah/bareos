/*
   BAREOS® - Backup Archiving REcovery Open Sourced

   Copyright (C) 2020-2020 Bareos GmbH & Co. KG

   This program is Free Software; you can redistribute it and/or
   modify it under the terms of version three of the GNU Affero General Public
   License as published by the Free Software Foundation and included
   in the file LICENSE.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef BAREOS_SRC_DIRD_DBCOPY_DATABASE_TABLES_H_
#define BAREOS_SRC_DIRD_DBCOPY_DATABASE_TABLES_H_

#include "dird/dbcopy/database_column_descriptions.h"
#include "dird/dbcopy/database_connection.h"
#include "include/make_unique.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

class BareosDb;

class DatabaseTableDescriptions {
 public:
  class TableDescription {
   public:
    TableDescription(std::string&& t,
                     std::vector<std::unique_ptr<ColumnDescription>>&& r)
        : table_name(t), column_descriptions(std::move(r)){};

    std::string table_name;
    DatabaseColumnDescriptions::VectorOfColumnDescriptions column_descriptions;
  };

 public:
  std::vector<TableDescription> tables;

  static std::unique_ptr<DatabaseTableDescriptions> Create(
      const DatabaseConnection& connection);

  const TableDescription* GetTableDescription(
      const std::string& table_name) const;

  const ColumnDescription* GetColumnDescription(
      const std::string& table_name,
      const std::string& column_name) const;

  virtual ~DatabaseTableDescriptions() = default;

 protected:
  DatabaseTableDescriptions(BareosDb* db) : db_{db} {}

  void SelectTableNames(const std::string& sql_query,
                        std::vector<std::string>& tables_names);

 private:
  BareosDb* db_{};
  static int ResultHandler(void* ctx, int fields, char** row);
};

class DatabaseTablesPostgresql : public DatabaseTableDescriptions {
 public:
  DatabaseTablesPostgresql(BareosDb* db);
};

class DatabaseTablesMysql : public DatabaseTableDescriptions {
 public:
  DatabaseTablesMysql(BareosDb* db);
};

#endif  // BAREOS_SRC_DIRD_DBCOPY_DATABASE_TABLES_H_
