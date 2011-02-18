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

/* $Id$ */

// Generator using AMPT as an external generator

#include "AliGenAmpt.h"

#include <TClonesArray.h>
#include <TGraph.h>
#include <TAmpt.h>
#include <TLorentzVector.h>
#include <TPDGCode.h>
#include <TParticle.h>

#include "AliGenHijingEventHeader.h"
#define AliGenAmptEventHeader AliGenHijingEventHeader
#include "AliAmptRndm.h"
#include "AliLog.h"
#include "AliRun.h"

ClassImp(AliGenAmpt)

AliGenAmpt::AliGenAmpt() 
  : AliGenMC(),
    fFrame("CMS"),
    fMinImpactParam(0.),
    fMaxImpactParam(5.),
    fKeep(0),
    fQuench(0),
    fShadowing(1),
    fDecaysOff(1),
    fTrigger(0),     
    fEvaluate(0),
    fSelectAll(0),
    fFlavor(0),
    fKineBias(0.),
    fTrials(0),
    fXsection(0.),
    fAmpt(0),
    fPtHardMin(2.0),
    fPtHardMax(-1),
    fSpectators(1),
    fDsigmaDb(0),
    fDnDb(0),
    fPtMinJet(-2.5),
    fEtaMinJet(-20.),
    fEtaMaxJet(+20.),
    fPhiMinJet(0.),
    fPhiMaxJet(TMath::TwoPi()),
    fRadiation(3),
    fSimpleJet(kFALSE),
    fNoGammas(kFALSE),
    fProjectileSpecn(0),
    fProjectileSpecp(0),
    fTargetSpecn(0),
    fTargetSpecp(0),
    fLHC(kFALSE),
    fRandomPz(kFALSE),
    fNoHeavyQuarks(kFALSE),
    fEventTime(0.),
    fIsoft(1),
    fNtMax(150),
    fIpop(1),
    fXmu(3.2264),
    fAlpha(1./3),
    fStringA(0.5),
    fStringB(0.9),
    fHeader(new AliGenAmptEventHeader("Ampt"))
{
  // Constructor
  fEnergyCMS = 5500.;
  AliAmptRndm::SetAmptRandom(GetRandom());
}

AliGenAmpt::AliGenAmpt(Int_t npart)
  : AliGenMC(npart),
    fFrame("CMS"),
    fMinImpactParam(0.),
    fMaxImpactParam(5.),
    fKeep(0),
    fQuench(0),
    fShadowing(1),
    fDecaysOff(1),
    fTrigger(0),     
    fEvaluate(0),
    fSelectAll(0),
    fFlavor(0),
    fKineBias(0.),
    fTrials(0),
    fXsection(0.),
    fAmpt(0),
    fPtHardMin(2.0),
    fPtHardMax(-1),
    fSpectators(1),
    fDsigmaDb(0),
    fDnDb(0),
    fPtMinJet(-2.5),
    fEtaMinJet(-20.),
    fEtaMaxJet(+20.),
    fPhiMinJet(0.),
    fPhiMaxJet(2. * TMath::Pi()),
    fRadiation(3),
    fSimpleJet(kFALSE),
    fNoGammas(kFALSE),
    fProjectileSpecn(0),
    fProjectileSpecp(0),
    fTargetSpecn(0),
    fTargetSpecp(0),
    fLHC(kFALSE),
    fRandomPz(kFALSE),
    fNoHeavyQuarks(kFALSE),
    fEventTime(0.),
    fIsoft(1),
    fNtMax(150),
    fIpop(1),
    fXmu(3.2264),
    fAlpha(1./3),
    fStringA(0.5),
    fStringB(0.9),
    fHeader(new AliGenAmptEventHeader("Ampt"))
{
  // Default PbPb collisions at 5.5 TeV

  fEnergyCMS = 5500.;
  fName = "Ampt";
  fTitle= "Particle Generator using AMPT";
  AliAmptRndm::SetAmptRandom(GetRandom());
}

