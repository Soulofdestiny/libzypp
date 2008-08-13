/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/RepoindexFileReader.cc
 * Implementation of repoindex.xml file reader.
 */
#include <iostream>

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"

#include "zypp/RepoInfo.h"

#include "zypp/Pathname.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/ParseException.h"

#include "zypp/parser/RepoindexFileReader.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {


  ///////////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : RepoindexFileReader::Impl
  //
  class RepoindexFileReader::Impl : private base::NonCopyable
  {
  public:
    /**
     * CTOR
     *
     * \see RepoindexFileReader::RepoindexFileReader(Pathname,ProcessResource)
     */
    Impl(const Pathname &repoindex_file, const ProcessResource & callback);

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode( Reader & reader_r );


  private:
    /** Function for processing collected data. Passed-in through constructor. */
    ProcessResource _callback;
  };
  ///////////////////////////////////////////////////////////////////////

  RepoindexFileReader::Impl::Impl(
      const Pathname &repoindex_file, const ProcessResource & callback)
    : _callback(callback)
  {
    Reader reader( repoindex_file );
    MIL << "Reading " << repoindex_file << endl;
    reader.foreachNode( bind( &RepoindexFileReader::Impl::consumeNode, this, _1 ) );
  }

  // --------------------------------------------------------------------------

  /*
   * xpath and multiplicity of processed nodes are included in the code
   * for convenience:
   *
   * // xpath: <xpath> (?|*|+)
   *
   * if multiplicity is ommited, then the node has multiplicity 'one'.
   */

  // --------------------------------------------------------------------------

  bool RepoindexFileReader::Impl::consumeNode( Reader & reader_r )
  {
    if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT )
    {
      // xpath: /repoindex
      if ( reader_r->name() == "repoindex" )
      {
        return true;
      }

      // xpath: /repoindex/data (+)
      if ( reader_r->name() == "repo" )
      {
        XmlString s;
        RepoInfo info;

        // url and/or path
        string url_s;
        s = reader_r->getAttribute("url");
        if (s.get())
          url_s = s.asString();
        string path_s;
        s = reader_r->getAttribute("path");
        if (s.get())
          path_s = s.asString();

        if (url_s.empty() && path_s.empty())
          throw ParseException(str::form(_("One or both of '%s' or '%s' attributes is required."), "url", "path"));
        //! \todo FIXME this hardcodes the "/repo/" fragment - should not be if we want it to be usable by others!
        else if (url_s.empty())
          info.setPath(Pathname(string("/repo/") + path_s));
        else if (path_s.empty())
          info.setBaseUrl(Url(url_s));
        else
          info.setBaseUrl(Url(url_s + "/repo/" + path_s));

        // required alias
        info.setAlias(reader_r->getAttribute("alias").asString());

        // optional type
        s = reader_r->getAttribute("type");
        if (s.get())
          info.setType(repo::RepoType(s.asString()));

        // optional name
        s = reader_r->getAttribute("name");
        if (s.get())
          info.setName(s.asString());

        DBG << info << endl;

        // ignore the rest
        _callback(info);
        return true;
      }
    }

    return true;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : RepoindexFileReader
  //
  ///////////////////////////////////////////////////////////////////

  RepoindexFileReader::RepoindexFileReader(
      const Pathname & repoindex_file, const ProcessResource & callback)
    :
      _pimpl(new Impl(repoindex_file, callback))
  {}

  RepoindexFileReader::~RepoindexFileReader()
  {}


  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai: