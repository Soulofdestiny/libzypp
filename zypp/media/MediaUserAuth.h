/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaUserAuth.h
 * Convenience interface for handling authentication data of media user.
 */
#ifndef ZYPP_MEDIA_USER_AUTH_H
#define ZYPP_MEDIA_USER_AUTH_H

#include <curl/curl.h>

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////


/**
 * Class for handling media authentication data. This is the most generic
 * class containing only username and password members.
 */
class AuthData
{
public:
  AuthData() : _username(), _password()
  {}

  AuthData(std::string & username, std::string & password)
    : _username(username), _password(password)
  {}

  virtual ~AuthData() {};

  /**
   * Checks validity of authentication data.
   * \return true if the object contains non-empty username and
   *  non-empty password, false otherwise.  
   */
  virtual bool valid() const;

  void setUserName(std::string username) { _username = username; }
  void setPassword(std::string password) { _password = password; }  

  std::string username() const { return _username; }
  std::string password() const { return _password; } 

  virtual std::ostream & dumpOn( std::ostream & str ) const;


private:
  std::string _username;
  std::string _password;
};

/**
 * Curl HTTP authentication data.
 */
class CurlAuthData : public AuthData {
public:
  /**
   * Default constructor. Initializes username and password to empty strings
   * and authetication type to CURLAUTH_NONE.
   */
  CurlAuthData() : AuthData(), _auth_type_str(), _auth_type(CURLAUTH_NONE)
  {}

  CurlAuthData(std::string & username, std::string & password, std::string & auth_type)
    : AuthData(username,password), _auth_type_str(auth_type)
  {
    _auth_type = auth_type_str2long(auth_type);
  }

  CurlAuthData(std::string & username, std::string & password, long auth_type)
    : AuthData(username,password), _auth_type(auth_type)
  {
    _auth_type_str = auth_type_long2str(auth_type);
  }

  /**
   * Checks validity of authentication data.
   * \return true if the object contains non-empty username,
   *  non-empty password, and specifies authentication type; false otherwise.  
   */
  virtual bool valid() const;

  /**
   * Set HTTP authentication type(s) to use.
   * \param comma separated list of HTTP authentication type names
   */
  void setAuthType(std::string auth_type)
  {
    _auth_type_str = auth_type; _auth_type = auth_type_str2long(auth_type);
  }

  /*
   * Set HTTP authentication type(s) to use.
   * \param HTTP authentication type as in long ORed form.
   * \see curl.h for available auth types
   */
  void setAuthType(long auth_type)
  {
    _auth_type = auth_type;
    _auth_type_str = auth_type_long2str(auth_type);
  }

  long authType() { return _auth_type; } const
  std::string authTypeAsString() { return _auth_type_str; } const

  std::string getUserPwd() const { return username() + ":" + password(); } 


  /**
   * Converts a string of comma separated list of authetication type names
   * into a long of ORed CURLAUTH_* identifiers.
   * The method also automatically leaves out any auth types declared
   * not supported by curl_version_info().
   * 
   * \throws MediaException if an invalid authentication type name is
   *         encountered.
   */
  static long auth_type_str2long(std::string & auth_type_str);

  /**
   * Converts a long of ORed CURLAUTH_* identifiers into a string of comma
   * separated list of authentication type names.
   */
  static std::string auth_type_long2str(long auth_type);

  virtual std::ostream & dumpOn( std::ostream & str ) const;

private:
  std::string _auth_type_str;
  long _auth_type;
};


std::ostream & operator << (std::ostream & str, AuthData & auth_data);
std::ostream & operator << (std::ostream & str, CurlAuthData & auth_data);

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_USER_AUTH_H