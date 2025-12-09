// Fill out your copyright notice in the Description page of Project Settings.


#include "State/Storage/MissionAssetsManager.h"
#include "State/Storage/SQLiteUtility.h"
#include "Objects/Interests/POI.h"
#include "Objects/Interests/AreaOfInterest.h"
#include "State/Missions/InterestsUtil.h"
#include "State/Storage/GeoJsonManager.h"
#include "State/Storage/SPStorageManager.h"
#include "../ThirdParty/SQLite/Include/sqlite3.h"

const MigrationMap MIGRATIONS = {
	{1, {
		VERSION_TABLE_SCHEMA,

		/*"CREATE TABLE IF NOT EXISTS tbl_missions (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	name TEXT,\n"
		"	description TEXT,\n"
		"	start_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,\n"
		"	end_time TIMESTAMP,\n"
		"	last_state TEXT\n"
		");",*/

		"CREATE TABLE IF NOT EXISTS tbl_missions (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	name TEXT UNIQUE,\n"
		"   interests_order TEXT\n"
		");",

		"CREATE TABLE IF NOT EXISTS tbl_interests (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"   name TEXT,\n"
		"	type INT8,\n"
		"   point_locations TEXT,\n"
		"	group_id_fk INTEGER,\n"
		"	params TEXT,\n"	
		"	FOREIGN KEY (group_id_fk) REFERENCES tbl_interest_groups(id_pk)\n"
		");",

		"CREATE TABLE IF NOT EXISTS tbl_swarms (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	name TEXT UNIQUE\n"
		");",

		"CREATE TABLE IF NOT EXISTS tbl_drones (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	name TEXT,\n"
		"	swarm_id_fk INTEGER,\n"
		"	FOREIGN KEY (swarm_id_fk) REFERENCES tbl_swarms(id_pk)\n"
		");",

		"CREATE TABLE IF NOT EXISTS tbl_mission_interests (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	mission_id_fk INTEGER,\n"
		"	interest_group_id_fk INTEGER,\n"
		"	FOREIGN KEY (mission_id_fk) REFERENCES tbl_missions(id_pk),\n"
		"	FOREIGN KEY (interest_group_id_fk) REFERENCES tbl_interest_groups(id_pk)\n"
		");",

		"CREATE TABLE IF NOT EXISTS tbl_mission_swarms (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	mission_id_fk INTEGER,\n"
		"	swarm_id_fk INTEGER,\n"
		"	FOREIGN KEY (mission_id_fk) REFERENCES tbl_missions(id_pk),\n"
		"	FOREIGN KEY (swarm_id_fk) REFERENCES tbl_swarms(id_pk)\n"
		");",

		"CREATE TABLE IF NOT EXISTS tbl_telemetry_log (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	timestamp	TIMESTAMP,\n"
		"	data TEXT,\n"
		"	drone_id_fk INTEGER,\n"
		"	mission_id_fk INTEGER,\n"
		"	FOREIGN KEY (drone_id_fk) REFERENCES tbl_drones(id_pk),\n"
		"	FOREIGN KEY (mission_id_fk) REFERENCES tbl_missions(id_pk)\n"
		");",

		"CREATE TABLE IF NOT EXISTS tbl_mission_log (\n"
		"	id_pk INTEGER PRIMARY KEY,\n"
		"	timestamp	TIMESTAMP,\n"
		"	type INTEGER,\n"
		"	data TEXT,\n"
		"	drone_id_fk INTEGER,\n"
		"	mission_id_fk INTEGER,\n"
		"	FOREIGN KEY (drone_id_fk) REFERENCES tbl_drones(id_pk),\n"
		"	FOREIGN KEY (mission_id_fk) REFERENCES tbl_missions(id_pk)\n"
		");",
	}},
};

UMissionAssetsManager::UMissionAssetsManager()
	: dbConnection(nullptr)
{
	FCoreDelegates::OnPreExit.AddUObject(this, &UMissionAssetsManager::OnPreExitCleanup);
}

UMissionAssetsManager::~UMissionAssetsManager() {
	if (dbConnection) {
		sqlite3_close(dbConnection);
		dbConnection = nullptr;
	}
}

void UMissionAssetsManager::Setup_Implementation(bool& outSuccess) {
	Super::Setup_Implementation(outSuccess);
	InitializeDatabase(outSuccess);
	
	GeoJsonManagerRef = StorageManagerRef->geoJsonManagerRef;
}