AliGenAmpt::~AliGenAmpt()
{
  // Destructor
  if ( fDsigmaDb) delete fDsigmaDb;  
  if ( fDnDb)     delete fDnDb;
  if ( fHeader)   delete fHeader;
}

void AliGenAmpt::Init()
{
  // Initialisation

  fFrame.Resize(8);
  fTarget.Resize(8);
  fProjectile.Resize(8);

  fAmpt = new TAmpt(fEnergyCMS, fFrame, fProjectile, fTarget, 
                    fAProjectile, fZProjectile, fATarget, fZTarget, 
                    fMinImpactParam, fMaxImpactParam);
  SetMC(fAmpt);

  fAmpt->SetIHPR2(2,  fRadiation);
  fAmpt->SetIHPR2(3,  fTrigger);
  fAmpt->SetIHPR2(6,  fShadowing);
  fAmpt->SetIHPR2(12, fDecaysOff);    
  fAmpt->SetIHPR2(21, fKeep);
  fAmpt->SetHIPR1(8,  fPtHardMin); 	
  fAmpt->SetHIPR1(9,  fPtHardMax); 	
  fAmpt->SetHIPR1(10, fPtMinJet); 	
  fAmpt->SetHIPR1(50, fSimpleJet);

  //  Quenching
  //  fQuench = 0:  no quenching
  //  fQuench = 1:  Hijing default
  //  fQuench = 2:  new LHC  parameters for HIPR1(11) and HIPR1(14)
  //  fQuench = 3:  new RHIC parameters for HIPR1(11) and HIPR1(14)
  //  fQuench = 4:  new LHC  parameters with log(e) dependence
  //  fQuench = 5:  new RHIC parameters with log(e) dependence
  fAmpt->SetIHPR2(50, 0);
  if (fQuench > 0) 
    fAmpt->SetIHPR2(4,  1);
  else
    fAmpt->SetIHPR2(4,  0);

  if (fQuench == 2) {
    fAmpt->SetHIPR1(14, 1.1);
    fAmpt->SetHIPR1(11, 3.7);
  } else if (fQuench == 3) {
    fAmpt->SetHIPR1(14, 0.20);
    fAmpt->SetHIPR1(11, 2.5);
  } else if (fQuench == 4) {
    fAmpt->SetIHPR2(50, 1);
    fAmpt->SetHIPR1(14, 4.*0.34);
    fAmpt->SetHIPR1(11, 3.7);
  } else if (fQuench == 5) {
    fAmpt->SetIHPR2(50, 1);
    fAmpt->SetHIPR1(14, 0.34);
    fAmpt->SetHIPR1(11, 2.5);
  }
    
  // Heavy quarks
  if (fNoHeavyQuarks) {
    fAmpt->SetIHPR2(49, 1);
  } else {
    fAmpt->SetIHPR2(49, 0);
  }

  // Ampt specific
  fAmpt->SetIsoft(fIsoft);
  fAmpt->SetNtMax(fNtMax);
  fAmpt->SetIpop(fIpop);
  fAmpt->SetXmu(fXmu);
  fAmpt->SetAlpha(fAlpha);
  fAmpt->SetStringFrag(fStringA, fStringB);

  AliGenMC::Init();
    
  // Initialize Ampt  
  fAmpt->Initialize();
  if (fEvaluate) 
    EvaluateCrossSections();
}

void AliGenAmpt::Generate()
{
  // Generate one event

  Float_t polar[3]    =   {0,0,0};
  Float_t origin[3]   =   {0,0,0};
  Float_t origin0[3]  =   {0,0,0};
  Float_t p[3];
  Float_t tof;

  //  converts from mm/c to s
  const Float_t kconv = 0.001/2.99792458e8;

  Int_t nt  = 0;
  Int_t jev = 0;
  Int_t j, kf, ks, ksp, imo;
  kf = 0;
    
  fTrials = 0;
  for (j = 0;j < 3; j++) 
    origin0[j] = fOrigin[j];

  if(fVertexSmear == kPerEvent) {
    Vertex();
    for (j=0; j < 3; j++) 
      origin0[j] = fVertex[j];
  } 

  Float_t sign = (fRandomPz && (Rndm() < 0.5))? -1. : 1.;

  while(1) {
    // Generate one event
    Int_t fpemask = gSystem->GetFPEMask();
    gSystem->SetFPEMask(0);
    fAmpt->GenerateEvent();
    gSystem->SetFPEMask(fpemask);
    fTrials++;
    fNprimaries = 0;
    fAmpt->ImportParticles(&fParticles,"All");
    Int_t np = fParticles.GetEntriesFast();
    if (np == 0 ) 
      continue;

    if (fTrigger != kNoTrigger) {
      if (!CheckTrigger()) 
        continue;
    }
    if (fLHC) 
      Boost();
      
    Int_t nc = 0;
    Int_t* newPos     = new Int_t[np];
    Int_t* pSelected  = new Int_t[np];

    for (Int_t i = 0; i < np; i++) {
      newPos[i]    = i;
      pSelected[i] = 0;
    }
      
    // Get event vertex
    //TParticle *  iparticle = (TParticle *) fParticles.At(0);
    fVertex[0] = origin0[0];
    fVertex[1] = origin0[1];	
    fVertex[2] = origin0[2];
      
    // First select parent particles
    for (Int_t i = 0; i < np; i++) {
      TParticle *iparticle = (TParticle *) fParticles.At(i);

    // Is this a parent particle ?
      if (Stable(iparticle)) continue;
      Bool_t  selected             =  kTRUE;
      Bool_t  hasSelectedDaughters =  kFALSE;
      kf        = iparticle->GetPdgCode();
      ks        = iparticle->GetStatusCode();
      if (kf == 92) 
        continue;
	    
      if (!fSelectAll) 
        selected = KinematicSelection(iparticle, 0) && SelectFlavor(kf);
      hasSelectedDaughters = DaughtersSelection(iparticle);

      // Put particle on the stack if it is either selected or 
      // it is the mother of at least one seleted particle
      if (selected || hasSelectedDaughters) {
        nc++;
        pSelected[i] = 1;
      } // selected
    } // particle loop parents

    // Now select the final state particles
    fProjectileSpecn    = 0;  
    fProjectileSpecp    = 0;
    fTargetSpecn        = 0;  
    fTargetSpecp        = 0;
    for (Int_t i = 0; i<np; i++) {
      TParticle *iparticle = (TParticle *) fParticles.At(i);
      // Is this a final state particle ?
      if (!Stable(iparticle)) 
        continue;
      Bool_t  selected             =  kTRUE;
      kf        = iparticle->GetPdgCode();
      ks        = iparticle->GetStatusCode();
      ksp       = iparticle->GetUniqueID();
	  
      // --------------------------------------------------------------------------
      // Count spectator neutrons and protons
      if(ksp == 0 || ksp == 1) {
        if(kf == kNeutron) fProjectileSpecn += 1;
        if(kf == kProton)  fProjectileSpecp += 1;
      } else if(ksp == 10 || ksp == 11) {
        if(kf == kNeutron) fTargetSpecn += 1;
        if(kf == kProton)  fTargetSpecp += 1;
      }
      // --------------------------------------------------------------------------
      if (!fSelectAll) {
        selected = KinematicSelection(iparticle,0)&&SelectFlavor(kf);
        if (!fSpectators && selected) 
          selected = (ksp != 0 && ksp != 1 && ksp != 10 && ksp != 11);
      }

     // Put particle on the stack if selected
      if (selected) {
        nc++;
        pSelected[i] = 1;
      } // selected
    } // particle loop final state

    //Time of the interactions
    Float_t tInt = 0.;
    if (fPileUpTimeWindow > 0.) 
      tInt = fPileUpTimeWindow * (2. * gRandom->Rndm() - 1.);

    // Write particles to stack
    for (Int_t i = 0; i<np; i++) {
      TParticle *iparticle = (TParticle *) fParticles.At(i);
      Bool_t  hasMother   = (iparticle->GetFirstMother()     >=0);
      Bool_t  hasDaughter = (iparticle->GetFirstDaughter()   >=0);
      if (pSelected[i]) {
        kf   = iparticle->GetPdgCode();
        ks   = iparticle->GetStatusCode();
        p[0] = iparticle->Px();
        p[1] = iparticle->Py();
        p[2] = iparticle->Pz() * sign;
        origin[0] = origin0[0]+iparticle->Vx()/10;
        origin[1] = origin0[1]+iparticle->Vy()/10;
        origin[2] = origin0[2]+iparticle->Vz()/10;
        fEventTime = 0.;
	      
        if (TestBit(kVertexRange)) {
          fEventTime = sign * origin0[2] / 2.99792458e10;
          tof = kconv * iparticle->T() + fEventTime;
        } else {
          tof = kconv * iparticle->T();
          fEventTime = tInt;
          if (fPileUpTimeWindow > 0.) tof += tInt;
        }
        imo = -1;
        TParticle* mother = 0;
        if (hasMother) {
          imo = iparticle->GetFirstMother();
          mother = (TParticle *) fParticles.At(imo);
          imo = (mother->GetPdgCode() != 92) ? newPos[imo] : -1;
        } // if has mother   
        Bool_t tFlag = (fTrackIt && !hasDaughter);
        PushTrack(tFlag,imo,kf,p,origin,polar,tof,kPNoProcess,nt, 1., ks);
        fNprimaries++;
        KeepTrack(nt);
        newPos[i] = nt;
      } // if selected
    } // particle loop
    delete[] newPos;
    delete[] pSelected;
      
    AliInfo(Form("\n I've put %i particles on the stack \n",nc));
    if (nc > 0) {
      jev += nc;
      if (jev >= fNpart || fNpart == -1) {
        fKineBias = Float_t(fNpart)/Float_t(fTrials);
        AliInfo(Form("\n Trials: %i %i %i\n",fTrials, fNpart, jev));
        break;
      }
    }
  } // event loop
  MakeHeader();
  SetHighWaterMark(nt);
}

