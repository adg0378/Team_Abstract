// Copyright (c) 2025 Synetos Aerospace


#include "Core/State/SPDatabaseManager.h"
#include "Util/SPSQLiteUtility.h"
#include "SPEnvConstants.h"
#include <sqlite3.h>

const MigrationMap MIGRATIONS = {
	{1, {
		VERSION_TABLE_SCHEMA,

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

USPDatabaseManager::USPDatabaseManager()
	: dbConnection(nullptr)
{
	FCoreDelegates::OnPreExit.AddUObject(this, &USPDatabaseManager::OnPreExitCleanup);
}

USPDatabaseManager::~USPDatabaseManager() {
	if (dbConnection) {
		sqlite3_close(dbConnection);
		dbConnection = nullptr;
	}
}

void USPDatabaseManager::Setup_Implementation() {
	Super::Setup_Implementation();
	InitializeDatabase();
}

void USPDatabaseManager::InsertAOI(FString name, TArray<FVector> pointLocations, int32 groupID, int32& outID) {
	TArray<FString> CSVLines;

	for (const FVector& v : pointLocations)
	{
		CSVLines.Add(FString::Printf(TEXT("%f,%f,%f"), v.X, v.Y, v.Z));
	}

	FString pointsCSV = FString::Join(CSVLines, TEXT("\n"));
	InsertInterest(name, DBInterestType::AOI, pointsCSV, groupID, outID);
}

void USPDatabaseManager::InsertGeoFence(FString Name, TArray<FVector> PointLocations, int32 GroupID, int32& OutID) {
	TArray<FString> CSVLines;

	for (const FVector& V : PointLocations)
	{
		CSVLines.Add(FString::Printf(TEXT("%f,%f,%f"), V.X, V.Y, V.Z));
	}

	FString PointsCSV = FString::Join(CSVLines, TEXT("\n"));
	InsertInterest(Name, DBInterestType::GeoFence, PointsCSV, GroupID, OutID);
}

void USPDatabaseManager::InsertPOI(FString name, FVector pointLocation, int32 groupID, int32& outID) {
	FString pointCSV = FString::Printf(TEXT("%f,%f,%f"), pointLocation.X, pointLocation.Y, pointLocation.Z);
	InsertInterest(name, DBInterestType::POI, pointCSV, groupID, outID);
}

void USPDatabaseManager::InsertTakeoffPoint(FString Name, FVector PointLocation, int32 MissionID, int32& OutID) {
	FString PointCSV = FString::Printf(TEXT("%f,%f,%f"), PointLocation.X, PointLocation.Y, PointLocation.Z);
	InsertInterest(Name, DBInterestType::TakeoffPoint, PointCSV, MissionID, OutID);
}

void USPDatabaseManager::InsertInterest(FString name, DBInterestType type, FString pointCSV, int32 groupID, int32& outID) {
	const char* query = "INSERT INTO tbl_interests(name, type, point_locations, group_id_fk) VALUES( ?, ?, ?, ? ) RETURNING id_pk;";
	sqlite3_stmt* stmt;

	outID = -1;
	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*name), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, static_cast<uint8>(type));
	sqlite3_bind_text(stmt, 3, TCHAR_TO_UTF8(*pointCSV), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 4, groupID);

	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		outID = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::RenameInterest(int32 InID, FString InNewName) {
	const char* query = "UPDATE tbl_interests SET name = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*InNewName), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to rename interest with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::ReassignInterest(int32 InID, int32 InNewGroupID) {
	const char* query = "UPDATE tbl_interests SET group_id_fk = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, InNewGroupID);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to reassign interest with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::MovePOI(int32 InID, FVector InNewPointLocation) {
	FString PointCSV = FString::Printf(TEXT("%f,%f,%f"), InNewPointLocation.X, InNewPointLocation.Y, InNewPointLocation.Z);
	MoveInterest(InID, PointCSV);
}

void USPDatabaseManager::MoveTakeoffPoint(int32 InID, FVector InNewPointLocation) {
	FString PointCSV = FString::Printf(TEXT("%f,%f,%f"), InNewPointLocation.X, InNewPointLocation.Y, InNewPointLocation.Z);
	MoveInterest(InID, PointCSV);
}

void USPDatabaseManager::MoveAOI(int32 InID, TArray<FVector> InNewPointLocations) {
	TArray<FString> CSVLines;

	for (const FVector& V : InNewPointLocations)
	{
		CSVLines.Add(FString::Printf(TEXT("%f,%f,%f"), V.X, V.Y, V.Z));
	}

	FString PointsCSV = FString::Join(CSVLines, TEXT("\n"));
	MoveInterest(InID, PointsCSV);
}

void USPDatabaseManager::MoveGeoFence(int32 InID, TArray<FVector> InNewPointLocations) {
	MoveAOI(InID, InNewPointLocations);
}

void USPDatabaseManager::MoveInterest(int32 InID, FString PointCSV) {
	const char* query = "UPDATE tbl_interests SET point_locations = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*PointCSV), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to move interest with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::LoadInterests(
	TArray<FPOIDataStruct>& outPOIs,
	TArray<FAOIDataStruct>& outAOIs,
	TArray<FGeoFenceDataStruct>& OutGeoFences,
	TArray<FTakeoffPointDataStruct>& OutTakeoffPoints
) {
	outPOIs.Empty();
	outAOIs.Empty();
	OutGeoFences.Empty();
	OutTakeoffPoints.Empty();

	const char* query = "SELECT id_pk, name, type, point_locations, group_id_fk FROM tbl_interests;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
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

		if (type == DBInterestType::POI || type == DBInterestType::TakeoffPoint) {
			TArray<FString> values;
			pointLocations.ParseIntoArray(values, TEXT(","));

			if (values.Num() >= 3) {
				FVector location(FCString::Atof(*values[0]), FCString::Atof(*values[1]), FCString::Atof(*values[2]));

				if (type == DBInterestType::POI) {
					FPOIDataStruct poi(id, name, groupID, location);
					outPOIs.Add(poi);
				}
				else {
					FTakeoffPointDataStruct TakeoffPoint(id, name, groupID, location);
					OutTakeoffPoints.Add(TakeoffPoint);
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
				FAOIDataStruct aoi(id, name, groupID, locations);
				outAOIs.Add(aoi);
			}
			else {
				FGeoFenceDataStruct GeoFence{ .ID = id,.GroupID = groupID, .Name = name, .LonLatHeights = locations };
				OutGeoFences.Add(GeoFence);
			}
		}
	}

	if (rc != SQLITE_DONE)
	{
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(TEXT("Failed to load interests"), GetClass()->GetName(), true);
	}

	sqlite3_finalize(stmt);
}

void USPDatabaseManager::InsertMission(FString name, int32& outID) {
	const char* query = "INSERT INTO tbl_missions(name) VALUES( ? ) RETURNING id_pk;";
	sqlite3_stmt* stmt;

	outID = -1;
	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*name), -1, SQLITE_STATIC);

	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		outID = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::RenameMission(int32 InID, FString InNewName) {
	const char* query = "UPDATE tbl_missions SET name = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*InNewName), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to rename mission with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::UpdateMissionOrder(int32 InID, TArray<int32> InterestOrder) {
	TArray<FString> StringOrder;
	for (const int32& ID : InterestOrder) {
		StringOrder.Add(FString::FromInt(ID));
	}

	FString OrderCSV = FString::Join(StringOrder, TEXT(","));

	const char* query = "UPDATE tbl_missions SET interests_order = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*OrderCSV), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to update mission order with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::DeleteMission(int32 InID) {
	const char* query = "DELETE FROM tbl_missions WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to delete mission with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}

	sqlite3_finalize(stmt);
}

void USPDatabaseManager::LoadMissions(TMap<int32, FSPMissionStruct>& outMissions) {
	outMissions.Empty();

	const char* query = "SELECT id_pk, name, interests_order FROM tbl_missions;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
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
		LOG->Error(TEXT("Failed to load all missions"), GetClass()->GetName(), true);
	}

	sqlite3_finalize(stmt);
}