void UMissionAssetsManager::InsertAOI(FString name, TArray<FVector> pointLocations, int32 groupID, FString& ParamsJson, int32& outID) {
	TArray<FString> CSVLines;

	for (const FVector& v : pointLocations)
	{
		CSVLines.Add(FString::Printf(TEXT("%f,%f,%f"), v.X, v.Y, v.Z));
	}

	FString pointsCSV = FString::Join(CSVLines, TEXT("\n"));
	InsertInterest(name, DBInterestType::AOI, pointsCSV, groupID, ParamsJson, outID);

	GeoJsonManagerRef->AddGeoJsonPolygon(name, TEXT("AOI"), groupID, outID, pointLocations);
}

void UMissionAssetsManager::InsertGeoFence(FString Name, TArray<FVector> PointLocations, int32 GroupID, FString& ParamsJson, int32& OutID) {
	TArray<FString> CSVLines;

	for (const FVector& V : PointLocations)
	{
		CSVLines.Add(FString::Printf(TEXT("%f,%f,%f"), V.X, V.Y, V.Z));
	}

	FString PointsCSV = FString::Join(CSVLines, TEXT("\n"));
	InsertInterest(Name, DBInterestType::GeoFence, PointsCSV, GroupID, ParamsJson, OutID);
	
	GeoJsonManagerRef->AddGeoJsonPolygon(Name, TEXT("GeoFence"), GroupID, OutID, PointLocations);
}

void UMissionAssetsManager::InsertPOI(FString name, FVector pointLocation, int32 groupID, FString& ParamsJson, int32& outID) {
	FString pointCSV = FString::Printf(TEXT("%f,%f,%f"), pointLocation.X, pointLocation.Y, pointLocation.Z);
	InsertInterest(name, DBInterestType::POI, pointCSV, groupID, ParamsJson, outID);

	GeoJsonManagerRef->AddGeoJsonPoint(name, TEXT("POI"), groupID, outID, pointLocation);
}

void UMissionAssetsManager::InsertTakeoffPoint(FString Name, FVector PointLocation, int32 MissionID, FString& ParamsJson, int32& OutID) {
	FString PointCSV = FString::Printf(TEXT("%f,%f,%f"), PointLocation.X, PointLocation.Y, PointLocation.Z);
	InsertInterest(Name, DBInterestType::TakeoffPoint, PointCSV, MissionID, ParamsJson, OutID);

	GeoJsonManagerRef->AddGeoJsonPoint(Name, TEXT("TakeOffPoint"), MissionID, OutID, PointLocation);
}

void UMissionAssetsManager::InsertInterest(FString name, DBInterestType type, FString pointCSV, int32 groupID, FString& ParamsJson, int32& outID) {
	const char* query = "INSERT INTO tbl_interests(name, type, point_locations, group_id_fk, params) VALUES( ?, ?, ?, ?, ? );";
	sqlite3_stmt* stmt;

	outID = -1;
	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*name), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, static_cast<uint8>(type));
	sqlite3_bind_text(stmt, 3, TCHAR_TO_UTF8(*pointCSV), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 4, groupID);
	sqlite3_bind_text(stmt, 5, TCHAR_TO_UTF8(*ParamsJson), -1, SQLITE_TRANSIENT);

	int rc = sqlite3_step(stmt);
	bool SetID = true;
	if (rc != SQLITE_DONE) {
		SetID = false;
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString("Failed to insert interest"), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);

	if (SetID) {
		outID = static_cast<int32>(sqlite3_last_insert_rowid(dbConnection));
	}
}

