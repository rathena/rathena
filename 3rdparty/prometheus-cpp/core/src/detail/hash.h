#pragma once

#include <cstddef>
#include <functional>

namespace prometheus {

namespace detail {

/// \brief Combine a hash value with nothing.
/// It's the boundary condition of this serial functions.
///
/// \param seed Not effect.
inline void hash_combine(std::size_t*) {}

/// \brief Combine the given hash value with another object.
///
/// \param seed The given hash value. It's a input/output parameter.
/// \param value The object that will be combined with the given hash value.
template <typename T>
inline void hash_combine(std::size_t* seed, const T& value) {
  *seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (*seed << 6) + (*seed >> 2);
}

/// \brief Combine the given hash value with another objects. It's a recursion。
///
/// \param seed The give hash value. It's a input/output parameter.
/// \param value The object that will be combined with the given hash value.
/// \param args The objects that will be combined with the given hash value.
template <typename T, typename... Types>
inline void hash_combine(std::size_t* seed, const T& value,
                         const Types&... args) {
  hash_combine(seed, value);
  hash_combine(seed, args...);
}

}  // namespace detail

}  // namespace prometheus
