#ifndef _C4_YML_NODE_HPP_
#define _C4_YML_NODE_HPP_

/** @file node.hpp
 * @see NodeRef */

#include <cstddef>

#include "c4/yml/tree.hpp"
#include "c4/base64.hpp"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251/*needs to have dll-interface to be used by clients of struct*/)
#   pragma warning(disable: 4296/*expression is always 'boolean_value'*/)
#endif

namespace c4 {
namespace yml {

template<class K> struct Key { K & k; };
template<> struct Key<fmt::const_base64_wrapper> { fmt::const_base64_wrapper wrapper; };
template<> struct Key<fmt::base64_wrapper> { fmt::base64_wrapper wrapper; };

template<class K> C4_ALWAYS_INLINE Key<K> key(K & k) { return Key<K>{k}; }
C4_ALWAYS_INLINE Key<fmt::const_base64_wrapper> key(fmt::const_base64_wrapper w) { return {w}; }
C4_ALWAYS_INLINE Key<fmt::base64_wrapper> key(fmt::base64_wrapper w) { return {w}; }

template<class T> void write(NodeRef *n, T const& v);

template<class T>
typename std::enable_if< ! std::is_floating_point<T>::value, bool>::type
read(NodeRef const& n, T *v);

template<class T>
typename std::enable_if<   std::is_floating_point<T>::value, bool>::type
read(NodeRef const& n, T *v);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** a reference to a node in an existing yaml tree, offering a more
 * convenient API than the index-based API used in the tree. */
class RYML_EXPORT NodeRef
{
private:

    // require valid: a helper macro, undefined at the end
    #define _C4RV() RYML_ASSERT(valid() && !is_seed())

    Tree *C4_RESTRICT m_tree;
    size_t m_id;

    /** This member is used to enable lazy operator[] writing. When a child
     * with a key or index is not found, m_id is set to the id of the parent
     * and the asked-for key or index are stored in this member until a write
     * does happen. Then it is given as key or index for creating the child.
     * When a key is used, the csubstr stores it (so the csubstr's string is
     * non-null and the csubstr's size is different from NONE). When an index is
     * used instead, the csubstr's string is set to null, and only the csubstr's
     * size is set to a value different from NONE. Otherwise, when operator[]
     * does find the child then this member is empty: the string is null and
     * the size is NONE. */
    csubstr m_seed;

public:

    /** @name node construction */
    /** @{ */

    NodeRef() : m_tree(nullptr), m_id(NONE), m_seed() { _clear_seed(); }
    NodeRef(Tree &t) : m_tree(&t), m_id(t .root_id()), m_seed() { _clear_seed(); }
    NodeRef(Tree *t) : m_tree(t ), m_id(t->root_id()), m_seed() { _clear_seed(); }
    NodeRef(Tree *t, size_t id) : m_tree(t), m_id(id), m_seed() { _clear_seed(); }
    NodeRef(Tree *t, size_t id, size_t seed_pos) : m_tree(t), m_id(id), m_seed() { m_seed.str = nullptr; m_seed.len = seed_pos; }
    NodeRef(Tree *t, size_t id, csubstr  seed_key) : m_tree(t), m_id(id), m_seed(seed_key) {}
    NodeRef(std::nullptr_t) : m_tree(nullptr), m_id(NONE), m_seed() {}

    NodeRef(NodeRef const&) = default;
    NodeRef(NodeRef     &&) = default;

    NodeRef& operator= (NodeRef const&) = default;
    NodeRef& operator= (NodeRef     &&) = default;

    /** @} */

public:

    inline Tree      * tree()       { return m_tree; }
    inline Tree const* tree() const { return m_tree; }

    inline size_t id() const { return m_id; }

    inline NodeData      * get()       { return m_tree->get(m_id); }
    inline NodeData const* get() const { return m_tree->get(m_id); }

    inline bool operator== (NodeRef const& that) const { _C4RV(); RYML_ASSERT(that.valid() && !that.is_seed()); RYML_ASSERT(that.m_tree == m_tree); return m_id == that.m_id; }
    inline bool operator!= (NodeRef const& that) const { return ! this->operator==(that); }

    inline bool operator== (std::nullptr_t) const { return m_tree == nullptr || m_id == NONE || is_seed(); }
    inline bool operator!= (std::nullptr_t) const { return ! this->operator== (nullptr); }