void USPDatabaseManager::LoadSwarms(TMap<int32, FString>& OutSwarms) {
	OutSwarms.Empty();

	const char* query = "SELECT id_pk, name FROM tbl_swarms;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
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
		LOG->Error(TEXT("Failed to load all swarms"), GetClass()->GetName(), true);
	}

	sqlite3_finalize(stmt);
}

void USPDatabaseManager::InsertSwarm(FString Name, int32& OutID) {
	const char* query = "INSERT INTO tbl_swarms(name) VALUES( ? ) RETURNING id_pk;";
	sqlite3_stmt* stmt;

	OutID = -1;
	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*Name), -1, SQLITE_STATIC);

	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		OutID = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::RenameSwarm(int32 InID, FString InNewName) {
	const char* query = "UPDATE tbl_swarms SET name = ? WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, TCHAR_TO_UTF8(*InNewName), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to rename swarm with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}
	sqlite3_finalize(stmt);
}

void USPDatabaseManager::DeleteSwarm(int32 InID) {
	const char* query = "DELETE FROM tbl_swarms WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, InID);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to delete swarm with id: '{0}'"), { InID }), GetClass()->GetName(), true);
	}

	sqlite3_finalize(stmt);
}

void USPDatabaseManager::OnPreExitCleanup() {
	if (dbConnection) {
		sqlite3_close(dbConnection);
		dbConnection = nullptr;
	}
}

