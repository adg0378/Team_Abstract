// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPObstacleUtility.generated.h"

USTRUCT(BlueprintType)
struct FFAAObjectDataStruct
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString oas;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool verified;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString country;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString state;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString city;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float latitude;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float longitude;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString type;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int quantity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float agl;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float amsl;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString lighting;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString horizontalAccuracy;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString verticalAccuracy;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString marking;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString faaStudy;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString lastAction;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int lastActionDate;
};

USTRUCT(BlueprintType)
struct FFAAObjectAPIResult
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FFAAObjectDataStruct> obstacles;
};


/// ADSB Struct
//

USTRUCT(BlueprintType)
struct FADSBLastPositionStruct
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double lat;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double lon;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int nic;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int rc;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double seen_pos;


};

USTRUCT(BlueprintType)
struct FADSBAircraftStruct
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int alert;

    //altitude - if airplane is grounded, it will be "ground" instead of an altitude
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString alt_baro;

    // geometric altitude (height above ellipsoid in feet. Maybe consider using this when it is provided)
    // if alt_baro is "ground", this is probably empty
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int alt_geom;

    // change in barometric altitude in ft / min
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int baro_rate;

    // emitter category to identify particular aircraft or vehicle classes
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString category;

    // emergency / priority status
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString emergency;

    // flight number or callsign of aircraft
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString flight;

    // ground speed in knots
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double gs;

    // geometric vertical accuracy
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int gva;

    // unique International Civil Aviation Organization ID
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString hex;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double lat;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double lon;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int messages;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FString> mlat;

    // navigation accuracy for position
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int nac_p;

    // navigation accuracy for velocity
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int nac_v;

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int nav_altitude_mcp;*/

    // heading used for nav
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double nav_heading;

    //// Altimeter setting
    //UPROPERTY(BlueprintReadWrite, EditAnywhere)
    //double nav_qnh;

    // NIC indicates geometric position accuracy and reliability
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int nic;

    // Indicates integrity of aircraft's barometric altitude
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int nic_baro;

    // aircraft registration
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString r;

    // radius of containment - smaller RC values indicate higher level of confidence in position
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int rc;

    // radio signal strength indicator
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double rssi;

    // signal detectiono algorithm
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int sda;

    // how long ago a message was last recieved from this aircraft
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double seen;

    // how long ago was the position updated
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double seen_pos;

    // probability that aircraft position is accurate
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int sil;

    // interpretation of SIL
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString sil_type;

    // special position identification bit
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int spi;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString squawk;

    // aircraaft ICAO type
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString t;

    // data broadcast by local aircraft control to fill in ADSB gaps
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FString> tisb;

    // true track over ground in degrees
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double track;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString type;

    // adsb version number
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int version;

    // change in geometric altitude in feet/min
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int geom_rate;

    // bitfield for certain database flags
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int dbFlags;

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FString> nav_modes;*/

    // direction aircraft is heading (nose pointing) measured clockwise from true north.
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double true_heading;

    // Indicated airspeed in knots
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int ias;

    // mach number
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double mach;

    // Heading, degrees clockwise from magnetic north. This one is usually used
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double mag_heading;

    // outer/static air temp (C)
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int oat;

    // Roll, degrees, negative is left roll
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double roll;

    // true airspeed in knots. This one is better for tracking
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int tas;

    // total air temperature (C)
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int tat;

    // rate of change of track in degrees/second
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double track_rate;

    // wind direction
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int wd;

    // wind speed
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int ws;

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double gpsOkBefore;*/

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double gpsOkLat;*/

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double gpsOkLon;*/

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FADSBLastPositionStruct lastPosition; */

    // rough lat if no ADSB or MLAT pos is available
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double rr_lat;

    // rough lon if no ADSB or MLAT pos is available
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    double rr_lon;

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int calc_track;*/

    /*UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int nav_altitude_fms; */
};

USTRUCT(BlueprintType)
struct FADSBObjectDataResult
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FADSBAircraftStruct> ac;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int ctime;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString msg;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int now;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int ptime;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int total;


};

/**
 * 
 */
UCLASS()
class SKYPLEXC2_API USPObstacleUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    static TFuture<TArray<FFAAObjectDataStruct>> GetFAADOFObjects(class USPLogger* Log, FString LogOrigin, float Lon, float Lat, int32 RadiusNM);
    static TFuture<FADSBObjectDataResult> GetADSBObjects(USPLogger* Log, FString LogOrigin, float Lon, float Lat, int32 RadiusNM);
};
