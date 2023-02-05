// SSImportGAIADR3.cpp
// SSCore
//
// Created by Tim DeBenedictis on 1/29/23.
// Copyright ©2023 Southern Stars Group, LLC. All rights reserved.
//
// Routines for importing GAIA DR3 star catalog data.
// Currently only tested on MacOS; will not compile on Windows due to
// dependencies on zlib.h and dirent.h (and possibly others).

#ifdef __APPLE__

#include <algorithm>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <zlib.h>

#include "SSImportGAIADR3.hpp"
#include "SSImportHIP.hpp"
#include "SSImportTYC.hpp"
#include "SSUtilities.hpp"

#define GAIADR3_HIP2_NUM_FIELDS 5
#define GAIADR3_TYC2_NUM_FIELDS 6
#define GAIADR3_SOURCE_NUM_FIELDS 152

struct SSGAIADir
{
    string  root;               // path to root directory containing gzipped GAIA source data files in CSV format
    DIR     *dp = nullptr;      // pointer to directory
    gzFile  fp = nullptr;       // pointer to gzipped data file
};

// Opens the GAIA source data file directory with the given path (root)
// Returns pointer to GAIADir structure if successful or nullptr on failure.

SSGAIADir *OpenGAIADir ( const string &root )
{
    SSGAIADir *gdp = new SSGAIADir();
    if ( gdp == NULL )
        return nullptr;
    
    gdp->dp = opendir ( root.c_str() );
    if ( gdp->dp == nullptr )
    {
        delete gdp;
        return nullptr;
    }
    
    gdp->root = root;
    return gdp;
}

// Closes GAIADir (gdp) and releases memory; gdp pointer will be invalid after return.

void SSCloseGAIADir ( SSGAIADir *gdp )
{
    closedir ( gdp->dp );
    gzclose ( gdp->fp );
    delete gdp;
}

// Reads one GAIA DR3 source record from directory (gdp) into (rec)
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file.

int SSReadGAIADir ( SSGAIADir *gdp, SSGAIADR3SourceRecord &rec )
{
    int     result = 0;
    string  csv_gz_path;

    if ( gdp == NULL || gdp->dp == NULL )
        return EOF;
    
    if ( gdp->fp )
    {
        result = SSReadGAIADR3SourceRecord ( gdp, rec );
        if ( result != EOF )
            return result;

        gzclose ( gdp->fp );
        gdp->fp = NULL;
    }

    while ( gdp->fp == NULL )
    {
        struct dirent *pDirEnt = readdir ( gdp->dp );
        if ( pDirEnt == NULL )
            return EOF;
        
        size_t len = strlen ( pDirEnt->d_name );
        if ( len < 6 || strcmp ( pDirEnt->d_name + len - 6, "csv.gz" ) != 0 )
            continue;
        
        csv_gz_path = gdp->root;
        if ( csv_gz_path.back() != '/' )
            csv_gz_path += '/';
        csv_gz_path += pDirEnt->d_name;
        
        gdp->fp = gzopen ( csv_gz_path.c_str(), "rb" );
    }

    return SSReadGAIADir ( gdp, rec );
}

// Reads one CSV record from gzip-compressed GAIA data file.
// Returns number of fields in record or -1 (EOF) on failure.
// Field contents are returned in vector of strings (fields)

static int SSReadGAIACSVRecord ( gzFile gz_fp, vector<string> &fields )
{
    int i = 0;
    char csv_buf[4086] = { 0 };
    
    if ( gzgets ( gz_fp, csv_buf, sizeof ( csv_buf ) ) == NULL )
        return ( EOF );
    
    for ( i = (int) strlen ( csv_buf ) - 1; i > 0; i-- )
        if ( csv_buf[i] == '\r' || csv_buf[i] == '\n' )
            csv_buf[i] = 0;
        else
            break;
    
    fields = split_csv ( csv_buf );
    return (int) fields.size();
}

// Reads one record from gzip-compressed GAIA DR3 source file.
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file

