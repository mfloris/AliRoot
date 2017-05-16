#include "ESDEventConverter.h"
#include <AliESDEvent.h>
#include <AliMCEvent.h>
#include <AliMCParticle.h>
#include <TRandom2.h>
#include <math.h>

using namespace ecs;

void ESDEventConverter::addESDEvent(double timestampNs,
                                    const AliESDEvent *event,
                                    const AliMCEvent *mcEvent) {
  TRandom2 rng;
  int numberOfTracks = event->GetNumberOfTracks();
  if (0 == numberOfTracks) {
    return;
  }
  if (mContainsMcInfo && (nullptr == mcEvent)) {
    return;
  }
  const AliESDVertex *vertex = event->GetVertex();
  mVertexX.push_back(vertex->GetX());
  mVertexY.push_back(vertex->GetY());
  mVertexZ.push_back(vertex->GetZ());
  mVertexT.push_back(timestampNs +
                     rng.Gaus(timestampNs, 100 / sqrt(numberOfTracks)));
  Double_t cov[6];
  vertex->GetCovarianceMatrix(cov);
  mVertexCovariance.push_back(cov);
  mVertexSignalToNoiseX.push_back(vertex->GetXSNR());
  mVertexSignalToNoiseY.push_back(vertex->GetYSNR());
  mVertexSignalToNoiseZ.push_back(vertex->GetZSNR());
  mVertexChiSquared.push_back(vertex->GetChi2());
  mVertexSigma.push_back(vertex->GetDispersion());
  int nIndices = vertex->GetNIndices();
  // TODO: verify
  // How many tracks were already added before this event.
  size_t trackIndexOffset = mTrackX.size();
  UShort_t *indices = vertex->GetIndices();
  for (int i = 0; i < nIndices; i++) {
    mVertexUsedTracksIndices.push_back(indices[i] + trackIndexOffset);
  }
  mVertexUsedTracksIndicesMapping.push_back(mVertexUsedTracksIndicesOffset);
  mVertexUsedTracksIndicesMapping.push_back(nIndices);
  mVertexUsedTracksIndicesOffset += nIndices;

  mVertexESDEventMapping.push_back(
      ecs::vertex::ESDEventMapping(mTrackX.size(), numberOfTracks));

  for (int i = 0; i < numberOfTracks; i++) {
    AliESDtrack *esdTrack = event->GetTrack(i);

    mTrackX.push_back(esdTrack->GetX());
    mTrackY.push_back(esdTrack->GetY());
    mTrackZ.push_back(esdTrack->GetZ());
    mTrackT.push_back(timestampNs + rng.Gaus(timestampNs, 100));
    Double_t cov[15];
    esdTrack->GetExternalCovariance(cov);
    mTrackCovariance.push_back(cov);
    mTrackPx.push_back(esdTrack->Px());
    mTrackPy.push_back(esdTrack->Py());
    mTrackPz.push_back(esdTrack->Pz());
    mTrackMass.push_back(esdTrack->M());
    mTrackChargeSign.push_back(esdTrack->Charge() > 0);
    /// Documentation? sign used as flag, absolute value corresponds actual
    /// index. What is the value if there is no mapping for this track?
    if (mContainsMcInfo) {
      Int_t label = esdTrack->GetLabel();
      mTrackMonteCarloIndex.push_back(abs(label) + mParticleX.size());
      mAmbiguousClustersFlag.push_back(label >= 0);
    }
  }
  if (mContainsMcInfo) {
    int numberOfParticles = mcEvent->GetNumberOfTracks();
    for (int i = 0; i < numberOfParticles; i++) {
      const AliMCParticle *mcParticle =
          (const AliMCParticle *)mcEvent->GetTrack(i);

      mParticleX.push_back(mcParticle->Xv());
      mParticleY.push_back(mcParticle->Yv());
      mParticleZ.push_back(mcParticle->Zv());
      mParticleT.push_back(timestampNs); // ignoring secondaries and
                                         // lifetimes.
      mParticlePx.push_back(mcParticle->Px());
      mParticlePy.push_back(mcParticle->Py());
      mParticlePz.push_back(mcParticle->Pz());
      mParticlePdgCode.push_back(mcParticle->PdgCode());
    }
  }
  mEventCounter++;
}

