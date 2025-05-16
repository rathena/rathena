#include "c4/yml/node.hpp"

namespace c4 {
namespace yml {

size_t NodeRef::set_key_serialized(c4::fmt::const_base64_wrapper w)
{
    _apply_seed();
    csubstr encoded = this->to_arena(w);
    this->set_key(encoded);
    return encoded.len;
}

size_t NodeRef::set_val_serialized(c4::fmt::const_base64_wrapper w)
{
    _apply_seed();
    csubstr encoded = this->to_arena(w);
    this->set_val(encoded);
    return encoded.len;
}

size_t NodeRef::deserialize_key(c4::fmt::base64_wrapper w) const
{
    RYML_ASSERT( ! is_seed());
    RYML_ASSERT(valid());
    RYML_ASSERT(get() != nullptr);
    return from_chars(key(), &w);
}

size_t NodeRef::deserialize_val(c4::fmt::base64_wrapper w) const
{
    RYML_ASSERT( ! is_seed());
    RYML_ASSERT(valid());
    RYML_ASSERT(get() != nullptr);
    return from_chars(val(), &w);
}

} // namespace yml
} // namespace c4