int SSReadGAIADR3SourceRecord ( SSGAIADir *gdp, SSGAIADR3SourceRecord &rec )
{
    vector<string> fields;
    int result = SSReadGAIACSVRecord ( gdp->fp, fields );
    if ( result == EOF )
        return result;
    if ( result < GAIADR3_SOURCE_NUM_FIELDS )
        return false;
    
    rec.solution_id = strtoint64 ( fields[0] );
    rec.source_id = strtoint64 ( fields[2] );
    rec.ref_epoch = strtoint64 ( fields[4] );
    rec.ra = strtofloat64 ( fields[5] );
    rec.ra_error = strtofloat64 ( fields[6] );
    rec.dec = strtofloat64 ( fields[7] );
    rec.dec_error = strtofloat64 ( fields[8] );
    rec.parallax = strtofloat64 ( fields[9] );
    rec.parallax_error = strtofloat64 ( fields[10] );
    rec.pmra = strtofloat64 ( fields[13] );
    rec.pmra_error = strtofloat64 ( fields[14] );
    rec.pmdec = strtofloat64 ( fields[15] );
    rec.pmdec_error = strtofloat64 ( fields[16] );
    rec.duplicated_source = fields[64][0] == 'T' ? true : false;
    rec.phot_g_mean_mag = strtofloat ( fields[69] );
    rec.phot_bp_mean_mag = strtofloat ( fields[74] );
    rec.phot_rp_mean_mag = strtofloat ( fields[79] );
    rec.radial_velocity = strtofloat ( fields[89] );
    rec.radial_velocity_error = strtofloat ( fields[90] );
    rec.vbroad = strtofloat ( fields[104] );
    rec.vbroad_error = strtofloat ( fields[105] );
    rec.phot_variable_flag = fields[111][0];
    rec.teff_gspphot = strtofloat ( fields[130] );
    rec.logg_gspphot = strtofloat ( fields[133] );
    rec.mh_gspphot = strtofloat ( fields[136] );
    rec.distance_gspphot = strtofloat ( fields[139] );
    rec.azero_gspphot = strtofloat ( fields[142] );
    rec.ag_gspphot = strtofloat ( fields[145] );
    rec.ebpminrp_gspphot = strtofloat ( fields[148] );
    
    // record must have a non-zero solution and source ID to be valid
    
    return ( rec.solution_id && rec.source_id ) ? true : false;
}

// Reads one record from a GAIA DR3 <-> Hipparcos 2 cross-match file.
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file

static int SSReadGAIADR3Hipparcos2CrossMatchRecord ( gzFile gz_fp, SSGAIACrossMatchRecord &record )
{
    vector<string> fields;
    int result = SSReadGAIACSVRecord ( gz_fp, fields );
    if ( result == EOF )
        return result;
    if ( result < GAIADR3_HIP2_NUM_FIELDS )
        return false;

    record.source_id = strtoint64 ( fields[0] );
    record.ext_source_id = strtoint ( fields[1] );
    record.angular_distance = strtofloat ( fields[2] );
    record.number_of_neighbours = strtoint ( fields[3] );
    record.xm_flag = strtoint ( fields[4] );

    return ( record.source_id && record.ext_source_id ) ? true : false;
}

// Reads one record from a GAIA DR3 <-> Tycho 2 cross-match file.
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file

static int SSReadGAIADR3Tycho2CrossMatchRecord ( gzFile gz_fp, SSGAIACrossMatchRecord &record )
{
    vector<string> fields;
    int result = SSReadGAIACSVRecord ( gz_fp, fields );
    if ( result == EOF )
        return result;
    
    record.source_id = strtoint64 ( fields[0] );
    
    int rgn = 0, num = 0, ext = 0;
    if ( sscanf ( fields[1].c_str(), "%d-%d-%d", &rgn, &num, &ext ) == 3 )
        record.ext_source_id = ext + num * 10LL + rgn * 1000000LL;
    else
        record.ext_source_id = 0;
    
    record.angular_distance = strtofloat ( fields[2] );
    record.xm_flag = strtoint ( fields[3] );
    record.number_of_neighbours = strtoint ( fields[5] );
    
    return ( record.source_id && record.ext_source_id ) ? true : false;
}

