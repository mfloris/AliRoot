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

/* $Id: AliEMCALRecoUtils.cxx 33808 2009-07-15 09:48:08Z gconesab $ */

///////////////////////////////////////////////////////////////////////////////
//
// Class AliEMCALRecoUtils
// Some utilities to recalculate the cluster position or energy linearity
//
//
// Author:  Gustavo Conesa (LPSC- Grenoble) 
///////////////////////////////////////////////////////////////////////////////

// --- standard c ---

// standard C++ includes
//#include <Riostream.h>

// ROOT includes
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoBBox.h>

// STEER includes
#include "AliEMCALRecoUtils.h"
#include "AliEMCALGeometry.h"
#include "AliVCluster.h"
#include "AliVCaloCells.h"
#include "AliLog.h"

ClassImp(AliEMCALRecoUtils)
  
//______________________________________________
AliEMCALRecoUtils::AliEMCALRecoUtils():
  fNonLinearityFunction (kPi0GammaGamma), fParticleType(kPhoton),
  fPosAlgo(kPosTowerIndex),fW0(4.),fRecalibration(kFALSE), fEMCALRecalibrationFactors()
{
//
  // Constructor.
  // Initialize all constant values which have to be used
  // during Reco algorithm execution
  //
  
  for(Int_t i = 0; i < 15 ; i++) {fMisalTransShift[i] = 0.; fMisalRotShift[i] = 0.; }
  for(Int_t i = 0; i < 6  ; i++) fNonLinearityParams[i] = 0.; 
  //By default kPi0GammaGamma case
  fNonLinearityParams[0] = 0.1457/0.1349766/1.038;
  fNonLinearityParams[1] = -0.02024/0.1349766/1.038;
  fNonLinearityParams[2] = 1.046;
  
}

//______________________________________________________________________
AliEMCALRecoUtils::AliEMCALRecoUtils(const AliEMCALRecoUtils & reco) 
: TNamed(reco), fNonLinearityFunction(reco.fNonLinearityFunction), 
  fParticleType(reco.fParticleType), fPosAlgo(reco.fPosAlgo), 
  fW0(reco.fW0), fRecalibration(reco.fRecalibration),fEMCALRecalibrationFactors(reco.fEMCALRecalibrationFactors)
{
  //Copy ctor
  
  for(Int_t i = 0; i < 15 ; i++) {fMisalRotShift[i] = reco.fMisalRotShift[i]; fMisalTransShift[i] = reco.fMisalTransShift[i]; } 
  for(Int_t i = 0; i < 6  ; i++) fNonLinearityParams[i] = reco.fNonLinearityParams[i]; 
}


//______________________________________________________________________
AliEMCALRecoUtils & AliEMCALRecoUtils::operator = (const AliEMCALRecoUtils & reco) 
{
  //Assignment operator
  
  if(this == &reco)return *this;
  ((TNamed *)this)->operator=(reco);

  fNonLinearityFunction = reco.fNonLinearityFunction;
  fParticleType         = reco.fParticleType;
  fPosAlgo              = reco.fPosAlgo; 
  fW0                   = reco.fW0;
  fRecalibration        = reco.fRecalibration;
  fEMCALRecalibrationFactors = reco.fEMCALRecalibrationFactors;
  
  for(Int_t i = 0; i < 15 ; i++) {fMisalTransShift[i] = reco.fMisalTransShift[i]; fMisalRotShift[i] = reco.fMisalRotShift[i];}
  for(Int_t i = 0; i < 6  ; i++) fNonLinearityParams[i] = reco.fNonLinearityParams[i]; 
  
  return *this;
}


//__________________________________________________
AliEMCALRecoUtils::~AliEMCALRecoUtils()
{
  //Destructor.
	
	if(fEMCALRecalibrationFactors) { 
		fEMCALRecalibrationFactors->Clear();
		delete  fEMCALRecalibrationFactors;
	}	
}