    inline bool operator== (csubstr val) const { _C4RV(); RYML_ASSERT(has_val()); return m_tree->val(m_id) == val; }
    inline bool operator!= (csubstr val) const { _C4RV(); RYML_ASSERT(has_val()); return m_tree->val(m_id) != val; }

    //inline operator bool () const { return m_tree == nullptr || m_id == NONE || is_seed(); }

public:

    inline bool valid() const { return m_tree != nullptr && m_id != NONE; }
    inline bool is_seed() const { return m_seed.str != nullptr || m_seed.len != NONE; }

    inline void _clear_seed() { /*do this manually or an assert is triggered*/ m_seed.str = nullptr; m_seed.len = NONE; }

public:

    /** @name node property getters */
    /** @{ */

    inline NodeType     type() const { _C4RV(); return m_tree->type(m_id); }
    inline const char*  type_str() const { _C4RV(); RYML_ASSERT(valid() && ! is_seed()); return m_tree->type_str(m_id); }

    inline csubstr    key()        const { _C4RV(); return m_tree->key(m_id); }
    inline csubstr    key_tag()    const { _C4RV(); return m_tree->key_tag(m_id); }
    inline csubstr    key_ref()    const { _C4RV(); return m_tree->key_ref(m_id); }
    inline csubstr    key_anchor() const { _C4RV(); return m_tree->key_anchor(m_id); }
    inline NodeScalar keysc()      const { _C4RV(); return m_tree->keysc(m_id); }

    inline csubstr    val()        const { _C4RV(); return m_tree->val(m_id); }
    inline csubstr    val_tag()    const { _C4RV(); return m_tree->val_tag(m_id); }
    inline csubstr    val_ref()    const { _C4RV(); return m_tree->val_ref(m_id); }
    inline csubstr    val_anchor() const { _C4RV(); return m_tree->val_anchor(m_id); }
    inline NodeScalar valsc()      const { _C4RV(); return m_tree->valsc(m_id); }

    inline bool key_is_null() const { _C4RV(); return m_tree->key_is_null(m_id); }
    inline bool val_is_null() const { _C4RV(); return m_tree->val_is_null(m_id); }

    /** decode the base64-encoded key deserialize and assign the
     * decoded blob to the given buffer/
     * @return the size of base64-decoded blob */
    size_t deserialize_key(fmt::base64_wrapper v) const;
    /** decode the base64-encoded key deserialize and assign the
     * decoded blob to the given buffer/
     * @return the size of base64-decoded blob */
    size_t deserialize_val(fmt::base64_wrapper v) const;

    /** @} */

public:

    /** @name node property predicates */
    /** @{ */

    C4_ALWAYS_INLINE bool is_stream()        const { _C4RV(); return m_tree->is_stream(m_id); }
    C4_ALWAYS_INLINE bool is_doc()           const { _C4RV(); return m_tree->is_doc(m_id); }
    C4_ALWAYS_INLINE bool is_container()     const { _C4RV(); return m_tree->is_container(m_id); }
    C4_ALWAYS_INLINE bool is_map()           const { _C4RV(); return m_tree->is_map(m_id); }
    C4_ALWAYS_INLINE bool is_seq()           const { _C4RV(); return m_tree->is_seq(m_id); }
    C4_ALWAYS_INLINE bool has_val()          const { _C4RV(); return m_tree->has_val(m_id); }
    C4_ALWAYS_INLINE bool has_key()          const { _C4RV(); return m_tree->has_key(m_id); }
    C4_ALWAYS_INLINE bool is_val()           const { _C4RV(); return m_tree->is_val(m_id); }
    C4_ALWAYS_INLINE bool is_keyval()        const { _C4RV(); return m_tree->is_keyval(m_id); }
    C4_ALWAYS_INLINE bool has_key_tag()      const { _C4RV(); return m_tree->has_key_tag(m_id); }
    C4_ALWAYS_INLINE bool has_val_tag()      const { _C4RV(); return m_tree->has_val_tag(m_id); }
    C4_ALWAYS_INLINE bool has_key_anchor()   const { _C4RV(); return m_tree->has_key_anchor(m_id); }
    C4_ALWAYS_INLINE bool is_key_anchor()    const { _C4RV(); return m_tree->is_key_anchor(m_id); }
    C4_ALWAYS_INLINE bool has_val_anchor()   const { _C4RV(); return m_tree->has_val_anchor(m_id); }
    C4_ALWAYS_INLINE bool is_val_anchor()    const { _C4RV(); return m_tree->is_val_anchor(m_id); }
    C4_ALWAYS_INLINE bool has_anchor()       const { _C4RV(); return m_tree->has_anchor(m_id); }
    C4_ALWAYS_INLINE bool is_anchor()        const { _C4RV(); return m_tree->is_anchor(m_id); }
    C4_ALWAYS_INLINE bool is_key_ref()       const { _C4RV(); return m_tree->is_key_ref(m_id); }
    C4_ALWAYS_INLINE bool is_val_ref()       const { _C4RV(); return m_tree->is_val_ref(m_id); }
    C4_ALWAYS_INLINE bool is_ref()           const { _C4RV(); return m_tree->is_ref(m_id); }
    C4_ALWAYS_INLINE bool is_anchor_or_ref() const { _C4RV(); return m_tree->is_anchor_or_ref(m_id); }
    C4_ALWAYS_INLINE bool is_key_quoted()    const { _C4RV(); return m_tree->is_key_quoted(m_id); }
    C4_ALWAYS_INLINE bool is_val_quoted()    const { _C4RV(); return m_tree->is_val_quoted(m_id); }
    C4_ALWAYS_INLINE bool is_quoted()        const { _C4RV(); return m_tree->is_quoted(m_id); }