// Reads a GAIA cross-match file from the specified path.
// Returns number of records read from file.
// Records are returned in the map of SSGAIACrossMatchRecords (records)

int SSReadGAIACrossMatchFile ( const string &path, SSGAIACrossMatchFile cmf, SSGAIACrossMatch &records )
{
    int       result = 0;
    gzFile    gzfp = NULL;

    gzfp = gzopen ( path.c_str(), "r" );
    if ( gzfp == NULL )
        return NULL;
    
    while ( true )
    {
        SSGAIACrossMatchRecord    record = { 0 };
        
        if ( cmf == kHipparcos2CrossMatch )
            result = SSReadGAIADR3Hipparcos2CrossMatchRecord ( gzfp, record );
        else if ( cmf == kTycho2CrossMatch )
            result = SSReadGAIADR3Tycho2CrossMatchRecord ( gzfp, record );

        if ( result == EOF )
            break;
        
        if ( result == true )
            records[ record.source_id ] = record;
    }

    gzclose ( gzfp );
    return (int) records.size();
}

// Converts GAIA DR3 magnitude sytem (G, G_BP, G_RP) to Tycho magnitude system (V_T, B_T).
// See DAIA DR3 documentation version 1.1, page 349, Table 5.8; reproduced here:
// https://gea.esac.esa.int/archive/documentation/GDR3/Data_processing/chap_cu5pho/cu5pho_sec_photSystem/cu5pho_ssec_photRelations.html

void GAIADR3toTycho2Magnitude ( float g, float gbp, float grp, float &vt, float &bt )
{
    float gbp_grp = clamp ( gbp - grp, -0.35f, 4.0f );
    float gbp_grp2 = gbp_grp * gbp_grp;
    float gbp_grp3 = gbp_grp * gbp_grp2;
    float gbp_grp4 = gbp_grp * gbp_grp3;
    float gbp_grp5 = gbp_grp * gbp_grp4;
    
    float g_vt = -0.01077  - 0.0682 * gbp_grp - 0.2387 * gbp_grp2;
    float g_bt = -0.004288 - 0.8547 * gbp_grp + 0.1244 * gbp_grp2 - 0.9085 * gbp_grp3 + 0.4843 * gbp_grp4 - 0.06814 * gbp_grp5;
    
    vt = g - g_vt;
    bt = g - g_bt;
}

// Converts GAIA DR3 magnitude sytem (G, G_BP, G_RP) to Johnson-Cousins magnitude system (V, R, I_C).
// See DAIA DR3 documentation version 1.1, page 350, Table 5.9; reproduced here:
// https://gea.esac.esa.int/archive/documentation/GDR3/Data_processing/chap_cu5pho/cu5pho_sec_photSystem/cu5pho_ssec_photRelations.html
// Note there is no direct transformation to Johnson B magnitude. The above table gives a
// cubic polynomial transformation from B-V to G-V, but this is not reversible analytically
// or numerically because it gives two solutions when G-V > -0.09 (i.w. when B-V < 0.4).
// Instead, transform GAIA magnitudes to Tycho VT and BT, then Tycho to Johnson V and B.

void GAIADR3toJohnsonMagnitude ( float g, float gbp, float grp, float &vj, float &rj, float &ic )
{
    float gbp_grp = clamp ( gbp - grp, -0.5f, 5.0f );
    float gbp_grp2 = gbp_grp * gbp_grp;
    float gbp_grp3 = gbp_grp * gbp_grp2;
    float gbp_grp4 = gbp_grp * gbp_grp3;
    
    float g_v = -0.02704 + 0.01424 * gbp_grp - 0.2156 * gbp_grp2 + 0.01426 * gbp_grp3;
    float g_r = -0.02275 + 0.3961  * gbp_grp - 0.1243 * gbp_grp2 - 0.01396 * gbp_grp3 + 0.003775 * gbp_grp4;
    float g_i =  0.01753 + 0.76    * gbp_grp - 0.0991 * gbp_grp2;

    vj = g - g_v;
    rj = g - g_r;
    ic = g - g_i;
}