//__________________________________________________
Float_t AliEMCALRecoUtils::CorrectClusterEnergyLinearity(AliVCluster* cluster){
// Correct cluster energy from non linearity functions
  Float_t energy = cluster->E();
  
  switch (fNonLinearityFunction) {
      
    case kPi0MC:
      //Non-Linearity correction (from MC with function ([0]*exp(-[1]/E))+(([2]/([3]*2.*TMath::Pi())*exp(-(E-[4])^2/(2.*[3]^2)))))
      //Double_t par0 = 1.001;
      //Double_t par1 = -0.01264;
      //Double_t par2 = -0.03632;
      //Double_t par3 = 0.1798;
      //Double_t par4 = -0.522;
       energy /= (fNonLinearityParams[0]*exp(-fNonLinearityParams[1]/energy))+
                  ((fNonLinearityParams[2]/(fNonLinearityParams[3]*2.*TMath::Pi())*
                    exp(-(energy-fNonLinearityParams[4])*(energy-fNonLinearityParams[4])/(2.*fNonLinearityParams[3]*fNonLinearityParams[3]))));
      break;
      
    case kPi0GammaGamma:

      //Non-Linearity correction (from Olga Data with function p0+p1*exp(-p2*E))
      //Double_t par0 = 0.1457;
      //Double_t par1 = -0.02024;
      //Double_t par2 = 1.046;
      energy /= (fNonLinearityParams[0]+fNonLinearityParams[1]*exp(-fNonLinearityParams[2]*energy)); //Olga function
      break;
      
    case kPi0GammaConversion:
      
      //Non-Linearity correction (Nicolas from Dimitri Data with function C*[1-a*exp(-b*E)])
      //Double_t C = 0.139393/0.1349766;
      //Double_t a = 0.0566186;
      //Double_t b = 0.982133;
      energy /= fNonLinearityParams[0]*(1-fNonLinearityParams[1]*exp(-fNonLinearityParams[2]*energy));
      
      break;
      
    case kNoCorrection:
      AliDebug(2,"No correction on the energy\n");
      break;
      
  }
  
  return energy;

}

//__________________________________________________
Float_t  AliEMCALRecoUtils::GetDepth(const Float_t energy, const Int_t iParticle, const Int_t iSM) const 
{
  //Calculate shower depth for a given cluster energy and particle type

  // parameters 
  Float_t x0    = 1.23;
  Float_t ecr   = 8;
  Float_t depth = 0;
  
  switch ( iParticle )
  {
    case kPhoton:
      depth = x0 * TMath::Log( (energy*1000/ ecr) + 0.5); //Multiply energy by 1000 to transform to MeV
      break;
      
    case kElectron:
      depth = x0 * TMath::Log( (energy*1000/ ecr) - 0.5); //Multiply energy by 1000 to transform to MeV
      break;
      
    case kHadron:
      // hadron 
      // boxes anc. here
      if(gGeoManager){
        gGeoManager->cd("ALIC_1/XEN1_1");
        TGeoNode        *geoXEn1    = gGeoManager->GetCurrentNode();
        TGeoNodeMatrix  *geoSM      = dynamic_cast<TGeoNodeMatrix *>(geoXEn1->GetDaughter(iSM));
        TGeoVolume      *geoSMVol   = geoSM->GetVolume(); 
        TGeoShape       *geoSMShape = geoSMVol->GetShape();
        TGeoBBox        *geoBox     = dynamic_cast<TGeoBBox *>(geoSMShape);
        Float_t l = geoBox->GetDX()*2 ;
        depth = 0.5 * l;
      }
      else{//electron
        depth = x0 * TMath::Log( (energy*1000 / ecr)  - 0.5); //Multiply energy by 1000 to transform to MeV
      }
        
      break;
      
    default://photon
      depth = x0 * TMath::Log( (energy*1000 / ecr) + 0.5); //Multiply energy by 1000 to transform to MeV
  }  
  
  return depth;
  
}

