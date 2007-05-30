#ifndef ALI_TEST_SHUTTLE_H
#define ALI_TEST_SHUTTLE_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

//
// test implementation of the AliShuttleInterface, to be used for local tests of preprocessors
//

#include <AliShuttleInterface.h>
#include <TString.h>

class TMap;
class TList;
class AliCDBMetaData;
class AliCDBPath;
class AliCDBPath;

class AliTestShuttle : public AliShuttleInterface
{
  public:
    AliTestShuttle(Int_t run, UInt_t startTime, UInt_t endTime);
    virtual ~AliTestShuttle();

    void AddInputFile(Int_t system, const char* detector, const char* id, const char* source, const char* fileName);
    void SetDCSInput(TMap* dcsAliasMap) { fDcsAliasMap = dcsAliasMap; }
    void AddInputRunParameter(const char* key, const char* value);
    void SetInputRunType(const char* runType) { fRunType = runType; }
    Bool_t AddInputCDBEntry(AliCDBEntry* entry);

    void Process();

    // AliShuttleInterface functions
    virtual Bool_t Store(const AliCDBPath& path, TObject* object, AliCDBMetaData* metaData,
        				Int_t validityStart = 0, Bool_t validityInfinite = kFALSE);
    virtual Bool_t StoreReferenceData(const AliCDBPath& path, TObject* object, AliCDBMetaData* metaData);
    virtual Bool_t StoreReferenceFile(const char* detector, const char* localFile, const char* gridFileName);
    virtual const char* GetFile(Int_t system, const char* detector, const char* id, const char* source);
    virtual TList* GetFileSources(Int_t system, const char* detector, const char* id = 0);
    virtual TList* GetFileIDs(Int_t system, const char* detector, const char* source);
    virtual const char* GetRunParameter(const char* key);
    virtual AliCDBEntry* GetFromOCDB(const char* detector, const AliCDBPath& path);
    virtual const char* GetRunType();
    virtual void Log(const char* detector, const char* message);

    virtual void RegisterPreprocessor(AliPreprocessor* preprocessor);

    static void SetMainCDB (TString mainCDB) {fgkMainCDB = mainCDB;}
    static void SetLocalCDB (TString localCDB) {fgkLocalCDB = localCDB;}

    static void SetMainRefStorage (TString mainRefStorage) {fgkMainRefStorage = mainRefStorage;}
    static void SetLocalRefStorage (TString localRefStorage) {fgkLocalRefStorage = localRefStorage;}

    static void SetShuttleTempDir (const char* tmpDir);
    static void SetShuttleLogDir (const char* logDir);

  protected:

    Int_t fRun;         // run that is simulated with the AliTestShuttle
    UInt_t fStartTime;  // starttime that is simulated with the AliTestShuttle
    UInt_t fEndTime;    // endtime that is simulated with the AliTestShuttle

    TMap* fInputFiles;      // files for GetFile, GetFileSources
    TMap* fRunParameters;   // run parameters
    TString fRunType; 	    // run type
    TObjArray* fPreprocessors; // list of preprocessors that are to be tested
    TMap* fDcsAliasMap; // DCS data for testing

  private:
    ClassDef(AliTestShuttle, 0);
};

#endif