// Exports GAIA DR3 "essentials" from full GAIA source catalog.
// Gzipped GAIA DR2 source files are stored in the root directory.
// Essentials file is written to the output file at (outpath).
// Hipparcos (hipCM) and Tycho (tycCM) cross-match indexes should have been read previously.
// GAIA sources brighter than gmin or fainter than gmax will be discarded.

int SSExportGAIADR3StarData ( const string &root, const string &outpath, const SSGAIACrossMatch &hipCM, const SSGAIACrossMatch &tycCM, float gmin, float gmax )
{
    int     n_outrecs = 0, n_records = 0, result = 0;
    FILE    *outfile = NULL;
    double  startJD = SSTime::fromSystem().jd, endJD = 0;

    // Open GAIA root directory
    
    SSGAIADir *gdp = OpenGAIADir ( root );
    if ( gdp == NULL )
    {
        printf ( "Can't open GAIA directory!\n" );
        return -1;
    }
    
    printf ( "Opened GAIA directory.\n" );
    
    // Attempt to open output file. Return error code on failure.
    
    outfile = fopen ( outpath.c_str(), "wb" );
    if ( outfile == NULL )
    {
        printf ( "Can't open output file %s!\n", outpath.c_str() );
        SSCloseGAIADir ( gdp );
        return -1;
    }
    
    // Read records from GAIA directory individually, until we hit end-of-file.
    // Write essentials from each record to output file.

    while ( true )
    {
        SSGAIADR3SourceRecord record = { 0 };
        result = SSReadGAIADir ( gdp, record );
        if ( result == EOF )
            break;
        
        // skip over invalid records
        
        if ( result == false )
            continue;
        
        n_records++;
        if ( n_records % 1000000 == 0 )
            printf ( "Read %d GAIA records...\n", n_records );

        // Strip out stars brighter than minimum, or fainter than maximum, G magnitude
        
        if ( record.phot_g_mean_mag < gmin || record.phot_g_mean_mag > gmax )
            continue;
        
        SSGAIARec outrec = { 0 };
        outrec.source_id = record.source_id;
        
        auto it = hipCM.find ( record.source_id );
        if ( it != hipCM.end() )
            outrec.hip_source_id = it->second.ext_source_id;

        it = tycCM.find ( record.source_id );
        if ( it != tycCM.end() )
            outrec.tyc_source_id = it->second.ext_source_id;
        
        outrec.ra_mas = record.ra * 3600000.0;
        outrec.dec_mas = record.dec * 3600000.0;
        outrec.pos_error = sqrt ( record.ra_error * record.ra_error + record.dec_error * record.dec_error );
        outrec.parallax = record.parallax;
        outrec.parallax_error = record.parallax_error;
        outrec.pmra_mas = record.pmra;
        outrec.pmdec_mas = record.pmdec;
        outrec.pm_error = sqrt ( record.pmra_error * record.pmra_error + record.pmdec_error * record.pmdec_error );
        outrec.phot_g_mean_mmag = record.phot_g_mean_mag * 1000.0;
        outrec.phot_bp_mean_mmag = record.phot_bp_mean_mag * 1000.0;
        outrec.phot_rp_mean_mmag = record.phot_rp_mean_mag * 1000.0;
        outrec.radial_velocity = record.radial_velocity;
        outrec.radial_velocity_error = record.radial_velocity_error;
        outrec.teff_k = record.teff_gspphot;
        outrec.logg = record.logg_gspphot;
        outrec.distance_pc = record.distance_gspphot;
        outrec.extinction_mmag = record.ag_gspphot * 1000.0;
        outrec.reddening_mmag = record.ebpminrp_gspphot * 1000.0;

        if ( fwrite ( &outrec, sizeof ( outrec ), 1, outfile ) == 1 )
            n_outrecs++;
        else
            printf ( "Failed to write output record for GAIA source ID %lld to %s!\n", outrec.source_id, outpath.c_str() );
    }
    
    // Free memory for array anc close file.
    
    fclose ( outfile );
    printf ( "Wrote %d records to %s, file closed.\n", n_outrecs, outpath.c_str() );
    
    endJD = SSTime::fromSystem().jd;
    printf ( "Elapsed Time: %.02f min\n", SSTime::kMinutesPerDay * ( endJD - startJD ) );
    
    return n_outrecs;
}

