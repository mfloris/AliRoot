#ifndef ALI_O2_ENITIES_PARTICLE_H
#define ALI_O2_ENITIES_PARTICLE_H
#include <math.h>
#include "../Components/Particle.h"
#include "../ecs/Entity.h"

namespace ecs {
template <typename... Components>
class Particle : public Entity<Components...> {
public:
  static const char *Id() { return "Particle"; }
  using ecs::Entity<Components...>::Entity;
  template <typename U = Particle<Components...>,
            typename std::enable_if<U::template Contains<
                particle::X, particle::Y, particle::Z>()>::type * = nullptr>
  float length_xyz() const {
    auto x = this->template get<particle::X>();
    auto y = this->template get<particle::Y>();
    auto z = this->template get<particle::Z>();
    return sqrtf(x * x + y * y + z * z);
  }
  template <typename U = Particle<Components...>,
            typename std::enable_if<U::template Contains<
                particle::Px, particle::Py>()>::type * = nullptr>
  float pt() const {
    float px = this->template get<particle::Px>();
    float py = this->template get<particle::Py>();
    return sqrt(px * px + py * py);
  }
};
}

#endif