    C4_ALWAYS_INLINE bool parent_is_seq()    const { _C4RV(); return m_tree->parent_is_seq(m_id); }
    C4_ALWAYS_INLINE bool parent_is_map()    const { _C4RV(); return m_tree->parent_is_map(m_id); }

    /** true when name and value are empty, and has no children */
    C4_ALWAYS_INLINE bool empty() const { _C4RV(); return m_tree->empty(m_id); }

    /** @} */

public:

    /** @name hierarchy predicates */
    /** @{ */

    inline bool is_root()    const { _C4RV(); return m_tree->is_root(m_id); }
    inline bool has_parent() const { _C4RV(); return m_tree->has_parent(m_id); }

    inline bool has_child(NodeRef const& ch) const { _C4RV(); return m_tree->has_child(m_id, ch.m_id); }
    inline bool has_child(csubstr name) const { _C4RV();  return m_tree->has_child(m_id, name); }
    inline bool has_children() const { _C4RV(); return m_tree->has_children(m_id); }

    inline bool has_sibling(NodeRef const& n) const { _C4RV(); return m_tree->has_sibling(m_id, n.m_id); }
    inline bool has_sibling(csubstr name) const { _C4RV();  return m_tree->has_sibling(m_id, name); }
    /** counts with this */
    inline bool has_siblings() const { _C4RV(); return m_tree->has_siblings(m_id); }
    /** does not count with this */
    inline bool has_other_siblings() const { _C4RV(); return m_tree->has_other_siblings(m_id); }

    /** @} */

public:

    /** @name hierarchy getters */
    /** @{ */

    NodeRef       parent()       { _C4RV(); return {m_tree, m_tree->parent(m_id)}; }
    NodeRef const parent() const { _C4RV(); return {m_tree, m_tree->parent(m_id)}; }

    NodeRef       prev_sibling()       { _C4RV(); return {m_tree, m_tree->prev_sibling(m_id)}; }
    NodeRef const prev_sibling() const { _C4RV(); return {m_tree, m_tree->prev_sibling(m_id)}; }

    NodeRef       next_sibling()       { _C4RV(); return {m_tree, m_tree->next_sibling(m_id)}; }
    NodeRef const next_sibling() const { _C4RV(); return {m_tree, m_tree->next_sibling(m_id)}; }

    /** O(#num_children) */
    size_t  num_children() const { _C4RV(); return m_tree->num_children(m_id); }
    size_t  child_pos(NodeRef const& n) const { _C4RV(); return m_tree->child_pos(m_id, n.m_id); }
    NodeRef       first_child()       { _C4RV(); return {m_tree, m_tree->first_child(m_id)}; }
    NodeRef const first_child() const { _C4RV(); return {m_tree, m_tree->first_child(m_id)}; }
    NodeRef       last_child ()       { _C4RV(); return {m_tree, m_tree->last_child (m_id)}; }
    NodeRef const last_child () const { _C4RV(); return {m_tree, m_tree->last_child (m_id)}; }
    NodeRef       child(size_t pos)       { _C4RV(); return {m_tree, m_tree->child(m_id, pos)}; }
    NodeRef const child(size_t pos) const { _C4RV(); return {m_tree, m_tree->child(m_id, pos)}; }
    NodeRef       find_child(csubstr name)       { _C4RV(); return {m_tree, m_tree->find_child(m_id, name)}; }
    NodeRef const find_child(csubstr name) const { _C4RV(); return {m_tree, m_tree->find_child(m_id, name)}; }