// Imports the GAIA17 "essentials" file, generated by the above function, from (filename).
// GAIA-HIP and GAIA-GYC cross-match indexes (hipCM and tycCM) should have been previously
// read by SSReadGAIACrossMatchFile(). GAIA stars are read into a vector of SSObjects (stars).
// If only GAIA stars with HIP or TYC identifiers are desired, set (onlyHIPTYC) to true.
// If stars vector already contains the Hipparocs+Tycho star catalog on input, it will be updated
// with GAIA positions, motions, and idenfifiers, but no additional GAIA stars will be added.
// Returns the total number of GAIA stars imported.

int SSImportGAIA17 ( const string &filename, const SSGAIACrossMatch &hipCM, const SSGAIACrossMatch &tycCM, SSObjectArray &stars, bool onlyHIPTYC )
{
    // Open file; return on failure.

    ifstream file ( filename, ios::binary );
    if ( ! file )
        return 0;

    SSObjectMap hipMap = SSMakeObjectMap ( stars, kCatHIP );
    SSObjectMap tycMap = SSMakeObjectMap ( stars, kCatTYC );

    // Read file record-by-record until we reach end-of-file

    int numStars = 0;
    while ( true )
    {
        SSGAIARec gaia = { 0 };
        file.read ( (char *) &gaia, sizeof ( gaia ) );
        if ( ! file )
            break;
        
        // Get HIP and TYC identifiers from this GAIA star from cross-match indexes
        // because the hip_ and tyc_source_id fields in the gaia record are garbage.
        
        uint64_t hip_source_id = hipCM.count ( gaia.source_id ) ? hipCM.find ( gaia.source_id )->second.ext_source_id : 0;
        uint64_t tyc_source_id = tycCM.count ( gaia.source_id ) ? tycCM.find ( gaia.source_id )->second.ext_source_id : 0;

        if ( onlyHIPTYC && ( hip_source_id == 0 && tyc_source_id == 0 ) )
            continue;
        
        SSSpherical coords ( SSAngle::fromArcsec ( gaia.ra_mas / 1000.0 ), SSAngle::fromArcsec ( gaia.dec_mas / 1000.0 ), INFINITY );
        if ( gaia.parallax > 0.0 && gaia.parallax < 100.0 )
            coords.rad = 1000.0 * SSCoordinates::kLYPerParsec / gaia.parallax;
        
        SSSpherical motion ( SSAngle::fromArcsec ( gaia.pmra_mas / 1000.0 ) / cos ( coords.lat ), SSAngle::fromArcsec ( gaia.pmdec_mas / 1000.0 ), INFINITY );
        if ( gaia.radial_velocity != 0 && gaia.radial_velocity_error != 0 )
            motion.rad = gaia.radial_velocity / SSCoordinates::kLightKmPerSec;
                      
        // If proper motion is valid, use it to update position and proper motion from J2016 to J2000.
        
        if ( ! isinf ( motion.lon ) && ! isinf ( motion.lat ) )
            SSUpdateStarCoordsAndMotion ( 2016.0, nullptr, coords, motion );

        // Convert GAIA magnitudes to Tycho, then Johnson B and V if both GAIA BP and RP colors are valid.
        // Otherwise use GAIA G magnitude as Johnson V.
        
        float vmag = INFINITY, bmag = INFINITY;
        if ( gaia.phot_g_mean_mmag && gaia.phot_bp_mean_mmag && gaia.phot_rp_mean_mmag )
        {
            GAIADR3toTycho2Magnitude ( gaia.phot_g_mean_mmag / 1000.0, gaia.phot_bp_mean_mmag / 1000.0, gaia.phot_rp_mean_mmag / 1000.0, vmag, bmag );
            TychoToJohnsonMagnitude ( bmag, vmag, bmag, vmag );
        }
        else if ( gaia.phot_g_mean_mmag )   // don't have GAIA colors, just assume V is G magnitude
        {
            vmag = gaia.phot_g_mean_mmag / 1000.0;
        }
        
        // Get HIP, TYC, GAIA identifiers
        
        vector<SSIdentifier> idents;
        SSIdentifier hip = hip_source_id ? SSIdentifier ( kCatHIP, hip_source_id ) : SSIdentifier();
        SSIdentifier tyc = tyc_source_id ? SSIdentifier ( kCatTYC, tyc_source_id ) : SSIdentifier();
        SSIdentifier gai = gaia.source_id ? SSIdentifier ( kCatGAIA, gaia.source_id ) : SSIdentifier();

        if ( hip )
            idents.push_back ( hip );
        
        if ( tyc )
            idents.push_back ( tyc );

        if ( gai )
            idents.push_back ( gai );
        
        // If our star vector contained Hipparcos/Tycho stars on input, copy GAIA data into it.
        
        if ( ( hip || tyc ) && ( hipMap.size() || tycMap.size() ) )
        {
            // If GAIA star has a TYC identifier, find corresponding Tycho star.

            SSStarPtr pTycStar = SSGetStarPtr ( SSIdentifierToObject ( tyc, tycMap, stars ) );
            //if ( tyc && pTycStar == nullptr )
            //    cout << "Can't find " << tyc.toString() << endl;
            
            // If GAIA star also has a HIP identifier, find corresponding Hipparcos star,

            SSStarPtr pHipStar = SSGetStarPtr ( SSIdentifierToObject ( hip, hipMap, stars ) );
            //if ( hip && pHipStar == nullptr )
            //    cout << "Can't find " << hip.toString() << endl;
            
            // If GAIA star has both HIP and TYC idents, make sure it's the same as the Tycho star;
            // (some HIP identifiers are duplicated in Tycho).
            
            if ( pHipStar == nullptr && pTycStar == nullptr )
            {
            //    cout << "Can't find match for " << gai.toString() << endl;
                continue;
            }
            else if ( pHipStar && pTycStar && pTycStar != pHipStar )
            {
            //    cout << tyc.toString() << " does not match " << hip.toString() << endl;
                continue;
            }
            
            // Add GAIA identifier.

            pTycStar = pTycStar ? pTycStar : pHipStar;
            pTycStar->addIdentifier ( gai );
            
            // Copy GAIA position and motion, but don't overwrite Tycho distance or tadial velocities
            // with GAIA unknown distance or radial velocity.
            
            if ( ::isinf ( coords.rad ) )
                coords.rad = SSCoordinates::kLYPerParsec / pTycStar->getParallax();
            if ( ::isinf ( motion.rad ) )
                motion.rad = pTycStar->getRadVel();
            pTycStar->setFundamentalMotion ( coords, motion );
            
            // Don't copy GAGA magnitudes until we resolve differences between G and Tyc systems
            //if ( ! ::isinf ( vmag ) )
            //    pTycStar->setVMagnitude ( vmag );
            //if ( ! ::isinf ( bmag ) )
            //    pTycStar->setBMagnitude ( bmag );

            numStars++;
            continue;
        }
        
        // Otherwise, append a new GAIA star to the star vector.

        SSObjectPtr pObj = SSNewObject ( kTypeStar );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        if ( pStar != nullptr )
        {
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( coords, motion );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );
            
            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }
    
    return numStars;
}

#endif // __APPLE__