void UMissionAssetsManager::RenameInterest(int32 InID, FString InNewName) {
	const char* query = "UPDATE tbl_interests SET name = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*InNewName), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to rename interest with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	} else {
		GeoJsonManagerRef->RenameGeoJson(InNewName, InID);
	}
	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::UpdateInterestParams(int32 InID, FString& ParamsJson) {
	const char* query = "UPDATE tbl_interests SET params = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*ParamsJson), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to update interest params with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}

	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::ReassignInterest(int32 InID, int32 InNewGroupID) {
	const char* query = "UPDATE tbl_interests SET group_id_fk = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, InNewGroupID);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to reassign interest with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::MovePOI(int32 InID, FVector InNewPointLocation) {
	FString PointCSV = FString::Printf(TEXT("%f,%f,%f"), InNewPointLocation.X, InNewPointLocation.Y, InNewPointLocation.Z);
	MoveInterest(InID, PointCSV);
	GeoJsonManagerRef->UpdateCoordsGeoJsonPoint(InNewPointLocation, InID);
}

void UMissionAssetsManager::MoveTakeoffPoint(int32 InID, FVector InNewPointLocation) {
	FString PointCSV = FString::Printf(TEXT("%f,%f,%f"), InNewPointLocation.X, InNewPointLocation.Y, InNewPointLocation.Z);
	MoveInterest(InID, PointCSV);
	GeoJsonManagerRef->UpdateCoordsGeoJsonPoint(InNewPointLocation, InID);
}

void UMissionAssetsManager::MoveAOI(int32 InID, TArray<FVector> InNewPointLocations) {
	TArray<FString> CSVLines;

	for (const FVector& V : InNewPointLocations)
	{
		CSVLines.Add(FString::Printf(TEXT("%f,%f,%f"), V.X, V.Y, V.Z));
	}

	FString PointsCSV = FString::Join(CSVLines, TEXT("\n"));
	MoveInterest(InID, PointsCSV);
	GeoJsonManagerRef->UpdateCoordsGeoJsonPolygon(InNewPointLocations, InID);
}

void UMissionAssetsManager::MoveGeoFence(int32 InID, TArray<FVector> InNewPointLocations) {
	MoveAOI(InID, InNewPointLocations);
	GeoJsonManagerRef->UpdateCoordsGeoJsonPolygon(InNewPointLocations, InID);
}

void UMissionAssetsManager::MoveInterest(int32 InID, FString PointCSV) {
	const char* query = "UPDATE tbl_interests SET point_locations = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*PointCSV), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to move interest with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::LoadInterests(
	TArray<FPOIDataStruct>& outPOIs,
	TArray<FAOIDataStruct>& outAOIs,
	TArray<FGeoFenceDataStruct>& OutGeoFences,
	TArray<FTakeoffPointDataStruct>& OutTakeoffPoints
) {
	outPOIs.Empty();
	outAOIs.Empty();
	OutGeoFences.Empty();
	OutTakeoffPoints.Empty();
	GeoJsonManagerRef->ClearGeoJsons();

	const char* query = "SELECT id_pk, name, type, point_locations, group_id_fk, params FROM tbl_interests;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int32 id = sqlite3_column_int(stmt, 0);
		FString name = FString(UTF8_TO_TCHAR((const char*)sqlite3_column_text(stmt, 1)));
		DBInterestType type = static_cast<DBInterestType>(sqlite3_column_int(stmt, 2));
		FString pointLocations = FString(UTF8_TO_TCHAR((const char*)sqlite3_column_text(stmt, 3)));
		int32 groupID = sqlite3_column_int(stmt, 4);
		FString Params = FString(UTF8_TO_TCHAR((const char*)sqlite3_column_text(stmt, 5)));

		if (type == DBInterestType::POI || type == DBInterestType::TakeoffPoint) {
			TArray<FString> values;
			pointLocations.ParseIntoArray(values, TEXT(","));

			if (values.Num() >= 3) {
				FVector location(FCString::Atof(*values[0]), FCString::Atof(*values[1]), FCString::Atof(*values[2]));

				if (type == DBInterestType::POI) {
					FPOIDataStruct poi(id, name, groupID, location, Params);
					outPOIs.Add(poi);
					GeoJsonManagerRef->AddGeoJsonPoint(name, TEXT("POI"), groupID, id, location);
				}
				else {
					FTakeoffPointDataStruct TakeoffPoint(id, name, groupID, location, Params);
					OutTakeoffPoints.Add(TakeoffPoint);
					GeoJsonManagerRef->AddGeoJsonPoint(name, TEXT("TakeOffPoint"), groupID, id, location);
				}
				
			}
		}
		else if (type == DBInterestType::AOI || type == DBInterestType::GeoFence) {
			TArray<FString> locationLines;
			pointLocations.ParseIntoArrayLines(locationLines);

			TArray<FVector> locations;

			for (const FString& line : locationLines) {
				TArray<FString> values;
				line.ParseIntoArray(values, TEXT(","));

				if (values.Num() >= 3) {
					FVector location(FCString::Atof(*values[0]), FCString::Atof(*values[1]), FCString::Atof(*values[2]));
					locations.Add(location);
				}
			}

			if (type == DBInterestType::AOI) {
				FAOIDataStruct aoi(id, name, groupID, locations, Params);
				outAOIs.Add(aoi);
				GeoJsonManagerRef->AddGeoJsonPolygon(name, TEXT("AOI"), groupID, id, locations);
			}
			else {
				FGeoFenceDataStruct GeoFence(id, name, groupID, locations, Params);
				OutGeoFences.Add(GeoFence);
				GeoJsonManagerRef->AddGeoJsonPolygon(name, TEXT("GeoFence"), groupID, id, locations);
			}
		}
	}

	LOG->Info(TEXT("test test 1 2"), LOG_ORIGIN);
	if (rc != SQLITE_DONE)
	{
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, "Failed to load all interests", rc, errMsg, LOG_ORIGIN);
	}

	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::InsertMission(FString name, int32& outID) {
	const char* query = "INSERT INTO tbl_missions(name) VALUES( ? );";
	sqlite3_stmt* stmt;

	outID = -1;
	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*name), -1, SQLITE_STATIC);

	int rc = sqlite3_step(stmt);
	bool SetID = true;
	if (rc != SQLITE_DONE) {
		SetID = false;
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString("Failed to insert mission"), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);

	if (SetID) {
		outID = static_cast<int32>(sqlite3_last_insert_rowid(dbConnection));
	}
}

void UMissionAssetsManager::RenameMission(int32 InID, FString InNewName) {
	const char* query = "UPDATE tbl_missions SET name = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*InNewName), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to rename mission with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::UpdateMissionOrder(int32 InID, TArray<int32>& InterestOrder) {
	TArray<FString> StringOrder;
	for (const int32& ID : InterestOrder) {
		StringOrder.Add(FString::FromInt(ID));
	}

	FString OrderCSV = FString::Join(StringOrder, TEXT(","));

	const char* query = "UPDATE tbl_missions SET interests_order = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*OrderCSV), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to update mission order with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::DeleteMission(int32 InID) {
	const char* query = "DELETE FROM tbl_missions WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to delete mission with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}

	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::LoadMissions(TMap<int32, FSPMissionStruct>& outMissions) {
	outMissions.Empty();

	const char* query = "SELECT id_pk, name, interests_order FROM tbl_missions;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int32 id = sqlite3_column_int(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		const char* order = (const char*)sqlite3_column_text(stmt, 2);

		if (name)
		{
			FSPMissionStruct Mission;
			Mission.ID = id;
			Mission.Name = FString(UTF8_TO_TCHAR(name));

			TArray<FString> OrderArray;
			FString(UTF8_TO_TCHAR(order)).ParseIntoArray(OrderArray, TEXT(","));

			for (const FString& InterestID : OrderArray) {
				int32 AsInt = FCString::Atoi(*InterestID);
				Mission.InterestOrder.Add(AsInt);
			}
			
			outMissions.Add(id, Mission);
		}
	}

	if (rc != SQLITE_DONE)
	{
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, "Failed to load all missions", rc, errMsg, LOG_ORIGIN);
	}

	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::LoadSwarms(TMap<int32, FString>& OutSwarms) {
	OutSwarms.Empty();

	const char* query = "SELECT id_pk, name FROM tbl_swarms;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
	{
		int32 id = sqlite3_column_int(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		if (name)
		{
			OutSwarms.Add(id, FString(UTF8_TO_TCHAR(name)));
		}
	}

	if (rc != SQLITE_DONE)
	{
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, "Failed to load all swarms", rc, errMsg, LOG_ORIGIN);
	}

	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::InsertSwarm(FString Name, int32& OutID) {
	const char* query = "INSERT INTO tbl_swarms(name) VALUES( ? );";
	sqlite3_stmt* stmt;

	OutID = -1;
	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*Name), -1, SQLITE_STATIC);

	int rc = sqlite3_step(stmt);
	bool SetID = true;
	if (rc != SQLITE_DONE) {
		SetID = false;
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString("Failed to insert swarm"), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);

	if (SetID) {
		OutID = static_cast<int32>(sqlite3_last_insert_rowid(dbConnection));
	}
}

void UMissionAssetsManager::RenameSwarm(int32 InID, FString InNewName) {
	const char* query = "UPDATE tbl_swarms SET name = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*InNewName), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to rename swarm with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}
	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::DeleteSwarm(int32 InID) {
	const char* query = "DELETE FROM tbl_swarms WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to delete swarm with id: '{0}'"), { InID }), rc, errMsg, LOG_ORIGIN);
	}

	sqlite3_finalize(stmt);
}

void UMissionAssetsManager::OnPreExitCleanup() {
	if (dbConnection) {
		sqlite3_close(dbConnection);
		dbConnection = nullptr;
	}
}