    /** O(#num_siblings) */
    size_t  num_siblings() const { _C4RV(); return m_tree->num_siblings(m_id); }
    size_t  num_other_siblings() const { _C4RV(); return m_tree->num_other_siblings(m_id); }
    size_t  sibling_pos(NodeRef const& n) const { _C4RV(); return m_tree->child_pos(m_tree->parent(m_id), n.m_id); }
    NodeRef       first_sibling()       { _C4RV(); return {m_tree, m_tree->first_sibling(m_id)}; }
    NodeRef const first_sibling() const { _C4RV(); return {m_tree, m_tree->first_sibling(m_id)}; }
    NodeRef       last_sibling ()       { _C4RV(); return {m_tree, m_tree->last_sibling(m_id)}; }
    NodeRef const last_sibling () const { _C4RV(); return {m_tree, m_tree->last_sibling(m_id)}; }
    NodeRef       sibling(size_t pos)       { _C4RV(); return {m_tree, m_tree->sibling(m_id, pos)}; }
    NodeRef const sibling(size_t pos) const { _C4RV(); return {m_tree, m_tree->sibling(m_id, pos)}; }
    NodeRef       find_sibling(csubstr name)       { _C4RV(); return {m_tree, m_tree->find_sibling(m_id, name)}; }
    NodeRef const find_sibling(csubstr name) const { _C4RV(); return {m_tree, m_tree->find_sibling(m_id, name)}; }

    NodeRef       doc(size_t num)       { _C4RV(); return {m_tree, m_tree->doc(num)}; }
    NodeRef const doc(size_t num) const { _C4RV(); return {m_tree, m_tree->doc(num)}; }

    /** @} */

public:

    /** @name node modifiers */
    /** @{ */

    void change_type(NodeType t) { _C4RV(); m_tree->change_type(m_id, t); }
    void set_type(NodeType t) { _C4RV(); m_tree->_set_flags(m_id, t); }
    void set_key(csubstr key) { _C4RV(); m_tree->_set_key(m_id, key); }
    void set_val(csubstr val) { _C4RV(); m_tree->_set_val(m_id, val); }
    void set_key_tag(csubstr key_tag) { _C4RV(); m_tree->set_key_tag(m_id, key_tag); }
    void set_val_tag(csubstr val_tag) { _C4RV(); m_tree->set_val_tag(m_id, val_tag); }
    void set_key_anchor(csubstr key_anchor) { _C4RV(); m_tree->set_key_anchor(m_id, key_anchor); }
    void set_val_anchor(csubstr val_anchor) { _C4RV(); m_tree->set_val_anchor(m_id, val_anchor); }
    void set_key_ref(csubstr key_ref) { _C4RV(); m_tree->set_key_ref(m_id, key_ref); }
    void set_val_ref(csubstr val_ref) { _C4RV(); m_tree->set_val_ref(m_id, val_ref); }

    template<class T>
    size_t set_key_serialized(T const& C4_RESTRICT k)
    {
        _C4RV();
        csubstr s = m_tree->to_arena(k);
        m_tree->_set_key(m_id, s);
        return s.len;
    }
    template<class T>
    size_t set_val_serialized(T const& C4_RESTRICT v)
    {
        _C4RV();
        csubstr s = m_tree->to_arena(v);
        m_tree->_set_val(m_id, s);
        return s.len;
    }

    /** encode a blob as base64, then assign the result to the node's key
     * @return the size of base64-encoded blob */
    size_t set_key_serialized(fmt::const_base64_wrapper w);
    /** encode a blob as base64, then assign the result to the node's val
     * @return the size of base64-encoded blob */
    size_t set_val_serialized(fmt::const_base64_wrapper w);

public:

