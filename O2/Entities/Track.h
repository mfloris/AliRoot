#ifndef ALI_O2_ENITIES_TRACK_H
#define ALI_O2_ENITIES_TRACK_H
#include "../Components/Track.h"
#include "../ecs/Entity.h"
#include <math.h>

namespace ecs {
template <typename... Components> class Track : public Entity<Components...> {
public:
  static const char *Id() { return "Track"; }
  using ecs::Entity<Components...>::Entity;
  template <typename U = Track<Components...>,
            typename std::enable_if<U::template Contains<
                track::X, track::Y, track::Z>()>::type * = nullptr>
  float length_xyz() const {
    auto x = this->template get<track::X>();
    auto y = this->template get<track::Y>();
    auto z = this->template get<track::Z>();
    return sqrtf(x * x + y * y + z * z);
  }
  template <typename U = Track<Components...>,
            typename std::enable_if<
                U::template Contains<track::Px, track::Py>()>::type * = nullptr>
  float pt() const {
    float px = this->template get<track::Px>();
    float py = this->template get<track::Py>();
    return sqrt(px * px + py * py);
  }
  template <typename U = Track<Components...>,
            typename std::enable_if<U::template Contains<
                track::mc::MonteCarloIndex>()>::type * = nullptr>
  int mcLabel() const {
    return this->template get<track::mc::MonteCarloIndex>();
  }
  template <typename U = Track<Components...>,
            typename std::enable_if<U::template Contains<
                track::mc::AmbiguousClustersFlag>()>::type * = nullptr>
  bool containsAmbigousClusters() const {
    return this->template get<track::mc::AmbiguousClustersFlag>();
  }
};
}

#endif
