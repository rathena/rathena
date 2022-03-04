#include "c4/test.hpp"
#ifndef C4CORE_SINGLE_HEADER
#include "c4/std/string.hpp"
#include "c4/std/vector.hpp"
#include "c4/format.hpp"
#include "c4/utf.hpp"
#endif

#include "c4/libtest/supprwarn_push.hpp"

#include <cstring>

namespace c4 {

struct utft
{
    csubstr code_point;
    csubstr character;
    uint32_t character_val;
    csubstr character_val_hex;
};
constexpr const utft utf_chars[] = {
#include "./utfchars.inc"
};

TEST_CASE("utf.decode_code_point")
{
    size_t i = 0;
    char decoded_buf[64];
    for(auto uc : utf_chars)
    {
        INFO("utfchars[", i, "]: codepoint=", uc.code_point, ' ',
             "character=", uc.character.empty() ? csubstr{} : uc.character, ' ',
             "val=", uc.character_val_hex, '(', uc.character_val, ')');
        i++;
        csubstr cpstr = uc.code_point.sub(2).triml('0');
        csubstr decoded = decode_code_point(decoded_buf, cpstr);
        CHECK_UNARY(uc.code_point.begins_with("U+"));
        if(uc.character.empty())
            continue;
        CHECK_EQ(decoded.len, uc.character.len);
        CHECK_EQ(decoded, uc.character);
    }
}

} // namespace c4