    inline void clear()
    {
        if(is_seed())
            return;
        m_tree->remove_children(m_id);
        m_tree->_clear(m_id);
    }

    inline void clear_key()
    {
        if(is_seed())
            return;
        m_tree->_clear_key(m_id);
    }

    inline void clear_val()
    {
        if(is_seed())
            return;
        m_tree->_clear_val(m_id);
    }

    inline void clear_children()
    {
        if(is_seed())
            return;
        m_tree->remove_children(m_id);
    }

    /** @} */

public:

    /** hierarchy getters */
    /** @{ */

    /** O(num_children) */
    NodeRef operator[] (csubstr k)
    {
        RYML_ASSERT( ! is_seed());
        RYML_ASSERT(valid());
        size_t ch = m_tree->find_child(m_id, k);
        NodeRef r = ch != NONE ? NodeRef(m_tree, ch) : NodeRef(m_tree, m_id, k);
        return r;
    }

    /** O(num_children) */
    NodeRef const operator[] (csubstr k) const
    {
        RYML_ASSERT( ! is_seed());
        RYML_ASSERT(valid());
        size_t ch = m_tree->find_child(m_id, k);
        RYML_ASSERT(ch != NONE);
        NodeRef const r(m_tree, ch);
        return r;
    }

    /** O(num_children) */
    NodeRef operator[] (size_t pos)
    {
        RYML_ASSERT( ! is_seed());
        RYML_ASSERT(valid());
        size_t ch = m_tree->child(m_id, pos);
        NodeRef r = ch != NONE ? NodeRef(m_tree, ch) : NodeRef(m_tree, m_id, pos);
        return r;
    }

    /** O(num_children) */
    NodeRef const operator[] (size_t pos) const
    {
        RYML_ASSERT( ! is_seed());
        RYML_ASSERT(valid());
        size_t ch = m_tree->child(m_id, pos);
        RYML_ASSERT(ch != NONE);
        NodeRef const r(m_tree, ch);
        return r;
    }

    /** @} */

public:

    /** node modification */
    /** @{ */

    void create() { _apply_seed(); }

    inline void operator= (NodeType_e t)
    {
        _apply_seed();
        m_tree->_add_flags(m_id, t);
    }

    inline void operator|= (NodeType_e t)
    {
        _apply_seed();
        m_tree->_add_flags(m_id, t);
    }

    inline void operator= (NodeInit const& v)
    {
        _apply_seed();
        _apply(v);
    }

    inline void operator= (NodeScalar const& v)
    {
        _apply_seed();
        _apply(v);
    }

    inline void operator= (csubstr v)
    {
        _apply_seed();
        _apply(v);
    }

    template<size_t N>
    inline void operator= (const char (&v)[N])
    {
        _apply_seed();
        csubstr sv;
        sv.assign<N>(v);
        _apply(sv);
    }

    /** @} */

public:

    /** serialize a variable to the arena */
    template<class T>
    inline csubstr to_arena(T const& C4_RESTRICT s) const
    {
        _C4RV();
        return m_tree->to_arena(s);
    }

    /** serialize a variable, then assign the result to the node's val */
    inline NodeRef& operator<< (csubstr s)
    {
        // this overload is needed to prevent ambiguity (there's also
        // operator<< for writing a substr to a stream)
        _apply_seed();
        write(this, s);
        RYML_ASSERT(val() == s);
        return *this;
    }

    template<class T>
    inline NodeRef& operator<< (T const& C4_RESTRICT v)
    {
        _apply_seed();
        write(this, v);
        return *this;
    }

    template<class T>
    inline NodeRef const& operator>> (T &v) const
    {
        RYML_ASSERT( ! is_seed());
        RYML_ASSERT(valid());
        RYML_ASSERT(get() != nullptr);
        if( ! read(*this, &v))
        {
            c4::yml::error("could not deserialize value");
        }
        return *this;
    }

public:

    /** serialize a variable, then assign the result to the node's key */
    template<class T>
    inline NodeRef& operator<< (Key<const T> const& C4_RESTRICT v)
    {
        _apply_seed();
        set_key_serialized(v.k);
        return *this;
    }

    /** serialize a variable, then assign the result to the node's key */
    template<class T>
    inline NodeRef& operator<< (Key<T> const& C4_RESTRICT v)
    {
        _apply_seed();
        set_key_serialized(v.k);
        return *this;
    }