//__________________________________________________
void AliEMCALRecoUtils::GetMaxEnergyCell(AliEMCALGeometry *geom, AliVCaloCells* cells, AliVCluster* clu, Int_t & absId,  Int_t& iSupMod, Int_t& ieta, Int_t& iphi)
{
  //For a given CaloCluster gets the absId of the cell 
  //with maximum energy deposit.
  
  Double_t eMax        = -1.;
  Double_t eCell       = -1.;
  Float_t  fraction    = 1.;
  Float_t  recalFactor = 1.;
  Int_t    cellAbsId   = -1 ;

  Int_t iTower  = -1;
  Int_t iIphi   = -1;
  Int_t iIeta   = -1;
	
  for (Int_t iDig=0; iDig< clu->GetNCells(); iDig++) {
    cellAbsId = clu->GetCellAbsId(iDig);
    fraction  = clu->GetCellAmplitudeFraction(iDig);
    if(fraction < 1e-4) fraction = 1.; // in case unfolding is off
    if(IsRecalibrationOn()) {
      geom->GetCellIndex(cellAbsId,iSupMod,iTower,iIphi,iIeta); 
      geom->GetCellPhiEtaIndexInSModule(iSupMod,iTower,iIphi, iIeta,iphi,ieta);
      recalFactor = GetEMCALChannelRecalibrationFactor(iSupMod,ieta,iphi);
    }
    eCell  = cells->GetCellAmplitude(cellAbsId)*fraction*recalFactor;
    
    if(eCell > eMax)  { 
      eMax  = eCell; 
      absId = cellAbsId;
      //printf("\t new max: cell %d, e %f, ecell %f\n",maxId, eMax,eCell);
    }
  }// cell loop
  
  //Get from the absid the supermodule, tower and eta/phi numbers
  geom->GetCellIndex(absId,iSupMod,iTower,iIphi,iIeta); 
  //Gives SuperModule and Tower numbers
  geom->GetCellPhiEtaIndexInSModule(iSupMod,iTower,
                                         iIphi, iIeta,iphi,ieta);    
  
}

//________________________________________________________________
void AliEMCALRecoUtils::InitEMCALRecalibrationFactors(){
	//Init EMCAL recalibration factors
	AliDebug(2,"AliCalorimeterUtils::InitEMCALRecalibrationFactors()");
	//In order to avoid rewriting the same histograms
	Bool_t oldStatus = TH1::AddDirectoryStatus();
	TH1::AddDirectory(kFALSE);
  
	fEMCALRecalibrationFactors = new TObjArray(12);
	for (int i = 0; i < 12; i++) fEMCALRecalibrationFactors->Add(new TH2F(Form("EMCALRecalFactors_SM%d",i),Form("EMCALRecalFactors_SM%d",i),  48, 0, 48, 24, 0, 24));
	//Init the histograms with 1
	for (Int_t sm = 0; sm < 12; sm++) {
		for (Int_t i = 0; i < 48; i++) {
			for (Int_t j = 0; j < 24; j++) {
				SetEMCALChannelRecalibrationFactor(sm,i,j,1.);
			}
		}
	}
	fEMCALRecalibrationFactors->SetOwner(kTRUE);
	fEMCALRecalibrationFactors->Compress();
	
	//In order to avoid rewriting the same histograms
	TH1::AddDirectory(oldStatus);		
}


//________________________________________________________________
void AliEMCALRecoUtils::RecalibrateClusterEnergy(AliEMCALGeometry* geom, AliVCluster * cluster, AliVCaloCells * cells){
	// Recalibrate the cluster energy, considering the recalibration map and the energy of the cells that compose the cluster.
	
	//Get the cluster number of cells and list of absId, check what kind of cluster do we have.
	UShort_t * index    = cluster->GetCellsAbsId() ;
	Double_t * fraction = cluster->GetCellsAmplitudeFraction() ;
	Int_t ncells = cluster->GetNCells();
	
	//Initialize some used variables
	Float_t energy = 0;
	Int_t absId    = -1;
  Int_t icol = -1, irow = -1, imod=1;
	Float_t factor = 1, frac = 0;
	
	//Loop on the cells, get the cell amplitude and recalibration factor, multiply and and to the new energy
	for(Int_t icell = 0; icell < ncells; icell++){
		absId = index[icell];
		frac =  fraction[icell];
		if(frac < 1e-5) frac = 1; //in case of EMCAL, this is set as 0 since unfolding is off
		Int_t iTower = -1, iIphi = -1, iIeta = -1; 
		geom->GetCellIndex(absId,imod,iTower,iIphi,iIeta); 
		if(fEMCALRecalibrationFactors->GetEntries() <= imod) continue;
		geom->GetCellPhiEtaIndexInSModule(imod,iTower,iIphi, iIeta,irow,icol);			
		factor = GetEMCALChannelRecalibrationFactor(imod,icol,irow);
    AliDebug(2,Form("AliEMCALRecoUtils::RecalibrateClusterEnergy - recalibrate cell: module %d, col %d, row %d, cell fraction %f,recalibration factor %f, cell energy %f\n",
             imod,icol,irow,frac,factor,cells->GetCellAmplitude(absId)));
		
		energy += cells->GetCellAmplitude(absId)*factor*frac;
	}
	
	
		AliDebug(2,Form("AliEMCALRecoUtils::RecalibrateClusterEnergy - Energy before %f, after %f\n",cluster->E(),energy));
	
	cluster->SetE(energy);
	
}


