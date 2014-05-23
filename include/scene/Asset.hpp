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

#pragma once

#include <string>
#include <vector>

namespace Scene {

/** Tracks creation and modification time of assets.
  *
  * A minor change is any update to the underlying assets. This will be
  * exported as the "modified" property of the asset. A minor update doesn't
  * necessarily require a render list to be rebuilt. A minor change is usually
  * a change of a parameter etc.
  *
  * A major change is an update that requires render lists to be rebuilt. This
  * involves adding new geomtry, a new shader etc.
  */
class Asset
{
public:
    Asset();


    bool
    operator==(const Asset& other) const;

    const std::string
    created() const;

    void
    setCreated( const std::string& t );


    const std::string
    modified() const;

    void
    setModified( const std::string& t );


    /** Add a new contributor to this asset. */
    size_t
    addContributor( );

    /** Get the number of contributors to this asset. */
    size_t
    contributors() const;


    /** Author's name for a particular contribution. */
    const std::string&
    author( size_t contribution ) const;

    /** Author's full email address for a particular contribution. */
    const std::string&
    authorEmail( size_t contribution ) const;

    /** The name of the authoring tool used in a particular contribution. */
    const std::string&
    authoringTool( size_t contribution ) const;

    /** Contains a string with comments from this contributor. */
    const std::string&
    comments() const;

    const std::string&
    sourceDataURL() const;

    const std::string&
    subject() const;

    void
    setSubject( const std::string& contents );

    const std::string&
    title() const;




protected:
    std::string   m_created;
    std::string   m_modified;

    std::string  m_author;
    std::string  m_author_email;
    std::string  m_authoring_tool;
    std::string  m_comments;
    std::string  m_copyright;
    std::string  m_source_data_url;
    std::string  m_subject;
    std::string  m_title;
    struct Contributor
    {
        std::string  m_author_name;
        std::string  m_author_email;
        std::string  m_authoring_tool;
        std::string  m_comments;
        std::string  m_copyright;
        std::string  m_source_data_url;
    };
    std::vector<Contributor>  m_contributors;
};




} // of Scene
