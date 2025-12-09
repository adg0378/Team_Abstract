#pragma once

#include <map>
#include "CoreMinimal.h"

// Forward declarations
struct sqlite3;
struct sqlite3_stmt;
class USPLogger;

using MigrationMap = std::map<int, std::vector<std::string>>;

const std::string VERSION_TABLE_SCHEMA =
"CREATE TABLE IF NOT EXISTS tbl_version (\n"
"	id_pk INTEGER PRIMARY KEY,\n"
"	version INTEGER NOT NULL,\n"
"	migrated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP\n"
");";

/**
 * Utility functions for managing sqlite3 operations and migrations
 */
class SKYPLEXC2_API SQLiteUtility
{
public:

	SQLiteUtility();
	~SQLiteUtility();

	// Generic error handler for a sqlite3 return code
	static int ErrHandle(USPLogger* log, int rc, sqlite3* connection, const char* opMsg);

	// Prepare statement method that finalizes stmt on error
	static int PrepareSQLStatement(USPLogger* log, sqlite3* connection, const char* query, sqlite3_stmt** stmt, int nByte = -1, const char** pzTail = nullptr);

	// Execute a "Simple" SQL statement without preparation
	static int ExecuteSQL(USPLogger* log, sqlite3* connection, const std::string& sqlQuery);

	// Connect to a database at the given path. If db doesn't exist, create a new one
	static sqlite3* OpenDatabase(USPLogger* log, const FString& dbPath);

	// Get the current version of the given database connection
	//The connection's migration schema MUST implement `VERSION_TABLE_SCHEMA`
	static int GetDatabaseVersion(USPLogger* log, sqlite3* connection);

	// Migrate a database to the most current version
	static int MigrateDatabase(USPLogger* log, sqlite3* connection, const MigrationMap migrations);

	// Check if a database has no rows
	static bool IsEmpty(USPLogger* log, sqlite3* connection, const std::string& tbl);

	// Helpers
	static const char* ColText(sqlite3_stmt* stmt, int col);

	static FString ColFString(sqlite3_stmt* stmt, int col);

private:
	static const FString LOG_ORIGIN;
};