void UMissionAssetsManager::InitializeDatabase(bool& outSuccess)
{
	LOG->Info(FString::Printf(TEXT("SQLite Version: %s"), UTF8_TO_TCHAR(sqlite3_libversion())));

	if (dbConnection) {
		sqlite3_close(dbConnection);
		dbConnection = nullptr;
	}

	const FString dbPath = USPEnvConstants::GetDatabasePath();

	LOG->Info(FString::Format(*FString("Searching for database at path: '{0}'"), { *dbPath }), LOG_ORIGIN);

	// Get database connection
	if (!FPaths::FileExists(dbPath))
	{
		LOG->Info(FString::Format(*FString("Could not locate database at '{0}': creating new database"), { *dbPath }), LOG_ORIGIN);
	}
	else {
		LOG->Info(FString("Found database: connecting..."), LOG_ORIGIN);
	}
	dbConnection = SQLiteUtility::OpenDatabase(LOG, dbPath);

	if (!dbConnection) {
		LOG->Info(FString("No connection"), LOG_ORIGIN);
		outSuccess = false;
		return;
	}
	LOG->Info(FString("Connection successful"), LOG_ORIGIN);

	if (SQLiteUtility::MigrateDatabase(LOG, dbConnection, MIGRATIONS) != 0) {
		outSuccess = false;
		return;
	}
	outSuccess = true;
}

void UMissionAssetsManager::DeleteInterest(int32 id) {
	const char* query = "DELETE FROM tbl_interests WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (SQLiteUtility::PrepareSQLStatement(LOG, dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, id);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->LogStorageManagerMessage(ELogLevel::Error, FString::Format(*FString("Failed to delete interest with id: '{0}'"), { id }), rc, errMsg, LOG_ORIGIN);
	} else {
		GeoJsonManagerRef->DeleteGeoJson(id);
	}

	sqlite3_finalize(stmt);
}

//void UMissionAssetsManager::LoadInterestsViaFile(
//	FString filePath,
//	TArray<FPOIDataStruct>& outPOIs,
//	TArray<FAOIDataStruct>& outAOIs,
//	TArray<FGeoFenceDataStruct>& OutGeoFences,
//	TArray<FTakeoffPointDataStruct>& OutTakeoffPoints
//) {
//	
//	if (!GeoJsonManagerRef->LoadGeoJsonsViaFile(filePath)) {
//
//		LOG->Error(TEXT("Failed to load interests from GeoJson file"), LOG_ORIGIN);
//		return;
//	}
//	TArray<FGeoJsonData> geoJsonsData;
//	GeoJsonManagerRef->GetGeoJsons(geoJsonsData);
//
//	outPOIs.Empty();
//	outAOIs.Empty();
//	OutGeoFences.Empty();
//	OutTakeoffPoints.Empty();
//
//	for (FGeoJsonData& data : geoJsonsData) {
//		if (data.features.geometry.properties.interestType == TEXT("POI") || 
//			data.features.geometry.properties.interestType == TEXT("TakeOffPoint")) {
//			
//			if (data.features.geometry.properties.interestType == TEXT("POI")) {
//				FPOIDataStruct poi(data.features.geometry.properties.interestID,
//					data.features.geometry.properties.name, 
//					data.features.geometry.properties.groupID,
//					data.features.geometry.coordinates[0]);
//				outPOIs.Add(poi);
//			}
//			else {
//				FTakeoffPointDataStruct TakeoffPoint(data.features.geometry.properties.interestID,
//					data.features.geometry.properties.name,
//					data.features.geometry.properties.groupID,
//					data.features.geometry.coordinates[0]);
//				OutTakeoffPoints.Add(TakeoffPoint);
//			}
//
//		} else if (data.features.geometry.properties.interestType == TEXT("AOI") ||
//			data.features.geometry.properties.interestType == TEXT("GeoFence")) {
//
//			if (data.features.geometry.properties.interestType == TEXT("AOI")) {
//				FAOIDataStruct aoi(data.features.geometry.properties.interestID,
//					data.features.geometry.properties.name,
//					data.features.geometry.properties.groupID,
//					data.features.geometry.coordinates);
//				outAOIs.Add(aoi);
//			}
//			else {
//				FGeoFenceDataStruct GeoFence{.ID = data.features.geometry.properties.interestID,
//					.GroupID = data.features.geometry.properties.groupID,
//					.Name = data.features.geometry.properties.name,
//					.LonLatHeights = data.features.geometry.coordinates };
//				OutGeoFences.Add(GeoFence);
//			}
//		}
//	}
//	
//	LOG->Error(TEXT("Successfully loaded interests from GeoJson file"), LOG_ORIGIN);
//}
