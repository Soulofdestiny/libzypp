/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCurl.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIACURL_H
#define ZYPP_MEDIA_MEDIACURL_H

#include "zypp/base/Flags.h"
#include "zypp/media/TransferSettings.h"
#include "zypp/media/MediaHandler.h"
#include "zypp/ZYppCallbacks.h"

#include <curl/curl.h>

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaCurl
/**
 * @short Implementation class for FTP, HTTP and HTTPS MediaHandler
 * @see MediaHandler
 **/
class MediaCurl : public MediaHandler
{
  public:
    enum RequestOption
    {
        /** Defaults */
        OPTION_NONE = 0x0,
        /** retrieve only a range of the file */
        OPTION_RANGE = 0x1,
        /** only issue a HEAD (or equivalent) request */
        OPTION_HEAD = 0x02,
    };
    ZYPP_DECLARE_FLAGS(RequestOptions,RequestOption);

  protected:

    virtual void attachTo (bool next = false);
    virtual void releaseFrom( const std::string & ejectDev );
    virtual void getFile( const Pathname & filename ) const;
    virtual void getDir( const Pathname & dirname, bool recurse_r ) const;
    virtual void getDirInfo( std::list<std::string> & retlist,
                             const Pathname & dirname, bool dots = true ) const;
    virtual void getDirInfo( filesystem::DirContent & retlist,
                             const Pathname & dirname, bool dots = true ) const;
    /**
     * Repeatedly calls doGetDoesFileExist() until it successfully returns,
     * fails unexpectedly, or user cancels the operation. This is used to
     * handle authentication or similar retry scenarios on media level.
     */
    virtual bool getDoesFileExist( const Pathname & filename ) const;

    /**
     * \see MediaHandler::getDoesFileExist
     */
    virtual bool doGetDoesFileExist( const Pathname & filename ) const;

    /**
     *
     * \throws MediaException
     *
     */
    virtual void disconnectFrom();
    /**
     *
     * \throws MediaException
     *
     */
    virtual void getFileCopy( const Pathname & srcFilename, const Pathname & targetFilename) const;

    /**
     *
     * \throws MediaException
     *
     */
    virtual void doGetFileCopy( const Pathname & srcFilename, const Pathname & targetFilename, callback::SendReport<DownloadProgressReport> & _report, RequestOptions options = OPTION_NONE ) const;


    virtual bool checkAttachPoint(const Pathname &apoint) const;

  public:

    MediaCurl( const Url &      url_r,
	       const Pathname & attach_point_hint_r );

    virtual ~MediaCurl() { try { release(); } catch(...) {} }

    static void setCookieFile( const Pathname & );

    class Callbacks
    {
      public:
	virtual ~Callbacks() {}
        virtual bool progress( int percent ) = 0;
    };

  protected:

    static int progressCallback( void *clientp, double dltotal, double dlnow,
                                 double ultotal, double ulnow );
  private:
    /**
     * Return a comma separated list of available authentication methods
     * supported by server.
     */
    std::string getAuthHint() const;

    bool authenticate(const std::string & availAuthTypes, bool firstTry) const;

  private:
    CURL *_curl;
    char _curlError[ CURL_ERROR_SIZE ];
    long _curlDebug;
    curl_slist *_customHeaders;

    /*
    mutable std::string _userpwd;
    std::string _proxy;
    std::string _proxyuserpwd;
    */
    std::string _currentCookieFile;
    //std::string _ca_path;
    //long        _xfer_timeout;

    static Pathname _cookieFile;
protected:
    TransferSettings _settings;
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS(MediaCurl::RequestOptions);

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIACURL_H