//__________________________________________________
void AliEMCALRecoUtils::RecalculateClusterPosition(AliEMCALGeometry *geom, AliVCaloCells* cells, AliVCluster* clu)
{
  //For a given CaloCluster recalculates the position for a given set of misalignment shifts and puts it again in the CaloCluster.
  
  if     (fPosAlgo==kPosTowerGlobal) RecalculateClusterPositionFromTowerGlobal( geom, cells, clu);
  else if(fPosAlgo==kPosTowerIndex)  RecalculateClusterPositionFromTowerIndex ( geom, cells, clu);
  
}  

//__________________________________________________
void AliEMCALRecoUtils::RecalculateClusterPositionFromTowerGlobal(AliEMCALGeometry *geom, AliVCaloCells* cells, AliVCluster* clu)
{
  // For a given CaloCluster recalculates the position for a given set of misalignment shifts and puts it again in the CaloCluster.
  // The algorithm is a copy of what is done in AliEMCALRecPoint
  
  Double_t eCell       = 0.;
  Float_t  fraction    = 1.;
  Float_t  recalFactor = 1.;
  
  Int_t    absId   = -1;
  Int_t    iTower  = -1, iIphi  = -1, iIeta  = -1;
  Int_t    iSupModMax = -1, iSM=-1, iphi   = -1, ieta   = -1;
  Float_t  weight = 0.,  totalWeight=0.;
  Float_t  newPos[3] = {0,0,0};
  Double_t pLocal[3], pGlobal[3];
  
  Float_t  clEnergy = clu->E(); //Energy already recalibrated previously
  
  GetMaxEnergyCell(geom, cells, clu, absId,  iSupModMax, ieta, iphi);
  Double_t depth = GetDepth(clEnergy,fParticleType,iSupModMax) ;
  
  for (Int_t iDig=0; iDig< clu->GetNCells(); iDig++) {
    absId = clu->GetCellAbsId(iDig);
    fraction  = clu->GetCellAmplitudeFraction(iDig);
    if(fraction < 1e-4) fraction = 1.; // in case unfolding is off
    geom->GetCellIndex(absId,iSM,iTower,iIphi,iIeta); 
    geom->GetCellPhiEtaIndexInSModule(iSM,iTower,iIphi, iIeta,iphi,ieta);			
    
    if(IsRecalibrationOn()) {
      recalFactor = GetEMCALChannelRecalibrationFactor(iSM,ieta,iphi);
    }
    eCell  = cells->GetCellAmplitude(absId)*fraction*recalFactor;
    
    weight = GetCellWeight(eCell,clEnergy);
    totalWeight += weight;
    geom->RelPosCellInSModule(absId,depth,pLocal[0],pLocal[1],pLocal[2]);
    geom->GetGlobal(pLocal,pGlobal,iSupModMax);
    
    for(int i=0; i<3; i++ ) newPos[i] += (weight*pGlobal[i]);
    
  }// cell loop
  
  if(totalWeight>0){
    for(int i=0; i<3; i++ )    newPos[i] /= totalWeight;
  }
    
  //printf("iSM %d \n",iSupMod);
  //Float_t pos[]={0,0,0};
  //clu->GetPosition(pos);
  //printf("OldPos  : %2.3f,%2.3f,%2.3f\n",pos[0],pos[1],pos[2]);
  
  
  //printf("NewPos a: %2.3f,%2.3f,%2.3f\n",newPos[0],newPos[1],newPos[2]);
  
	if(iSupModMax > 1) {//sector 1
	  newPos[0] +=fMisalTransShift[3];//-=3.093; 
	  newPos[1] +=fMisalTransShift[4];//+=6.82;
	  newPos[2] +=fMisalTransShift[5];//+=1.635;
	}
	else {//sector 0
	  newPos[0] +=fMisalTransShift[0];//+=1.134;
	  newPos[1] +=fMisalTransShift[1];//+=8.2;
	  newPos[2] +=fMisalTransShift[2];//+=1.197;
	}
  
  clu->SetPosition(newPos);
  
  
  
}  

