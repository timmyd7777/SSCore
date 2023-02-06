// SSImportGAIADR3.hpp
// SSCore
//
// Created by Tim DeBenedictis on 1/29/23.
// Copyright ©2023 Southern Stars Group, LLC. All rights reserved.

#ifndef SSImportGAIADR3_hpp
#define SSImportGAIADR3_hpp

#include "SSStar.hpp"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Represents one record from a GAIA Dr3 source file

struct SSGAIADR3SourceRecord
{
    int64_t     solution_id;            // Solution Identifier
    int64_t     source_id;              // Unique source identifier within Data Release 3
    float       ref_epoch;              // Reference epoch [Julian Year]
    double      ra;                     // Barycentric right ascension in ICRS at the reference epoch [deg]
    double      ra_error;               // Standard error of right ascension [mas]
    double      dec;                    // Barycentric declination in ICRS at the reference epoch [deg]
    double      dec_error;              // Standard error of declination [mas]
    double      parallax;               // Absolute stellar parallax at the reference epoch [mas]
    double      parallax_error;         // Standard error of parallax [mas]
    double      pmra;                   // Proper motion in right ascension * cos (dec) in ICRS at the reference epoch [mas/year]
    double      pmra_error;             // Standard error of proper motion in right ascension * cos (dec)
    double      pmdec;                  // Proper motion in declination [mas/year]
    double      pmdec_error;            // Standard error of proper motion in declination direction [mas/year]
    bool        duplicated_source;      // Source was duplicated during data processing and only one source identifier has been kept.
    float       phot_g_mean_mag;        // G-band mean magnitude
    float       phot_bp_mean_mag;       // Integrated BP mean magnitude
    float       phot_rp_mean_mag;       // Integrated RP mean magnitude
    double      radial_velocity;        // Radial velocity [km/s]
    double      radial_velocity_error;  // Radial velocity error [km/s]
    float       vbroad;                 // Spectral line broadening parameter [km/sec]
    float       vbroad_error;           // Uncertainty on the spectral line broadening [km/sec]
    char        phot_variable_flag;     // Photometric variability flag: N = not available, C = constant, V = variable
    float       teff_gspphot;           // stellar effective temperature [K]
    float       logg_gspphot;           // Surface gravity from GSP-Phot Aeneas best library using BP/RP spectra [log(cm/sec^2)]
    float       mh_gspphot;             // Iron abundance from GSP-Phot Aeneas best library using BP/RP spectra [dex]
    float       distance_gspphot;       // Distance from GSP-Phot Aeneas best library using BP/RP spectra [pc]
    float       azero_gspphot;          // Monochromatic extinction A0 at 547.7nm from GSP-Phot Aeneas best library using BP/RP spectra [mag]
    float       ag_gspphot;             // Extinction in G band from GSP-Phot Aeneas best library using BP/RP spectra [mag]
    float       ebpminrp_gspphot;       // Reddening E(BP - RP) from GSP-Phot Aeneas best library using BP/RP spectra [mag]
};

// Condensed GAIA record, containing just the essentials

#pragma pack ( push, 1 )

struct SSGAIARec
{
    uint64_t    source_id;              // Unique GAIA DR3 source identifier
    uint64_t    tyc_source_id;          // Unique Tycho-2 catalogue source identifier
    uint32_t    hip_source_id;          // Unique Hipparcos-2 catalogue source identifier
    int32_t     ra_mas;                 // Barycentric right ascension in ICRS at epoch 2016.0 [mas]
    int32_t     dec_mas;                // Barycentric declination in ICRS at epoch 2016.0 [mas]
    float       pos_error;              // Standard error of position = sqrt ( ra_error^2 + dec_error^2 ) [mas]
    float       parallax;               // Absolute stellar parallax at epoch 2016.0 [mas]
    float       parallax_error;         // Standard error of parallax [mas]
    float       pmra_mas;               // Proper motion in right ascension * cos (dec) in ICRS at epoch 2016.0 [mas/year]
    float       pmdec_mas;              // Proper motion in declination at epoch 2016.0 [mas/year]
    float       pm_error;               // Standard error of proper motion = sqrt ( pm_ra_error^2 + pm_dec_error^2 ) [mas/year]
    int16_t     phot_g_mean_mmag;       // G-band mean magnitude [millimag]
    int16_t     phot_bp_mean_mmag;      // Integrated BP mean magnitude [millimag]
    int16_t     phot_rp_mean_mmag;      // Integrated RP mean magnitude [millimag]
    float       radial_velocity;        // Radial velocity [km/s]
    float       radial_velocity_error;  // Radial velocity error [km/s]
    uint16_t    teff_k;                 // Stellar effective temperature [K]
    float       logg;                   // Surface gravity from GSP-Phot Aeneas best library using BP/RP spectra [log(cm/sec^2)]
    float       distance_pc;            // Distance from GSP-Phot Aeneas best library using BP/RP spectra [pc]
    int16_t     extinction_mmag;        // Extinction in G band from GSP-Phot Aeneas best library using BP/RP spectra [millimag]
    int16_t     reddening_mmag;         // Reddening E(BP - RP) from GSP-Phot Aeneas best library using BP/RP spectra [millimag]
};

#pragma pack ( pop )

// Identifiers for the GAIA cross-match files that we can parse

enum SSGAIACrossMatchFile
{
    kHipparcos2CrossMatch = 1,          // Hipparcos2BestNeighbour.csv, 99525 records
    kTycho2CrossMatch = 2               // Tycho2tdscMergeBestNeighbour.csv, 2511088 records
};

// Represents one record from a GAIA cross-match file

struct SSGAIACrossMatchRecord
{
    uint64_t    source_id = 0;              // Unique Gaia source identifier
    uint64_t    ext_source_id = 0;          // Unique external catalogue source identifier
    float       angular_distance = 0;       // Angular distance between the two sources [arcsec]
    int8_t      number_of_neighbours = 0;   // Number of neighbours in external Ccatalogue which match the Gaia source within position errors.
    uint8_t     xm_flag = 0;                // Cross-match algorithm flag; see documentation
};

// Represents an entire GAIA cross-match file, indexed by GAIA DR3 source_id for fast lookups

typedef map<uint64_t,SSGAIACrossMatchRecord> SSGAIACrossMatch;

struct SSGAIADir;   // Forward declaration of opaque GAIA directory/file reference

SSGAIADir *SSOpenGAIADir ( const string &root );
int SSReadGAIADir ( SSGAIADir *gdp, SSGAIADR3SourceRecord &record );
void SSCloseGAIADir ( SSGAIADir *gdp );

int SSReadGAIACrossMatchFile ( const string &path, SSGAIACrossMatchFile cmf, SSGAIACrossMatch &records );
int SSReadGAIADR3SourceRecord ( SSGAIADir *gdp, SSGAIADR3SourceRecord &record );
int SSExportGAIADR3StarData ( const string &root, const string &outpath, const SSGAIACrossMatch &hipCM, const SSGAIACrossMatch &tycCM, float gmin, float gmax, bool onlyHIPTYC );
int SSImportGAIA17 ( const string &filename, SSObjectArray &stars, bool onlyHIPTYC );

void GAIADR3toTycho2Magnitude ( float g, float gbp, float grp, float &vt, float &bt );
void GAIADR3toJohnsonMagnitude ( float g, float gbp, float grp, float &vj, float &rj, float &ij );

#endif /* SSImportGAIADR3_hpp */