    /** deserialize the node's key to the given variable */
    template<class T>
    inline NodeRef const& operator>> (Key<T> v) const
    {
        RYML_ASSERT( ! is_seed());
        RYML_ASSERT(valid());
        RYML_ASSERT(get() != nullptr);
        from_chars(key(), &v.k);
        return *this;
    }

public:

    NodeRef& operator<< (Key<fmt::const_base64_wrapper> w)
    {
        set_key_serialized(w.wrapper);
        return *this;
    }

    NodeRef& operator<< (fmt::const_base64_wrapper w)
    {
        set_val_serialized(w);
        return *this;
    }

    NodeRef const& operator>> (Key<fmt::base64_wrapper> w) const
    {
        deserialize_key(w.wrapper);
        return *this;
    }

    NodeRef const& operator>> (fmt::base64_wrapper w) const
    {
        deserialize_val(w);
        return *this;
    }

public:

    template<class T>
    void get_if(csubstr name, T *var) const
    {
        auto ch = find_child(name);
        if(ch.valid())
        {
            ch >> *var;
        }
    }

    template<class T>
    void get_if(csubstr name, T *var, T fallback) const
    {
        auto ch = find_child(name);
        if(ch.valid())
        {
            ch >> *var;
        }
        else
        {
            *var = fallback;
        }
    }

private:

    void _apply_seed()
    {
        if(m_seed.str) // we have a seed key: use it to create the new child
        {
            //RYML_ASSERT(i.key.scalar.empty() || m_key == i.key.scalar || m_key.empty());
            m_id = m_tree->append_child(m_id);
            m_tree->_set_key(m_id, m_seed);
            m_seed.str = nullptr;
            m_seed.len = NONE;
        }
        else if(m_seed.len != NONE) // we have a seed index: create a child at that position
        {
            RYML_ASSERT(m_tree->num_children(m_id) == m_seed.len);
            m_id = m_tree->append_child(m_id);
            m_seed.str = nullptr;
            m_seed.len = NONE;
        }
        else
        {
            RYML_ASSERT(valid());
        }
    }

    inline void _apply(csubstr v)
    {
        m_tree->_set_val(m_id, v);
    }

    inline void _apply(NodeScalar const& v)
    {
        m_tree->_set_val(m_id, v);
    }

    inline void _apply(NodeInit const& i)
    {
        m_tree->_set(m_id, i);
    }

public:

    inline NodeRef insert_child(NodeRef after)
    {
        _C4RV();
        RYML_ASSERT(after.m_tree == m_tree);
        NodeRef r(m_tree, m_tree->insert_child(m_id, after.m_id));
        return r;
    }

    inline NodeRef insert_child(NodeInit const& i, NodeRef after)
    {
        _C4RV();
        RYML_ASSERT(after.m_tree == m_tree);
        NodeRef r(m_tree, m_tree->insert_child(m_id, after.m_id));
        r._apply(i);
        return r;
    }

    inline NodeRef prepend_child()
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->insert_child(m_id, NONE));
        return r;
    }

    inline NodeRef prepend_child(NodeInit const& i)
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->insert_child(m_id, NONE));
        r._apply(i);
        return r;
    }

    inline NodeRef append_child()
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->append_child(m_id));
        return r;
    }

    inline NodeRef append_child(NodeInit const& i)
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->append_child(m_id));
        r._apply(i);
        return r;
    }

public:

    inline NodeRef insert_sibling(NodeRef const after)
    {
        _C4RV();
        RYML_ASSERT(after.m_tree == m_tree);
        NodeRef r(m_tree, m_tree->insert_sibling(m_id, after.m_id));
        return r;
    }

    inline NodeRef insert_sibling(NodeInit const& i, NodeRef const after)
    {
        _C4RV();
        RYML_ASSERT(after.m_tree == m_tree);
        NodeRef r(m_tree, m_tree->insert_sibling(m_id, after.m_id));
        r._apply(i);
        return r;
    }

    inline NodeRef prepend_sibling()
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->prepend_sibling(m_id));
        return r;
    }

    inline NodeRef prepend_sibling(NodeInit const& i)
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->prepend_sibling(m_id));
        r._apply(i);
        return r;
    }

    inline NodeRef append_sibling()
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->append_sibling(m_id));
        return r;
    }

    inline NodeRef append_sibling(NodeInit const& i)
    {
        _C4RV();
        NodeRef r(m_tree, m_tree->append_sibling(m_id));
        r._apply(i);
        return r;
    }

