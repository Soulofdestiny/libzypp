/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoInfo.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/RepoInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo::Impl
  //
  /** RepoInfo implementation. */
  struct RepoInfo::Impl
  {

    Impl()
      : enabled (false),
        autorefresh(false),
        gpgcheck(true),
        type(repo::RepoType::NONE_e)
    {}

    ~Impl()
    {
      //MIL << std::endl;
    }
  public:
    bool enabled;
    bool autorefresh;
    bool gpgcheck;
    Url gpgkey_url;
    repo::RepoType type;
    Url mirrorlist_url;
    std::set<Url> baseUrls;
    Pathname path;
    std::string alias;
    std::string name;
    Pathname filepath;
    Pathname metadatapath;
  public:

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoInfo::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoInfo::Impl & obj )
  {
    return str << "RepoInfo::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoInfo::RepoInfo
  //	METHOD TYPE : Ctor
  //
  RepoInfo::RepoInfo()
  : _pimpl( new Impl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoInfo::~RepoInfo
  //	METHOD TYPE : Dtor
  //
  RepoInfo::~RepoInfo()
  {
    //MIL << std::endl;
  }

  RepoInfo & RepoInfo::setEnabled( bool enabled )
  {
    _pimpl->enabled = enabled;
    return *this;
  }

  RepoInfo & RepoInfo::setAutorefresh( bool autorefresh )
  {
    _pimpl->autorefresh = autorefresh;
    return *this;
  }

  RepoInfo & RepoInfo::setGpgCheck( bool check )
  {
    _pimpl->gpgcheck = check;
    return *this;
  }

  RepoInfo & RepoInfo::setMirrorListUrl( const Url &url )
  {
    _pimpl->mirrorlist_url = url;
    return *this;
  }

  RepoInfo & RepoInfo::setGpgKeyUrl( const Url &url )
  {
    _pimpl->gpgkey_url = url;
    return *this;
  }

  RepoInfo & RepoInfo::addBaseUrl( const Url &url )
  {
    _pimpl->baseUrls.insert(url);
    return *this;
  }

  RepoInfo & RepoInfo::setBaseUrl( const Url &url )
  {
    _pimpl->baseUrls.clear();
    addBaseUrl(url);
    return *this;
  }

  RepoInfo & RepoInfo::setPath( const Pathname &path )
  {
    _pimpl->path = path;
    return *this;
  }

  RepoInfo & RepoInfo::setAlias( const std::string &alias )
  {
    _pimpl->alias = alias;
    return *this;
  }

  RepoInfo & RepoInfo::setType( const repo::RepoType &t )
  {
    _pimpl->type = t;
    return *this;
  }

  RepoInfo & RepoInfo::setName( const std::string &name )
  {
    _pimpl->name = name;
    return *this;
  }

  RepoInfo & RepoInfo::setFilepath( const Pathname &filepath )
  {
    _pimpl->filepath = filepath;
    return *this;
  }

  RepoInfo & RepoInfo::setMetadataPath( const Pathname &path )
  {
    _pimpl->metadatapath = path;
    return *this;
  }

  bool RepoInfo::enabled() const
  { return _pimpl->enabled; }

  bool RepoInfo::autorefresh() const
  { return _pimpl->autorefresh; }

  bool RepoInfo::gpgCheck() const
  { return _pimpl->gpgcheck; }

  std::string RepoInfo::alias() const
  { return _pimpl->alias; }

  std::string RepoInfo::name() const
  {
    if ( _pimpl->name.empty() )
    {
      return alias();
    }
    
    repo::RepoVariablesStringReplacer replacer;
    return replacer(_pimpl->name);
  }

  Pathname RepoInfo::filepath() const
  { return _pimpl->filepath; }

  Pathname RepoInfo::metadataPath() const
  { return _pimpl->metadatapath; }

  repo::RepoType RepoInfo::type() const
  { return _pimpl->type; }

  Url RepoInfo::mirrorListUrl() const
  { return _pimpl->mirrorlist_url; }

  Url RepoInfo::gpgKeyUrl() const
  { return _pimpl->gpgkey_url; }

  std::set<Url> RepoInfo::baseUrls() const
  {
    RepoInfo::url_set replaced_urls;
    repo::RepoVariablesUrlReplacer replacer;
    for ( url_set::const_iterator it = _pimpl->baseUrls.begin();
          it != _pimpl->baseUrls.end();
          ++it )
    {
      replaced_urls.insert(replacer(*it));
    }
    return replaced_urls;

    return _pimpl->baseUrls;
  }

  Pathname RepoInfo::path() const
  { return _pimpl->path; }

  RepoInfo::urls_const_iterator RepoInfo::baseUrlsBegin() const
  {
    return make_transform_iterator( _pimpl->baseUrls.begin(),
                                    repo::RepoVariablesUrlReplacer() );
    //return _pimpl->baseUrls.begin();
  }

  RepoInfo::urls_const_iterator RepoInfo::baseUrlsEnd() const
  {
    //return _pimpl->baseUrls.end();
    return make_transform_iterator( _pimpl->baseUrls.end(),
                                    repo::RepoVariablesUrlReplacer() );
  }

  RepoInfo::urls_size_type RepoInfo::baseUrlsSize() const
  { return _pimpl->baseUrls.size(); }

  bool RepoInfo::baseUrlsEmpty() const
  { return _pimpl->baseUrls.empty(); }

  std::ostream & RepoInfo::dumpOn( std::ostream & str ) const
  {
    str << "--------------------------------------" << std::endl;
    str << "- alias       : " << alias() << std::endl;
    for ( urls_const_iterator it = baseUrlsBegin();
          it != baseUrlsEnd();
          ++it )
    {
      str << "- url         : " << *it << std::endl;
    }
    str << "- path        : " << path() << std::endl;
    str << "- type        : " << type() << std::endl;
    str << "- enabled     : " << enabled() << std::endl;

    str << "- autorefresh : " << autorefresh() << std::endl;
    str << "- gpgcheck : " << gpgCheck() << std::endl;
    str << "- gpgkey : " << gpgKeyUrl() << std::endl;

    return str;
  }

  std::ostream & RepoInfo::dumpRepoOn( std::ostream & str ) const
  {
    // we save the original data without variable replacement
    str << "[" << alias() << "]" << endl;
    str << "name=" << _pimpl->name << endl;

    if ( ! _pimpl->baseUrls.empty() )
      str << "baseurl=";
    for ( url_set::const_iterator it = _pimpl->baseUrls.begin();
          it != _pimpl->baseUrls.end();
          ++it )
    {
      str << *it << endl;
    }

    if ( ! _pimpl->path.empty() )
      str << "path="<< path() << endl;

    if ( ! (_pimpl->mirrorlist_url.asString().empty()) )
      str << "mirrorlist=" << _pimpl->mirrorlist_url << endl;

    str << "type=" << type().asString() << endl;
    str << "enabled=" << (enabled() ? "1" : "0") << endl;
    str << "autorefresh=" << (autorefresh() ? "1" : "0") << endl;
    str << "gpgcheck=" << (gpgCheck() ? "1" : "0") << endl;
    if ( ! (gpgKeyUrl().asString().empty()) )
      str << "gpgkey=" <<gpgKeyUrl() << endl;

    return str;
  }

  std::ostream & operator<<( std::ostream & str, const RepoInfo & obj )
  {
    return obj.dumpOn(str);
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////