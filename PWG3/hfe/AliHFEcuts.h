/**************************************************************************
* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
*                                                                        *
* Author: The ALICE Off-line Project.                                    *
* Contributors are mentioned in the code where appropriate.              *
*                                                                        *
* Permission to use, copy, modify and distribute this software and its   *
* documentation strictly for non-commercial purposes is hereby granted   *
* without fee, provided that the above copyright notice appears in all   *
* copies and that both the copyright notice and this permission notice   *
* appear in the supporting documentation. The authors make no claims     *
* about the suitability of this software for any purpose. It is          *
* provided "as is" without express or implied warranty.                  *
**************************************************************************/
#ifndef __ALIHFECUTS_H__
#define __ALIHFECUTS_H__

#ifndef ROOT_TObject
#include <TObject.h>
#endif

#ifndef __ALIHFELECTRONEXTRACUTS_H__
#include "AliHFEextraCuts.h"
#endif

class AliCFManager;
class AliESDtrack;
class AliMCParticle;

class TObjArray;
class TList;

class AliHFEcuts : public TObject{
  enum{
    kDebugMode = BIT(14)
      };
  typedef enum{
    kPrimary = 0,
      kProductionVertex = 1,
      kSigmaToVertex = 2,
      kDCAToVertex = 3,
      kITSPixel = 4,
      kMaxImpactParam = 5
      } Require_t;
 public:

  typedef enum{
    kStepMCGenerated = 0,
      kStepMCInAcceptance = 1,
      kStepRecKine = 2,
      kStepRecPrim = 3,
      kStepHFEcuts = 4
      } CutStep_t;

  static const Int_t kNcutSteps;

  AliHFEcuts();
  AliHFEcuts(const AliHFEcuts &c);
  AliHFEcuts &operator=(const AliHFEcuts &c);
  ~AliHFEcuts();
    
  void Initialize(AliCFManager *cfm);
  void Initialize();

  Bool_t CheckParticleCuts(CutStep_t step, TObject *o);

  TList *GetQAhistograms() const { return fHistQA; }
    
  void SetDebugMode();
  void UnsetDebugMode() { SetBit(kDebugMode, kFALSE); }
  Bool_t IsInDebugMode() const { return TestBit(kDebugMode); };
    
  // Getters
  Bool_t IsRequireITSpixel() const { return TESTBIT(fRequirements, kITSPixel); };
  Bool_t IsRequireMaxImpactParam() const { return TESTBIT(fRequirements, kMaxImpactParam); };
  Bool_t IsRequirePrimary() const { return TESTBIT(fRequirements, kPrimary); };
  Bool_t IsRequireProdVertex() const { return TESTBIT(fRequirements, kProductionVertex); };
  Bool_t IsRequireSigmaToVertex() const { return TESTBIT(fRequirements, kSigmaToVertex); };
  Bool_t IsRequireDCAToVertex() const {return TESTBIT(fRequirements, kDCAToVertex); };
    
  // Setters
  inline void SetCutITSpixel(UChar_t cut);
  void SetMinNClustersTPC(UChar_t minClustersTPC) { fMinClustersTPC = minClustersTPC; }
  void SetMinNTrackletsTRD(UChar_t minNtrackletsTRD) { fMinTrackletsTRD = minNtrackletsTRD; }
  void SetMaxChi2perClusterTPC(Double_t chi2) { fMaxChi2clusterTPC = chi2; };
  inline void SetMaxImpactParam(Double_t radial, Double_t z);
  void SetMinRatioTPCclusters(Double_t minRatioTPC) { fMinClusterRatioTPC = minRatioTPC; };
  void SetPtRange(Double_t ptmin, Double_t ptmax){fPtRange[0] = ptmin; fPtRange[1] = ptmax;};
  inline void SetProductionVertex(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax);
  inline void SetSigmaToVertex(Double_t sig);
    
  inline void CreateStandardCuts();
    
  // Requirements
  void SetRequireDCAToVertex() { SETBIT(fRequirements, kDCAToVertex); };
  void SetRequireIsPrimary() { SETBIT(fRequirements, kPrimary); };
  void SetRequireITSPixel() { SETBIT(fRequirements, kITSPixel); }
  void SetRequireMaxImpactParam() { SETBIT(fRequirements, kMaxImpactParam); };
  void SetRequireProdVetrex() { SETBIT(fRequirements, kProductionVertex); };
  void SetRequireSigmaToVertex() { SETBIT(fRequirements, kSigmaToVertex); };
    
 private:
  void SetParticleGenCutList();
  void SetAcceptanceCutList();
  void SetRecKineCutList();
  void SetRecPrimaryCutList();
  void SetHFElectronCuts();
    
  ULong64_t fRequirements;	// Bitmap for requirements
  Double_t fDCAtoVtx[2];	// DCA to Vertex
  Double_t fProdVtx[4];	// Production Vertex
  Double_t fPtRange[2];	// pt range
  UChar_t fMinClustersTPC;	// Min.Number of TPC clusters
  UChar_t fMinTrackletsTRD;	// Min. Number of TRD tracklets
  UChar_t fCutITSPixel;	// Cut on ITS pixel
  Double_t fMaxChi2clusterTPC;	// Max Chi2 per TPC cluster
  Double_t fMinClusterRatioTPC;	// Min. Ratio findable / found TPC clusters
  Double_t fSigmaToVtx;	// Sigma To Vertex
  Double_t fMaxImpactParamR;	// Max. Impact Parameter in Radial Direction
  Double_t fMaxImpactParamZ;	// Max. Impact Parameter in Z Direction
    
  TList *fHistQA;		//! QA Histograms
  TObjArray *fCutList;	//! List of cut objects(Correction Framework Manager)
    
  ClassDef(AliHFEcuts, 1)   // Container for HFE cuts
    };

    //__________________________________________________________________
    void AliHFEcuts::SetProductionVertex(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax){
      // Set the production vertex constraint
      SetRequireProdVetrex();
      fProdVtx[0] = xmin;
      fProdVtx[1] = xmax;
      fProdVtx[2] = ymin;
      fProdVtx[3] = ymax;
    }

//__________________________________________________________________
void AliHFEcuts::SetSigmaToVertex(Double_t sig){
  SetRequireSigmaToVertex();
  fSigmaToVtx = sig;
}

//__________________________________________________________________
void AliHFEcuts::SetMaxImpactParam(Double_t radial, Double_t z){
  SetRequireMaxImpactParam();
  fMaxImpactParamR = radial;
  fMaxImpactParamZ = z;
}

//__________________________________________________________________
void AliHFEcuts::SetCutITSpixel(UChar_t cut){
  SetRequireITSPixel();
  fCutITSPixel = cut;
}

//__________________________________________________________________
void AliHFEcuts::CreateStandardCuts(){
  //
  // Standard Cuts defined by the HFE Group
  //
  SetRequireProdVetrex();
  fProdVtx[0] = -1;
  fProdVtx[1] = 1;
  fProdVtx[2] = -1;
  fProdVtx[3] = 1;
  SetRequireDCAToVertex();
  fDCAtoVtx[0] = 4.;
  fDCAtoVtx[1] = 10.;
  fMinClustersTPC = 50;
  fMinTrackletsTRD = 6;
  fCutITSPixel = AliHFEextraCuts::kAny;
  fMaxChi2clusterTPC = 3.5;
  fMinClusterRatioTPC = 0.6;
  fPtRange[0] = 0.1;
  fPtRange[1] = 20.;
  fSigmaToVtx = 4.;
  SetRequireMaxImpactParam();
  fMaxImpactParamR = 3.;
  fMaxImpactParamZ = 12.;
}
#endif