//__________________________________________________
void AliEMCALRecoUtils::RecalculateClusterPositionFromTowerIndex(AliEMCALGeometry *geom, AliVCaloCells* cells, AliVCluster* clu)
{
  // For a given CaloCluster recalculates the position for a given set of misalignment shifts and puts it again in the CaloCluster.
  // The algorithm works with the tower indeces, averages the indeces and from them it calculates the global position
  
  Double_t eCell       = 1.;
  Float_t  fraction    = 1.;
  Float_t  recalFactor = 1.;
  
  Int_t absId   = -1;
  Int_t iTower  = -1;
  Int_t iIphi   = -1, iIeta   = -1;
	Int_t iSupMod = -1, iSupModMax = -1;
  Int_t iphi = -1, ieta =-1;
  
  Float_t clEnergy = clu->E(); //Energy already recalibrated previously.
  GetMaxEnergyCell(geom, cells, clu, absId,  iSupModMax, ieta, iphi);
  Float_t  depth = GetDepth(clEnergy,fParticleType,iSupMod) ;

  Float_t weight = 0., weightedCol = 0., weightedRow = 0., totalWeight=0.;
  Bool_t areInSameSM = kTRUE; //exclude clusters with cells in different SMs for now
  Int_t startingSM = -1;
  
  for (Int_t iDig=0; iDig< clu->GetNCells(); iDig++) {
    absId = clu->GetCellAbsId(iDig);
    fraction  = clu->GetCellAmplitudeFraction(iDig);
    if(fraction < 1e-4) fraction = 1.; // in case unfolding is off
    geom->GetCellIndex(absId,iSupMod,iTower,iIphi,iIeta); 
    geom->GetCellPhiEtaIndexInSModule(iSupMod,iTower,iIphi, iIeta,iphi,ieta);			
    
    if     (iDig==0)  startingSM = iSupMod;
    else if(iSupMod != startingSM) areInSameSM = kFALSE;

    eCell  = cells->GetCellAmplitude(absId);
    
    if(IsRecalibrationOn()) {
      recalFactor = GetEMCALChannelRecalibrationFactor(iSupMod,ieta,iphi);
    }
    eCell  = cells->GetCellAmplitude(absId)*fraction*recalFactor;
    
    weight = GetCellWeight(eCell,clEnergy);
    if(weight < 0) weight = 0;
    totalWeight += weight;
    weightedCol += ieta*weight;
    weightedRow += iphi*weight;
    
    //printf("Max cell? cell %d, amplitude org %f, fraction %f, recalibration %f, amplitude new %f \n",cellAbsId, cells->GetCellAmplitude(cellAbsId), fraction, recalFactor, eCell) ;
    
    }// cell loop
    
  Float_t xyzNew[]={0.,0.,0.};
  if(areInSameSM == kTRUE) {
    //printf("In Same SM\n");
    weightedCol = weightedCol/totalWeight;
    weightedRow = weightedRow/totalWeight;
    geom->RecalculateTowerPosition(weightedRow, weightedCol, iSupModMax, depth, fMisalTransShift, fMisalRotShift, xyzNew); 
  }
  else {
    //printf("In Different SM\n");
    geom->RecalculateTowerPosition(iphi,        ieta,        iSupModMax, depth, fMisalTransShift, fMisalRotShift, xyzNew); 
  }
  
  clu->SetPosition(xyzNew);
  
}

//__________________________________________________
void AliEMCALRecoUtils::Print(const Option_t *) const 
{
  // Print Parameters
  
  printf("AliEMCALRecoUtils Settings: \n");
  printf("Misalignment shifts\n");
  for(Int_t i=0; i<5; i++) printf("\t sector %d, traslation (x,y,z)=(%f,%f,%f), rotation (x,y,z)=(%f,%f,%f)\n",i, 
                                  fMisalTransShift[i*3],fMisalTransShift[i*3+1],fMisalTransShift[i*3+2],
                                  fMisalRotShift[i*3],  fMisalRotShift[i*3+1],  fMisalRotShift[i*3+2]   );
  printf("Non linearity function %d, parameters:\n", fNonLinearityFunction);
  for(Int_t i=0; i<6; i++) printf("param[%d]=%f\n",i, fNonLinearityParams[i]);
  
  printf("Position Recalculation option %d, Particle Type %d, fW0 %2.2f, Recalibrate Data %d \n",fPosAlgo,fParticleType,fW0, fRecalibration);
    
}