void AliGenAmpt::EvaluateCrossSections()
{
  // Glauber Calculation of geometrical x-section

  Float_t xTot       = 0.;          // barn
  Float_t xTotHard   = 0.;          // barn 
  Float_t xPart      = 0.;          // barn
  Float_t xPartHard  = 0.;          // barn 
  Float_t sigmaHard  = 0.1;         // mbarn
  Float_t bMin       = 0.;
  Float_t bMax       = fAmpt->GetHIPR1(34)+fAmpt->GetHIPR1(35);
  const Float_t kdib = 0.2;
  Int_t   kMax       = Int_t((bMax-bMin)/kdib)+1;

  printf("\n Projectile Radius (fm): %f \n",fAmpt->GetHIPR1(34));
  printf("\n Target     Radius (fm): %f \n",fAmpt->GetHIPR1(35));    

  Int_t i;
  Float_t oldvalue= 0.;
  Float_t* b   = new Float_t[kMax]; memset(b,0,kMax*sizeof(Float_t));
  Float_t* si1 = new Float_t[kMax]; memset(si1,0,kMax*sizeof(Float_t));
  Float_t* si2 = new Float_t[kMax]; memset(si2,0,kMax*sizeof(Float_t));
  for (i = 0; i < kMax; i++) {
    Float_t xb  = bMin+i*kdib;
    Float_t ov=fAmpt->Profile(xb);
    Float_t gb  =  2.*0.01*fAmpt->GetHIPR1(40)*kdib*xb*(1.-TMath::Exp(-fAmpt->GetHINT1(12)*ov));
    Float_t gbh =  2.*0.01*fAmpt->GetHIPR1(40)*kdib*xb*sigmaHard*ov;
    xTot+=gb;
    xTotHard += gbh;
    printf("profile %f %f %f\n", xb, ov, fAmpt->GetHINT1(12));
	
    if (xb > fMinImpactParam && xb < fMaxImpactParam) {
      xPart += gb;
      xPartHard += gbh;
    }
	
    if ((oldvalue) && ((xTot-oldvalue)/oldvalue<0.0001)) 
      break;
    oldvalue = xTot;
    printf("\n Total cross section (barn): %d %f %f \n",i, xb, xTot);
    printf("\n Hard  cross section (barn): %d %f %f \n\n",i, xb, xTotHard);
    if (i>0) {
      si1[i] = gb/kdib;
      si2[i] = gbh/gb;
      b[i]  = xb;
    }
  }

  printf("\n Total cross section (barn): %f \n",xTot);
  printf("\n Hard  cross section (barn): %f \n \n",xTotHard);
  printf("\n Partial       cross section (barn): %f %f \n",xPart, xPart/xTot*100.);
  printf("\n Partial  hard cross section (barn): %f %f \n",xPartHard, xPartHard/xTotHard*100.);

  //  Store result as a graph
  b[0] = 0;
  si1[0] = 0;
  si2[0]=si2[1];
  delete fDsigmaDb;
  fDsigmaDb  = new TGraph(i, b, si1);
  delete fDnDb;
  fDnDb      = new TGraph(i, b, si2);
}