/// Takes the stored data and sends it to a Handler which then outputs the
/// data to a file. No copies are involved except for N->M Mappings in order
/// to fit the data in a contigous buffer.
void ESDEventConverter::toFile(const std::string &filename) {
  Handler h;

  size_t vertexCount = mVertexX.size();
  h.forceInsertEntity<Vertex_t>(vertexCount);
  h.forceInsertComponentData<Vertex_t, vertex::X>(mVertexX.data());
  h.forceInsertComponentData<Vertex_t, vertex::Y>(mVertexY.data());
  h.forceInsertComponentData<Vertex_t, vertex::Z>(mVertexZ.data());
  h.forceInsertComponentData<Vertex_t, vertex::T>(mVertexT.data());
  h.forceInsertComponentData<Vertex_t, vertex::Covariance>(
      mVertexCovariance.data());
  h.forceInsertComponentData<Vertex_t, vertex::ESDEventMapping>(
      mVertexESDEventMapping.data());
  h.forceInsertComponentData<Vertex_t, vertex::SignalToNoiseX>(
      mVertexSignalToNoiseX.data());
  h.forceInsertComponentData<Vertex_t, vertex::SignalToNoiseY>(
      mVertexSignalToNoiseY.data());
  h.forceInsertComponentData<Vertex_t, vertex::SignalToNoiseZ>(
      mVertexSignalToNoiseZ.data());
  h.forceInsertComponentData<Vertex_t, vertex::ChiSquared>(
      mVertexChiSquared.data());
  h.forceInsertComponentData<Vertex_t, vertex::Sigma>(mVertexSigma.data());
  // TODO: insert this N->M data
  h.forceInsertComponentData<Vertex_t, vertex::UsedTracksIndices>(
      mVertexUsedTracksIndicesMapping.data(), mVertexUsedTracksIndices.size(),
      mVertexUsedTracksIndices.data());

  size_t trackCount = mTrackX.size();

  h.forceInsertEntity<Track_t>(trackCount);

  h.forceInsertComponentData<Track_t, track::X>(mTrackX.data());
  h.forceInsertComponentData<Track_t, track::Y>(mTrackY.data());
  h.forceInsertComponentData<Track_t, track::Z>(mTrackZ.data());
  h.forceInsertComponentData<Track_t, track::T>(mTrackT.data());
  h.forceInsertComponentData<Track_t, track::Px>(mTrackPx.data());
  h.forceInsertComponentData<Track_t, track::Py>(mTrackPy.data());
  h.forceInsertComponentData<Track_t, track::Pz>(mTrackPz.data());

  h.forceInsertComponentData<Track_t, track::Covariance>(
      mTrackCovariance.data());
  h.forceInsertComponentData<Track_t, track::Mass>(mTrackMass.data());
  h.forceInsertComponentData<Track_t, track::ChargeSign>(
      mTrackChargeSign.data());

  // h.forceInsertComponentData<Track_t, track::ESDEventIndex>(
  //     mTrackEventIndex.data());

  size_t trackCountMc = mParticleX.size();
  if (trackCountMc) {

    h.forceInsertComponentData<Track_t, track::mc::MonteCarloIndex>(
        mTrackMonteCarloIndex.data());
    h.forceInsertComponentData<Track_t, track::mc::AmbiguousClustersFlag>(
        mAmbiguousClustersFlag.data());

    h.forceInsertEntity<Particle_t>(trackCountMc);
    h.forceInsertComponentData<Particle_t, particle::X>(mParticleX.data());
    h.forceInsertComponentData<Particle_t, particle::Y>(mParticleY.data());
    h.forceInsertComponentData<Particle_t, particle::Z>(mParticleZ.data());
    h.forceInsertComponentData<Particle_t, particle::T>(mParticleT.data());
    h.forceInsertComponentData<Particle_t, particle::Px>(mParticlePx.data());
    h.forceInsertComponentData<Particle_t, particle::Py>(mParticlePy.data());
    h.forceInsertComponentData<Particle_t, particle::Pz>(mParticlePz.data());
    h.forceInsertComponentData<Particle_t, particle::PdgCode>(
        mParticlePdgCode.data());
  }
  // h.forceInsertComponentData<Track_t, TrackToClusterMapping>(
  //     mapTable.data(), map.size(), map.data());
  // and we tell h to serialize this data to the the file "buffer.bin"
  h.toFile(filename);
}
