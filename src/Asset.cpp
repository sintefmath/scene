/* Copyright STIFTELSEN SINTEF 2014
 * 
 * This file is part of Scene.
 * 
 * Scene is free software: you can redistribute it and/or modifyit under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * Scene is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
 * details.
 *  
 * You should have received a copy of the GNU Affero General Public License
 * along with the Scene.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "scene/Asset.hpp"

namespace Scene {
    using std::string;


Asset::Asset() {
}

bool
Asset::operator==(const Asset& other) const
{
    return (m_created == other.m_created) &&
           (m_modified == other.m_modified );
}

const std::string
Asset::created() const
{
    return m_created;
}

void
Asset::setCreated( const std::string& t )
{
    m_created = t;
}

const std::string
Asset::modified() const
{
    return m_modified;
}

void
Asset::setModified( const std::string& t )
{
    m_modified = t;
}

void
Asset::setSubject( const std::string& contents )
{
    m_subject = contents;
}

const std::string&
Asset::subject() const
{
    return m_subject;
}


/*
const std::string&
Asset::author() const
{
    return m_author;
}

void
Asset::setAuthor( const std::string& author )
{
    m_author = author;
}


const std::string&
Asset::authorEmail() const
{
    return m_author_email;
}

void
Asset::setAuthorEmail( const std::string& author_email )
{
    m_author_email = author_email;
}

const std::string&
Asset::authoringTool() const
{
    return m_authoring_tool;
}

void
Asset::setAuthoringTool( const std::string& authoring_tool )
{
    m_authoring_tool = authoring_tool;
}

const std::string&
Asset::comments() const
{
    return m_comments;
}

void
Asset::setComments() ( const std::string& comments )
{
    m_comments = comments;
}

const std::string&
Asset::sourceDataURL() const
{
    return m_source_data_url;
}

void
Asset::setSourceDataURL( const std::string& source_data_url )
{
    m_source_data_url = source_data_url;
}

const std::string&
Asset::subject() const
{
    return m_subject;
}

void
Asset::setSubject( const std::string& subject )
{
    m_subject = subject;
}

const std::string&
Asset::title() const
{
    return m_title;
}

void
Asset::setTitle( const std::string& title )
{
    m_title = title;
}


size_t
Asset::contributors() const
{
    return m_contributors.size();
}

size_t
Asset::addContributor( )
{
    size_t ix = m_contributors.size();
    m_contributors.resize( m_contributors.size() + 1 );
    return ix;
}
*/




} // of namespace Scene
