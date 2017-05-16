#ifndef ALI_O2_WRAPPER_ESD_EVENT_CONVERTER_H
#define ALI_O2_WRAPPER_ESD_EVENT_CONVERTER_H

#include "Entities/Particle.h"
#include "Entities/Track.h"
#include "Entities/Vertex.h"
#include <vector>

class AliESDEvent;
class AliMCEvent;
class ESDEventConverter {
  bool mContainsMcInfo = false;
  unsigned mEventCounter = 0;
  using Track_t = ecs::Track<>;
  using Vertex_t = ecs::Vertex<>;
  using Particle_t = ecs::Particle<>;

  std::vector<ecs::vertex::X> mVertexX;
  std::vector<ecs::vertex::Y> mVertexY;
  std::vector<ecs::vertex::Z> mVertexZ;
  std::vector<ecs::vertex::T> mVertexT;
  std::vector<ecs::vertex::Covariance> mVertexCovariance;
  std::vector<ecs::vertex::ESDEventMapping> mVertexESDEventMapping;
  std::vector<ecs::vertex::SignalToNoiseX> mVertexSignalToNoiseX;
  std::vector<ecs::vertex::SignalToNoiseY> mVertexSignalToNoiseY;
  std::vector<ecs::vertex::SignalToNoiseZ> mVertexSignalToNoiseZ;
  std::vector<ecs::vertex::ChiSquared> mVertexChiSquared;
  std::vector<ecs::vertex::Sigma> mVertexSigma;
  // TODO: wrap this in a class?
  std::vector<ecs::vertex::UsedTracksIndices> mVertexUsedTracksIndices;
  std::vector<uint64_t> mVertexUsedTracksIndicesMapping;
  size_t mVertexUsedTracksIndicesOffset = 0;

  // std::vector<ecs::track::ESDEventIndex> mTrackEventIndex;
  std::vector<ecs::track::X> mTrackX;
  std::vector<ecs::track::Y> mTrackY;
  std::vector<ecs::track::Z> mTrackZ;
  std::vector<ecs::track::T> mTrackT;
  std::vector<ecs::track::Covariance> mTrackCovariance;
  std::vector<ecs::track::Px> mTrackPx;
  std::vector<ecs::track::Py> mTrackPy;
  std::vector<ecs::track::Pz> mTrackPz;
  std::vector<ecs::track::ChargeSign> mTrackChargeSign;
  std::vector<ecs::track::Mass> mTrackMass;
  // Only makes sense for MCTracks
  std::vector<ecs::track::mc::MonteCarloIndex> mTrackMonteCarloIndex;
  std::vector<ecs::track::mc::AmbiguousClustersFlag> mAmbiguousClustersFlag;

  // std::vector<ecs::track::ESDEventIndex> mTrackEventIndexMc;
  std::vector<ecs::particle::X> mParticleX;
  std::vector<ecs::particle::Y> mParticleY;
  std::vector<ecs::particle::Z> mParticleZ;
  std::vector<ecs::particle::T> mParticleT;
  std::vector<ecs::particle::Px> mParticlePx;
  std::vector<ecs::particle::Py> mParticlePy;
  std::vector<ecs::particle::Pz> mParticlePz;
  // std::vector<ecs::Particle::Pt> mTrackPtMc;
  std::vector<ecs::particle::PdgCode> mParticlePdgCode;

public:
  void useMcInfo(bool setTo) { mContainsMcInfo = setTo; }
  /// Reads data from an AliESDEvent and copies it into it's own buffers. Uses
  /// the given timestamp as a 'real' event time to generate simulated detection
  /// times.
  void addESDEvent(double timestampNs, const AliESDEvent *event,
                   const AliMCEvent *mcEvent = nullptr);
  /// Takes the stored data and sends it to a Handler which then outputs the
  /// data to a file. No copies are involved except for N->M Mappings in order
  /// to fit the data in a contigous buffer.
  void toFile(const std::string &filename);
};
#endif