Bool_t AliGenAmpt::DaughtersSelection(TParticle* iparticle)
{
  // Looks recursively if one of the daughters has been selected
  //printf("\n Consider daughters %d:",iparticle->GetPdgCode());
  Int_t imin = -1;
  Int_t imax = -1;
  Bool_t hasDaughters = (iparticle->GetFirstDaughter() >=0);
  Bool_t selected = kFALSE;
  if (hasDaughters) {
    imin = iparticle->GetFirstDaughter();
    imax = iparticle->GetLastDaughter();       
    for (Int_t i = imin; i <= imax; i++){
      TParticle *  jparticle = (TParticle *) fParticles.At(i);	
      Int_t ip = jparticle->GetPdgCode();
      if (KinematicSelection(jparticle,0)&&SelectFlavor(ip)) {
        selected=kTRUE; break;
      }
      if (DaughtersSelection(jparticle)) {selected=kTRUE; break; }
    }
  } else {
    return kFALSE;
  }
  return selected;
}

Bool_t AliGenAmpt::SelectFlavor(Int_t pid)
{
  // Select flavor of particle
  // 0: all
  // 4: charm and beauty
  // 5: beauty
  Bool_t res = 0;
    
  if (fFlavor == 0) {
    res = kTRUE;
  } else {
    Int_t ifl = TMath::Abs(pid/100);
    if (ifl > 10) ifl/=10;
    res = (fFlavor == ifl);
  }

  //  This part if gamma writing is inhibited
  if (fNoGammas) 
    res = res && (pid != kGamma && pid != kPi0);

  return res;
}

Bool_t AliGenAmpt::Stable(TParticle*  particle) const
{
  // Return true for a stable particle

  if (particle->GetFirstDaughter() < 0 )
  {
    return kTRUE;
  } else {
    return kFALSE;
  }
}