void USPDatabaseManager::InitializeDatabase()
{
	if (dbConnection) {
		sqlite3_close(dbConnection);
		dbConnection = nullptr;
	}

	const FString dbPath = USPEnvConstants::GetDatabasePath();
	FString Origin = GetClass()->GetName();

	LOG->Info(FString::Format(*FString("Searching for database at path: '{0}'"), { *dbPath }), Origin);

	// Get database connection
	if (!FPaths::FileExists(dbPath))
	{
		LOG->Info(FString::Format(*FString("Could not locate database at '{0}': creating new database"), { *dbPath }), Origin);
	}
	else {
		LOG->Info(FString("Found database: connecting..."), Origin);
	}
	dbConnection = USPSQLiteUtility::OpenDatabase(dbPath);

	if (!dbConnection) {
		LOG->Critical(FString("No connection"), Origin);
		return;
	}
	LOG->Info(FString("Connection successful"), Origin);

	if (USPSQLiteUtility::MigrateDatabase(dbConnection, MIGRATIONS) != 0) {
		return;
	}
}

void USPDatabaseManager::DeleteInterest(int32 id) {
	const char* query = "DELETE FROM tbl_interests WHERE id_pk = ?;";
	sqlite3_stmt* stmt;

	if (USPSQLiteUtility::PrepareSQLStatement(dbConnection, query, &stmt) != SQLITE_OK) {
		return;
	}

	sqlite3_bind_int(stmt, 1, id);

	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		const char* errMsg = sqlite3_errmsg(dbConnection);
		LOG->Error(FString::Format(*FString("Failed to delete interest with id: '{0}'"), { id }), GetClass()->GetName(), true);
	}

	sqlite3_finalize(stmt);
}

FPOIDataStruct::FPOIDataStruct()
	: id(-1),
	groupID(-1),
	name(TEXT("unnamed")),
	longLatHeight(FVector())
{
}

FPOIDataStruct::FPOIDataStruct(int32 inID, FString inName, int32 inGroupID, FVector inLongLatHeight)
	: id(inID),
	groupID(inGroupID),
	name(inName),
	longLatHeight(inLongLatHeight)
{
}

FTakeoffPointDataStruct::FTakeoffPointDataStruct() {}
FTakeoffPointDataStruct::FTakeoffPointDataStruct(int32 InID, FString InName, int32 InGroupID, FVector InLonLatHeight)
	: ID(InID),
	GroupID(InGroupID),
	Name(InName),
	LonLatHeight(InLonLatHeight)
{
}

FAOIDataStruct::FAOIDataStruct()
	: id(-1),
	groupID(-1),
	name(TEXT("unnamed")),
	longLatHeights(TArray<FVector>())
{
}

FAOIDataStruct::FAOIDataStruct(int32 inID, FString inName, int32 inGroupID, TArray<FVector> inLongLatHeights)
	: id(inID),
	groupID(inGroupID),
	name(inName),
	longLatHeights(inLongLatHeights)
{
}
