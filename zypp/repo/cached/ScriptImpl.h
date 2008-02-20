/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef zypp_repo_cached_ScriptImpl_H
#define zypp_repo_cached_ScriptImpl_H

#include "zypp/detail/ScriptImpl.h"
#include "zypp/repo/cached/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace cached
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //        CLASS NAME : ScriptImpl
  //
  class ScriptImpl : public detail::ScriptImplIf
  {
  public:

    ScriptImpl( const data::RecordId &id, repo::cached::RepoImpl::Ptr repository_r );

    virtual TranslatedText summary() const;
    virtual TranslatedText description() const;
    virtual TranslatedText insnotify() const;
    virtual TranslatedText delnotify() const;
    virtual TranslatedText licenseToConfirm() const;
    virtual Vendor vendor() const;
    virtual ByteCount size() const;
    virtual bool installOnly() const;
    virtual Date buildtime() const;
    virtual Date installtime() const;

    // SCRIPT
    virtual std::string doScriptInlined() const;
    virtual OnMediaLocation doScriptLocation() const;
    virtual std::string undoScriptInlined() const;
    virtual OnMediaLocation undoScriptLocation() const;

    virtual Repository repository() const;

  private:
    repo::cached::RepoImpl::Ptr _repository;
    data::RecordId _id;
  };
  /////////////////////////////////////////////////////////////////
} // namespace cached
} // namespace repository
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H
