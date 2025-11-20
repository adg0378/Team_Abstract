// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPSQLiteUtility.h"
#include <sqlite3.h>
#include <string>

USPSQLiteUtility::USPSQLiteUtility() {}

USPSQLiteUtility::~USPSQLiteUtility() {}

int USPSQLiteUtility::ErrHandle(int rc, sqlite3* connection, const char* opMsg) {
	if (rc != SQLITE_OK) {
		const char* ErrMsg = sqlite3_errmsg(connection);
		const FString ErrMsgString = FString(UTF8_TO_TCHAR(ErrMsg));
		UE_LOG(LogTemp, Error, TEXT("%s %i %s"), *FString(opMsg), rc, *ErrMsgString);
	}
	return rc;
}

int USPSQLiteUtility::PrepareSQLStatement(sqlite3* connection, const char* query, sqlite3_stmt** stmt, int nByte, const char** pzTail) {
	int rc = ErrHandle(sqlite3_prepare_v2(connection, query, nByte, stmt, pzTail), connection, "Error preparing statement");
	if (rc != SQLITE_OK) {
		sqlite3_finalize(*stmt);
	}
	return rc;
}

int USPSQLiteUtility::ExecuteSQL(sqlite3* connection, const std::string& sqlQuery) {
	char* errMsg = nullptr;
	int rc = sqlite3_exec(connection, sqlQuery.c_str(), nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		UE_LOG(LogTemp, Error, TEXT("%s %i %hs"), *FString("Error executing SQL query"), rc, errMsg);
		sqlite3_free(errMsg);
	}
	return rc;
}

sqlite3* USPSQLiteUtility::OpenDatabase(const FString& dbPath) {
	sqlite3* connection = nullptr;
	int rc = sqlite3_open(TCHAR_TO_UTF8(*dbPath), &connection);

	if (rc != SQLITE_OK)
	{
		const char* ErrMsg = sqlite3_errmsg(connection);
		const FString ErrMsgString = FString(UTF8_TO_TCHAR(ErrMsg));
		const FString fmtStr = FString("Could not open database at path: '{0}'");
		UE_LOG(LogTemp, Error, TEXT("%s %i %s"), *FString::Format(*fmtStr, { *dbPath }), rc, *ErrMsgString);
		return nullptr;
	}
	return connection;
}

int USPSQLiteUtility::GetDatabaseVersion(sqlite3* connection) {
	sqlite3_stmt* stmt;

	// TODO: implement a better check for this. We should probably make sure tbl_version exists first or pop this into a separate function
	const char* query = "SELECT version FROM tbl_version ORDER BY id_pk DESC LIMIT 1;";
	if (PrepareSQLStatement(connection, query, &stmt) != SQLITE_OK) {
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

int USPSQLiteUtility::MigrateDatabase(sqlite3* connection, const MigrationMap migrations) {
	int currentVersion = GetDatabaseVersion(connection);

	if (currentVersion == -1) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get current database version"));
		return 1;
	}

	int latestVersion = migrations.rbegin()->first;

	UE_LOG(LogTemp, Log, TEXT("Current database version: %i - Latest version: %i"), currentVersion, latestVersion);

	// if any failure occurs, migration will cease
	for (int version = currentVersion + 1; version <= latestVersion; ++version) {
		UE_LOG(LogTemp, Log, TEXT("Migrating database to version %i"), version);

		// execute migration queries
		bool failed = false;
		for (const auto& query : migrations.at(version)) {
			if (ExecuteSQL(connection, query) != SQLITE_OK) {
				failed = true;
			}
		}

		if (failed) {
			UE_LOG(LogTemp, Warning, TEXT("Failed to apply migration version %i: Aborting migration"), version);
			return 1;
		}

		// stamp database version table upon successful migration
		std::string updateVersionQuery = "INSERT INTO tbl_version (version) VALUES (" + std::to_string(version) + ");";
		if (ExecuteSQL(connection, updateVersionQuery) != SQLITE_OK) {
			UE_LOG(LogTemp, Error, TEXT("Failed to stamp migration to version %i"), version)
			return 1;
		}
	}
	return 0;
}

bool USPSQLiteUtility::IsEmpty(sqlite3* connection, const std::string& tbl) {
	std::string query = "SELECT NOT EXISTS(SELECT 1 FROM " + tbl + " LIMIT 1);";
	sqlite3_stmt* stmt;

	if (PrepareSQLStatement(connection, query.c_str(), &stmt) != SQLITE_OK) {
		return true;
	}

	int rc = sqlite3_step(stmt);
	bool isEmpty = (rc == SQLITE_ROW) && (sqlite3_column_int(stmt, 0) == 1);
	sqlite3_finalize(stmt);
	return isEmpty;
}

const char* USPSQLiteUtility::ColText(sqlite3_stmt* stmt, int col) {
	return reinterpret_cast<const char*>(sqlite3_column_text(stmt, col));
}

FString USPSQLiteUtility::ColFString(sqlite3_stmt* stmt, int col) {
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(sqlite3_column_text(stmt, col)))).TrimStartAndEnd();
}

