#include "AliHLTTRDTrack.h"
#include "AliESDtrack.h"
#include "AliHLTTRDTrackData.h"

ClassImp(AliHLTTRDTrack);

AliHLTTRDTrack::AliHLTTRDTrack() :
  fTPCtrackId(0),
  fNtracklets(0),
  fNlayers(0),
  fNmissingConsecLayers(0),
  fNtrackletsOffline(0),
  fIsStopped(false)
{
  //------------------------------------------------------------------
  //Default constructor
  //------------------------------------------------------------------
  for (Int_t i=0; i<=5; ++i) {
    fAttachedTracklets[i] = -1;
  }
}


AliHLTTRDTrack::AliHLTTRDTrack(const AliHLTTRDTrack& t) :
  AliKalmanTrack(t),
  fTPCtrackId( t.fTPCtrackId),
  fNtracklets( t.fNtracklets),
  fNlayers( t.fNlayers),
  fNmissingConsecLayers( t.fNmissingConsecLayers),
  fNtrackletsOffline( t.fNtrackletsOffline),
  fIsStopped( t.fIsStopped)
{
  //------------------------------------------------------------------
  //Copy constructor
  //------------------------------------------------------------------
  for (Int_t i=0; i<=5; ++i) {
    fAttachedTracklets[i] = t.fAttachedTracklets[i];
  }
}


AliHLTTRDTrack &AliHLTTRDTrack::operator=(const AliHLTTRDTrack& t)
{
  //------------------------------------------------------------------
  //Assignment operator
  //------------------------------------------------------------------
  if( &t==this ) return *this;
  *(AliKalmanTrack*)this = t;
  fTPCtrackId = t.fTPCtrackId;
  fNtracklets = t.fNtracklets;
  fNlayers = t.fNlayers;
  fNmissingConsecLayers = t.fNmissingConsecLayers;
  fNtrackletsOffline = t.fNtrackletsOffline;
  fIsStopped = t.fIsStopped;
  for (Int_t i=0; i<=5; ++i) {
    fAttachedTracklets[i] = t.fAttachedTracklets[i];
  }
  return *this;
}


AliHLTTRDTrack::AliHLTTRDTrack(AliESDtrack& t,Bool_t c) throw (const Char_t *) :
  AliKalmanTrack(),
  fTPCtrackId(0),
  fNtracklets(0),
  fNlayers(0),
  fNmissingConsecLayers(0),
  fNtrackletsOffline(0),
  fIsStopped(false)
{
  //------------------------------------------------------------------
  // Conversion ESD track -> TRD HLT track.
  // If c==kTRUE, create the TRD track out of the constrained params.
  //------------------------------------------------------------------
  const AliExternalTrackParam *par=&t;
  if (c) {
    par=t.GetConstrainedParam();
    if (!par) throw "AliHLTTRDTrack: conversion failed !\n";
  }
  Set(par->GetX(),par->GetAlpha(),par->GetParameter(),par->GetCovariance());
  for (Int_t i=0; i<=5; ++i) {
    fAttachedTracklets[i] = -1;
  }
}

AliHLTTRDTrack::AliHLTTRDTrack(AliExternalTrackParam& t ) throw (const Char_t *) :
  AliKalmanTrack(),
  fTPCtrackId(0),
  fNtracklets(0),
  fNlayers(0),
  fNmissingConsecLayers(0),
  fNtrackletsOffline(0),
  fIsStopped(false)
{
  //------------------------------------------------------------------
  // Conversion ESD track -> TRD track.
  // If c==kTRUE, create the TRD track out of the constrained params.
  //------------------------------------------------------------------
  const AliExternalTrackParam *par=&t;
  Set(par->GetX(),par->GetAlpha(),par->GetParameter(),par->GetCovariance());
  for (Int_t i=0; i<=5; ++i) {
    fAttachedTracklets[i] = -1;
  }
}


Int_t AliHLTTRDTrack::GetTracklet(Int_t iLayer) const
{
  if (iLayer < 0 || iLayer > 5) {
    //Error("GetTracklet", "illegal layer number %i", iLayer);
    return -1;
  }
  return fAttachedTracklets[iLayer];
}


void AliHLTTRDTrack::ConvertTo( AliHLTTRDTrackDataRecord &t ) const
{
  // convert to HLT structure

  t.fAlpha = GetAlpha();
  t.fX = GetX();
  t.fY = GetY();
  t.fZ = GetZ();
  t.fq1Pt = GetSigned1Pt();
  t.fSinPhi = GetSnp();
  t.fTgl = GetTgl();
  for( int i=0; i<15; i++ ) t.fC[i] = GetCovariance()[i];
  t.fTPCTrackID = GetTPCtrackId();
  for ( int i = 0; i <6; i++ ){
    t.fAttachedTracklets[ i ] = GetTracklet( i );
  }
}

void AliHLTTRDTrack::ConvertFrom( const AliHLTTRDTrackDataRecord &t )
{
  // convert from HLT structure

  Set(t.fX, t.fAlpha, &(t.fY), t.fC);
  SetTPCtrackId( t.fTPCTrackID );
  fNtracklets = 0;
  fNlayers = 0;
  fNmissingConsecLayers = 0;
  fNtrackletsOffline = 0;
  fIsStopped = false;
  for ( int iLayer=0; iLayer <6; iLayer++ ){
    fAttachedTracklets[iLayer] = t.fAttachedTracklets[ iLayer ];
    if( fAttachedTracklets[iLayer]>=0 ) fNtracklets++;
  }
}