public:

    inline void remove_child(NodeRef & child)
    {
        _C4RV();
        RYML_ASSERT(has_child(child));
        RYML_ASSERT(child.parent().id() == id());
        m_tree->remove(child.id());
        child.clear();
    }

    //! remove the nth child of this node
    inline void remove_child(size_t pos)
    {
        _C4RV();
        RYML_ASSERT(pos >= 0 && pos < num_children());
        size_t child = m_tree->child(m_id, pos);
        RYML_ASSERT(child != NONE);
        m_tree->remove(child);
    }

    //! remove a child by name
    inline void remove_child(csubstr key)
    {
        _C4RV();
        size_t child = m_tree->find_child(m_id, key);
        RYML_ASSERT(child != NONE);
        m_tree->remove(child);
    }

public:

    /** change the node's position within its parent */
    inline void move(NodeRef const after)
    {
        _C4RV();
        m_tree->move(m_id, after.m_id);
    }

    /** move the node to a different parent, which may belong to a different
     * tree. When this is the case, then this node's tree pointer is reset to
     * the tree of the parent node. */
    inline void move(NodeRef const parent, NodeRef const after)
    {
        _C4RV();
        RYML_ASSERT(parent.m_tree == after.m_tree);
        if(parent.m_tree == m_tree)
        {
            m_tree->move(m_id, parent.m_id, after.m_id);
        }
        else
        {
            parent.m_tree->move(m_tree, m_id, parent.m_id, after.m_id);
            m_tree = parent.m_tree;
        }
    }

    inline NodeRef duplicate(NodeRef const parent, NodeRef const after) const
    {
        _C4RV();
        RYML_ASSERT(parent.m_tree == after.m_tree);
        if(parent.m_tree == m_tree)
        {
            size_t dup = m_tree->duplicate(m_id, parent.m_id, after.m_id);
            NodeRef r(m_tree, dup);
            return r;
        }
        else
        {
            size_t dup = parent.m_tree->duplicate(m_tree, m_id, parent.m_id, after.m_id);
            NodeRef r(parent.m_tree, dup);
            return r;
        }
    }

    inline void duplicate_children(NodeRef const parent, NodeRef const after) const
    {
        _C4RV();
        RYML_ASSERT(parent.m_tree == after.m_tree);
        if(parent.m_tree == m_tree)
        {
            m_tree->duplicate_children(m_id, parent.m_id, after.m_id);
        }
        else
        {
            parent.m_tree->duplicate_children(m_tree, m_id, parent.m_id, after.m_id);
        }
    }

private:

    template<class Nd>
    struct child_iterator
    {
        Tree * m_tree;
        size_t m_child_id;

        using value_type = NodeRef;

        child_iterator(Tree * t, size_t id) : m_tree(t), m_child_id(id) {}

        child_iterator& operator++ () { RYML_ASSERT(m_child_id != NONE); m_child_id = m_tree->next_sibling(m_child_id); return *this; }
        child_iterator& operator-- () { RYML_ASSERT(m_child_id != NONE); m_child_id = m_tree->prev_sibling(m_child_id); return *this; }

        Nd operator*  () const { return Nd(m_tree, m_child_id); }
        Nd operator-> () const { return Nd(m_tree, m_child_id); }

        bool operator!= (child_iterator that) const { RYML_ASSERT(m_tree == that.m_tree); return m_child_id != that.m_child_id; }
        bool operator== (child_iterator that) const { RYML_ASSERT(m_tree == that.m_tree); return m_child_id == that.m_child_id; }
    };

public:

    using       iterator = child_iterator<      NodeRef>;
    using const_iterator = child_iterator<const NodeRef>;

    inline iterator begin() { return iterator(m_tree, m_tree->first_child(m_id)); }
    inline iterator end  () { return iterator(m_tree, NONE); }

    inline const_iterator begin() const { return const_iterator(m_tree, m_tree->first_child(m_id)); }
    inline const_iterator end  () const { return const_iterator(m_tree, NONE); }

private:

    template<class Nd>
    struct children_view_
    {
        using n_iterator = child_iterator<Nd>;

        n_iterator b, e;

        inline children_view_(n_iterator const& b_, n_iterator const& e_) : b(b_), e(e_) {}

        inline n_iterator begin() const { return b; }
        inline n_iterator end  () const { return e; }
    };

public:

    using       children_view = children_view_<      NodeRef>;
    using const_children_view = children_view_<const NodeRef>;

          children_view children()       { return       children_view(begin(), end()); }
    const_children_view children() const { return const_children_view(begin(), end()); }

    #if defined(__clang__)
    #   pragma clang diagnostic push
    #   pragma clang diagnostic ignored "-Wnull-dereference"
    #elif defined(__GNUC__)
    #   pragma GCC diagnostic push
    #   if __GNUC__ >= 6
    #       pragma GCC diagnostic ignored "-Wnull-dereference"
    #   endif
    #endif

          children_view siblings()       { if(is_root()) { return       children_view(end(), end()); } else { size_t p = get()->m_parent; return       children_view(iterator(m_tree, m_tree->get(p)->m_first_child), iterator(m_tree, NONE)); } }
    const_children_view siblings() const { if(is_root()) { return const_children_view(end(), end()); } else { size_t p = get()->m_parent; return const_children_view(const_iterator(m_tree, m_tree->get(p)->m_first_child), const_iterator(m_tree, NONE)); } }

    #if defined(__clang__)
    #   pragma clang diagnostic pop
    #elif defined(__GNUC__)
    #   pragma GCC diagnostic pop
    #endif

public:

    /** visit every child node calling fn(node) */
    template<class Visitor> bool visit(Visitor fn, size_t indentation_level=0, bool skip_root=true);
    /** visit every child node calling fn(node) */
    template<class Visitor> bool visit(Visitor fn, size_t indentation_level=0, bool skip_root=true) const;

    /** visit every child node calling fn(node, level) */
    template<class Visitor> bool visit_stacked(Visitor fn, size_t indentation_level=0, bool skip_root=true);
    /** visit every child node calling fn(node, level) */
    template<class Visitor> bool visit_stacked(Visitor fn, size_t indentation_level=0, bool skip_root=true) const;

#undef _C4RV
};

//-----------------------------------------------------------------------------
template<class T>
inline void write(NodeRef *n, T const& v)
{
    n->set_val_serialized(v);
}

template<class T>
typename std::enable_if< ! std::is_floating_point<T>::value, bool>::type
inline read(NodeRef const& n, T *v)
{
    return from_chars(n.val(), v);
}

template<class T>
typename std::enable_if< std::is_floating_point<T>::value, bool>::type
inline read(NodeRef const& n, T *v)
{
    return from_chars_float(n.val(), v);
}


//-----------------------------------------------------------------------------
template<class Visitor>
bool NodeRef::visit(Visitor fn, size_t indentation_level, bool skip_root)
{
    return const_cast<NodeRef const*>(this)->visit(fn, indentation_level, skip_root);
}

template<class Visitor>
bool NodeRef::visit(Visitor fn, size_t indentation_level, bool skip_root) const
{
    size_t increment = 0;
    if( ! (is_root() && skip_root))
    {
        if(fn(this, indentation_level))
        {
            return true;
        }
        ++increment;
    }
    if(has_children())
    {
        for(auto ch : children())
        {
            if(ch.visit(fn, indentation_level + increment)) // no need to forward skip_root as it won't be root
            {
                return true;
            }
        }
    }
    return false;
}


template<class Visitor>
bool NodeRef::visit_stacked(Visitor fn, size_t indentation_level, bool skip_root)
{
    return const_cast< NodeRef const* >(this)->visit_stacked(fn, indentation_level, skip_root);
}

template<class Visitor>
bool NodeRef::visit_stacked(Visitor fn, size_t indentation_level, bool skip_root) const
{
    size_t increment = 0;
    if( ! (is_root() && skip_root))
    {
        if(fn(this, indentation_level))
        {
            return true;
        }
        ++increment;
    }
    if(has_children())
    {
        fn.push(this, indentation_level);
        for(auto ch : children())
        {
            if(ch.visit(fn, indentation_level + increment)) // no need to forward skip_root as it won't be root
            {
                fn.pop(this, indentation_level);
                return true;
            }
        }
        fn.pop(this, indentation_level);
    }
    return false;
}

} // namespace yml
} // namespace c4


#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

#endif /* _C4_YML_NODE_HPP_ */
