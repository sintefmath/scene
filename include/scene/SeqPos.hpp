#pragma once
#include <algorithm>
#include <boost/utility.hpp>


namespace Scene {


/** Runtime computation order timestamp.
  *
  *
  *
  *
  */
class SeqPos
{
public:
    SeqPos() : m_pos( 0u ) {}

    void
    invalidate()
    { m_pos = 0u; }

    bool
    asRecentAs( const SeqPos& a ) const
    { return m_pos >= a.m_pos; }

    void
    touch()
    { m_pos = ++m_global_next_pos; }

    static const SeqPos&
    mostRecent( const SeqPos& a, const SeqPos& b )
    { return a.m_pos > b.m_pos ? a : b; }

    const std::string
    string() const;

    const std::string
    debugString() const;

    /** Move this sequence number to argument if it is more recent.
      *
      * \returns True if this sequence number was changed.
      */
    bool
    moveForward( const SeqPos& a )
    {
        if( m_pos < a.m_pos ) {
            m_pos = a.m_pos;
            return true;
        }
        else {
            return false;
        }
    }

protected:
    size_t          m_pos;
    static size_t   m_global_next_pos;

};


class Identifiable : boost::noncopyable
{
public:
    typedef size_t  Id;

    Identifiable() : m_identity( m_identity_pool++) {}

    // Not supported in VS10 or VS11
    //Identifiable & operator=(const Identifiable&) = delete;
    //Identifiable( const Identifiable& ) = delete;

    const Id id() const { return m_identity; }

    const std::string idString() const;

protected:
    Id        m_identity;
    static Id m_identity_pool;
};


class StructureValueSequences : public Identifiable
{
public:

    void
    touchValueChanged()
    {
        m_value_changed.touch();
    }

    void
    touchStructureChanged()
    {
        m_structure_changed.touch();
        m_value_changed = m_structure_changed;
    }

    SeqPos&
    valueChanged() { return m_value_changed; }

    const SeqPos&
    valueChanged() const { return m_value_changed; }

    SeqPos&
    structureChanged() { return m_structure_changed; }

    const SeqPos&
    structureChanged() const { return m_structure_changed; }

    void
    moveForward( const StructureValueSequences& other )
    {
        m_value_changed = SeqPos::mostRecent( m_value_changed, other.m_value_changed );
        m_structure_changed = SeqPos::mostRecent( m_structure_changed, other.m_structure_changed );
    }

protected:
    SeqPos      m_value_changed;
    SeqPos      m_structure_changed;
};



} // of namespace Scene