void AliGenAmpt::MakeHeader()
{
  // Fills the event header, to be called after each event

  fHeader->SetNProduced(fNprimaries);
  fHeader->SetImpactParameter(fAmpt->GetHINT1(19));
  fHeader->SetTotalEnergy(fAmpt->GetEATT());
  fHeader->SetHardScatters(fAmpt->GetJATT());
  fHeader->SetParticipants(fAmpt->GetNP(), fAmpt->GetNT());
  fHeader->SetCollisions(fAmpt->GetN0(),
                        fAmpt->GetN01(),
                        fAmpt->GetN10(),
                        fAmpt->GetN11());
  fHeader->SetSpectators(fProjectileSpecn, fProjectileSpecp,
                        fTargetSpecn,fTargetSpecp);
  fHeader->SetReactionPlaneAngle(fAmpt->GetHINT1(20));
  //printf("Impact Parameter %13.3f \n", fAmpt->GetHINT1(19));

  // 4-momentum vectors of the triggered jets.
  // Before final state gluon radiation.
  TLorentzVector* jet1 = new TLorentzVector(fAmpt->GetHINT1(21), 
                                            fAmpt->GetHINT1(22),
                                            fAmpt->GetHINT1(23),
                                            fAmpt->GetHINT1(24));

  TLorentzVector* jet2 = new TLorentzVector(fAmpt->GetHINT1(31), 
                                            fAmpt->GetHINT1(32),
                                            fAmpt->GetHINT1(33),
                                            fAmpt->GetHINT1(34));
  // After final state gluon radiation.
  TLorentzVector* jet3 = new TLorentzVector(fAmpt->GetHINT1(26), 
                                            fAmpt->GetHINT1(27),
                                            fAmpt->GetHINT1(28),
                                            fAmpt->GetHINT1(29));

  TLorentzVector* jet4 = new TLorentzVector(fAmpt->GetHINT1(36), 
                                            fAmpt->GetHINT1(37),
                                            fAmpt->GetHINT1(38),
                                            fAmpt->GetHINT1(39));
  fHeader->SetJets(jet1, jet2, jet3, jet4);
  // Bookkeeping for kinematic bias
  fHeader->SetTrials(fTrials);
  // Event Vertex
  fHeader->SetPrimaryVertex(fVertex);
  fHeader->SetInteractionTime(fEventTime);

  fCollisionGeometry = fHeader;
  AddHeader(fHeader);
}


Bool_t AliGenAmpt::CheckTrigger()
{
  // Check the kinematic trigger condition

  Bool_t   triggered = kFALSE;
 
  if (fTrigger == 1) {
    // jet-jet Trigger	
    TLorentzVector* jet1 = new TLorentzVector(fAmpt->GetHINT1(26), 
                                              fAmpt->GetHINT1(27),
                                              fAmpt->GetHINT1(28),
                                              fAmpt->GetHINT1(29));
	
    TLorentzVector* jet2 = new TLorentzVector(fAmpt->GetHINT1(36), 
                                              fAmpt->GetHINT1(37),
                                              fAmpt->GetHINT1(38),
                                              fAmpt->GetHINT1(39));
    Double_t eta1      = jet1->Eta();
    Double_t eta2      = jet2->Eta();
    Double_t phi1      = jet1->Phi();
    Double_t phi2      = jet2->Phi();
    //printf("\n Trigger: %f %f %f %f", fEtaMinJet, fEtaMaxJet, fPhiMinJet, fPhiMaxJet);
    if ( (eta1 < fEtaMaxJet && eta1 > fEtaMinJet &&  
          phi1 < fPhiMaxJet && phi1 > fPhiMinJet) 
         ||
         (eta2 < fEtaMaxJet && eta2 > fEtaMinJet &&  
          phi2 < fPhiMaxJet && phi2 > fPhiMinJet)
      ) 
      triggered = kTRUE;
  } else if (fTrigger == 2) {
  // Gamma Jet
    Int_t np = fParticles.GetEntriesFast();
    for (Int_t i = 0; i < np; i++) {
      TParticle* part = (TParticle*) fParticles.At(i);
      Int_t kf = part->GetPdgCode();
      Int_t ksp = part->GetUniqueID();
      if (kf == 22 && ksp == 40) {
        Float_t phi = part->Phi();
        Float_t eta = part->Eta();
        if  (eta < fEtaMaxJet && 
             eta > fEtaMinJet &&
             phi < fPhiMaxJet && 
             phi > fPhiMinJet) {
          triggered = 1;
          break;
        } // check phi,eta within limits
      } // direct gamma ? 
    } // particle loop
  } // fTrigger == 2
  return triggered;
}
