#include "State/Storage/SQLiteUtility.h"
#include "SPLogger.h"
#include "../ThirdParty/SQLite/Include/sqlite3.h"
#include <string>

SQLiteUtility::SQLiteUtility()
{
}

SQLiteUtility::~SQLiteUtility()
{
}

const FString SQLiteUtility::LOG_ORIGIN = TEXT("SQLite");

int SQLiteUtility::ErrHandle(USPLogger* log, int rc, sqlite3* connection, const char* opMsg) {
	if (rc != SQLITE_OK) {
		const char* ErrMsg = sqlite3_errmsg(connection);
		const FString ErrMsgString = FString(UTF8_TO_TCHAR(ErrMsg));
		log->LogStorageManagerMessage(ELogLevel::Error, FString(opMsg), rc, *ErrMsgString, LOG_ORIGIN);
	}
	return rc;
}

int SQLiteUtility::PrepareSQLStatement(USPLogger* log, sqlite3* connection, const char* query, sqlite3_stmt** stmt, int nByte, const char** pzTail) {
	int rc = ErrHandle(log, sqlite3_prepare_v2(connection, query, nByte, stmt, pzTail), connection, "Error preparing statement");
	if (rc != SQLITE_OK) {
		sqlite3_finalize(*stmt);
	}
	return rc;
}

int SQLiteUtility::ExecuteSQL(USPLogger* log, sqlite3* connection, const std::string& sqlQuery) {
	char* errMsg = nullptr;
	int rc = sqlite3_exec(connection, sqlQuery.c_str(), nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		log->LogStorageManagerMessage(ELogLevel::Error, FString("Error executing SQL query"), rc, errMsg, LOG_ORIGIN);
		sqlite3_free(errMsg);
	}
	return rc;
}

sqlite3* SQLiteUtility::OpenDatabase(USPLogger* log, const FString& dbPath) {
	sqlite3* connection = nullptr;
	int rc = sqlite3_open(TCHAR_TO_UTF8(*dbPath), &connection);

	if (rc != SQLITE_OK)
	{
		const char* ErrMsg = sqlite3_errmsg(connection);
		const FString ErrMsgString = FString(UTF8_TO_TCHAR(ErrMsg));
		const FString fmtStr = FString("Could not open database at path: '{0}'");
		log->LogStorageManagerMessage(ELogLevel::Critical, FString::Format(*fmtStr, { *dbPath }), rc, *ErrMsgString, LOG_ORIGIN);
		return nullptr;
	}
	return connection;
}

int SQLiteUtility::GetDatabaseVersion(USPLogger* log, sqlite3* connection) {
	sqlite3_stmt* stmt;

	// TODO: implement a better check for this. We should probably make sure tbl_version exists first or pop this into a separate function
	const char* query = "SELECT version FROM tbl_version ORDER BY id_pk DESC LIMIT 1;";
	if (PrepareSQLStatement(log, connection, query, &stmt) != SQLITE_OK) {
		/*return -1;*/
		return 0; // return 0 for now. Should probably make a check to check if the db exists and if the version table exists
	}

	int rc = sqlite3_step(stmt);
	int version = 0;
	if (rc == SQLITE_ROW) {
		version = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return version;
}

int SQLiteUtility::MigrateDatabase(USPLogger* log, sqlite3* connection, const MigrationMap migrations) {
	int currentVersion = GetDatabaseVersion(log, connection);

	if (currentVersion == -1) {
		log->Error(FString("Failed to get current database version"), LOG_ORIGIN);
		return 1;
	}

	int latestVersion = migrations.rbegin()->first;

	log->Info(FString::Format(*FString("Current database version: {0} - Latest version: {1}"), { currentVersion, latestVersion }), LOG_ORIGIN);

	// if any failure occurs, migration will cease
	for (int version = currentVersion + 1; version <= latestVersion; ++version) {
		log->Info(FString::Format(*FString("Migrating database to version {0}"), { version }), LOG_ORIGIN);

		// execute migration queries
		bool failed = false;
		for (const auto& query : migrations.at(version)) {
			if (ExecuteSQL(log, connection, query) != SQLITE_OK) {
				failed = true;
			}
		}

		if (failed) {
			log->Warn(FString::Format(*FString("Failed to apply migration version {0}: Aborting migration."), { version }), LOG_ORIGIN);
			return 1;
		}

		// stamp database version table upon successful migration
		std::string updateVersionQuery = "INSERT INTO tbl_version (version) VALUES (" + std::to_string(version) + ");";
		if (ExecuteSQL(log, connection, updateVersionQuery) != SQLITE_OK) {
			log->Error(FString::Format(*FString("Failed to stamp migration to version {0}"), { version }), LOG_ORIGIN);
			return 1;
		}
	}
	return 0;
}

bool SQLiteUtility::IsEmpty(USPLogger* log, sqlite3* connection, const std::string& tbl) {
	std::string query = "SELECT NOT EXISTS(SELECT 1 FROM " + tbl + " LIMIT 1);";
	sqlite3_stmt* stmt;

	if (PrepareSQLStatement(log, connection, query.c_str(), &stmt) != SQLITE_OK) {
		return true;
	}

	int rc = sqlite3_step(stmt);
	bool isEmpty = (rc == SQLITE_ROW) && (sqlite3_column_int(stmt, 0) == 1);
	sqlite3_finalize(stmt);
	return isEmpty;
}

const char* SQLiteUtility::ColText(sqlite3_stmt* stmt, int col) {
	return reinterpret_cast<const char*>(sqlite3_column_text(stmt, col));
}

FString SQLiteUtility::ColFString(sqlite3_stmt* stmt, int col) {
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(sqlite3_column_text(stmt, col)))).TrimStartAndEnd();
}

