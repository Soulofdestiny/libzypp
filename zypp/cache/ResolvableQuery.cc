#include <iterator>
#include <algorithm>
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Logger.h"
#include "zypp/cache/CacheTypes.h"
#include "zypp/cache/ResolvableQuery.h"
#include "zypp/Package.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

using namespace sqlite3x;
using namespace std;
using namespace zypp;

typedef shared_ptr<sqlite3_command> sqlite3_command_ptr;

namespace zypp { namespace cache {

struct ResolvableQuery::Impl
{
  Pathname _dbdir;
  string _fields;
  CacheTypes _type_cache;
  sqlite3_connection _con;
  sqlite3_command_ptr _cmd_attr_str;
  sqlite3_command_ptr _cmd_attr_tstr;
  sqlite3_command_ptr _cmd_attr_num;
  sqlite3_command_ptr _cmd_disk_usage;
  sqlite3_command_ptr _cmd_shared_id;

  Impl( const Pathname &dbdir)
  : _dbdir(dbdir)
  , _type_cache(dbdir)
  {
    _con.open((dbdir + "zypp.db").asString().c_str());
    _con.executenonquery("PRAGMA cache_size=8000;");

    _cmd_shared_id.reset( new sqlite3_command( _con, "select shared_id from resolvables where id=:rid;") );

    _cmd_attr_tstr.reset( new sqlite3_command( _con, "select a.text, l.name from text_attributes a,types l,types t where a.weak_resolvable_id=:rid and a.lang_id=l.id and a.attr_id=t.id and l.class=:lclass and t.class=:tclass and t.name=:tname;") );


    _cmd_attr_str.reset( new sqlite3_command( _con, "select a.text from text_attributes a,types l,types t where a.weak_resolvable_id=:rid and a.lang_id=l.id and a.attr_id=t.id and l.class=:lclass and l.name=:lname and t.class=:tclass and t.name=:tname;"));

    _cmd_attr_num.reset( new sqlite3_command( _con, "select a.value from numeric_attributes a,types t where a.weak_resolvable_id=:rid and a.attr_id=t.id and t.class=:tclass and t.name=:tname;"));

    _cmd_disk_usage.reset( new sqlite3_command( _con, "select d.name,du.size,du.files from resolvable_disk_usage du,dir_names d where du.resolvable_id=:rid and du.dir_name_id=d.id;"));
    
    MIL << "Creating Resolvable query impl" << endl;
    //         0   1     2        3        4      5     6     7               8             9             10          11            12
    _fields = "id, name, version, release, epoch, arch, kind, installed_size, archive_size, install_only, build_time, install_time, repository_id";
  }

  ~Impl()
  {
      MIL << "Destroying Resolvable query impl" << endl;
  }

  //
  // convert regex ? and * operators to sql _ and % respectively
  //  example: regex2sql( "*foo?bar*" ) => "%foo_bar%"
  // FIXME: take care of ".*" and "."
  std::string regex2sql( const std::string & s)
  {
    std::string sql( s );
    string::iterator it;
    for (it = sql.begin(); it != sql.end(); ++it)
    {
      if (*it == '*') *it = '%';
      else if (*it == '?') *it = '_';
    }
    return sql;
  }

  data::ResObject_Ptr fromRow( sqlite3_reader &reader )
  {
    data::ResObject_Ptr ptr (new data::ResObject);

    // see _fields definition above for the getXXX() numbers

    ptr->name = reader.getstring(1);
    ptr->edition = Edition( reader.getstring(2), reader.getstring(3), reader.getint(4));
    ptr->arch = _type_cache.archFor(reader.getint(5));
    ptr->kind = _type_cache.kindFor( reader.getint(6) );
    ptr->repository = reader.getint( 12 );

    // TODO get the rest of the data

    return ptr;
  }

  
  void query( const data::RecordId &id,
                  ProcessResolvable fnc )
  {
    sqlite3_command cmd( _con, "select " + _fields + " from resolvables where id=:id;");
    cmd.bind(":id", id);
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      fnc( id, fromRow(reader) );
    }
  }


  void query( const std::string &s,
              ProcessResolvable fnc  )
  {  
    
    sqlite3_command cmd( _con, "select " + _fields + " from resolvables where name like :name;");
    cmd.bind( ":name", regex2sql( s ) );
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      fnc( reader.getint64(0), fromRow(reader) );
    }
  }


  std::string queryStringAttribute( const data::RecordId &record_id,
                                    const std::string &klass,
                                    const std::string &name,
                                    const std::string &default_value )
  {
    string value;
    return queryStringAttributeTranslationInternal( _con, record_id, Locale(), klass, name, default_value);
  }


  std::string queryStringAttributeTranslation( const data::RecordId &record_id,
                                               const Locale &locale,
                                               const std::string &klass,
                                               const std::string &name,
                                               const std::string &default_value )
  {
    return queryStringAttributeTranslationInternal( _con, record_id, locale, klass, name, default_value );
  }


  TranslatedText queryTranslatedStringAttribute( const data::RecordId &record_id,
                                                 const std::string &klass,
                                                 const std::string &name,
                                                 const TranslatedText &default_value )
  {
    return queryTranslatedStringAttributeInternal( _con, record_id, klass, name, default_value );
  }


  bool queryBooleanAttribute( const data::RecordId &record_id,
                              const std::string &klass,
                              const std::string &name,
                              bool default_value )
  {
    return ( queryNumericAttributeInternal( _con, record_id, klass, name, default_value) > 0 );
  }
      
  int queryNumericAttribute( const data::RecordId &record_id,
                             const std::string &klass,
                             const std::string &name,
                             int default_value )
  {
    return queryNumericAttributeInternal( _con, record_id, klass, name, default_value);
  }

  void queryDiskUsage( const data::RecordId &record_id, DiskUsage &du )
  {
    _cmd_disk_usage->bind(":rid", record_id);
    sqlite3_reader reader = _cmd_disk_usage->executereader();

    while ( reader.read() )
    {
      DiskUsage::Entry entry(reader.getstring(0),
                             reader.getint(1),
                             reader.getint(2) );
      du.add(entry);
    }
  }

  std::string queryRepositoryAlias( const data::RecordId &repo_id )
  {
    std::string alias;
    sqlite3_command cmd( _con, "select alias from repositories where id=:id;" );
    cmd.bind( ":id", repo_id );
    sqlite3_reader reader = cmd.executereader();
    while( reader.read() )
    {
      alias = reader.getstring( 0 );
      break;
    }
    return alias;
  }
  
  data::RecordId queryRepositoryId( const std::string &repo_alias )
  {
    long long id = 0;
    sqlite3_command cmd( _con, "select id from repositories where alias=:alias;" );
    cmd.bind( ":alias", repo_alias );
    sqlite3_reader reader = cmd.executereader();
    while( reader.read() )
    {
      id = reader.getint64(0);
      break;
    }
    return id;
  }
  
  void iterateResolvablesByKindsAndStringsAndRepos( const std::vector<zypp::Resolvable::Kind> & kinds,
                  const std::vector<std::string> &strings, int flags, const std::vector<std::string> repos, ProcessResolvable fnc )
  {
    std::string sqlcmd( "SELECT " + _fields + " FROM resolvables" );

    std::vector<std::string>::const_iterator it_s;
    for (it_s = strings.begin(); it_s != strings.end(); ++it_s)
    {
      std::string s( *it_s );

      if (it_s == strings.begin())
        sqlcmd += " WHERE (";
      else
        sqlcmd += " AND ";

//FIXME: Implement MATCH_RESSUMM and MATCH_RESDESC

      sqlcmd += " name ";
      if ((flags & MATCH_WILDCARDS) == 0)
      {
        sqlcmd += "=";
      }
      else
      {
        sqlcmd += "like";
        s = regex2sql( s );
      }
      if (flags & MATCH_LEADING)
        s += "%";
      if (flags & MATCH_TRAILING)
        s = string("%") + s;

      sqlcmd += " '";
      sqlcmd += s;
      sqlcmd += "'";
    }

    if (it_s != strings.begin())
    {
      sqlcmd += ")";
    }

    std::vector<zypp::Resolvable::Kind>::const_iterator it_k;
    if (!kinds.empty())
    {
      if (it_s == strings.begin())
        sqlcmd += " WHERE";
      else
        sqlcmd += " AND";

      for (it_k = kinds.begin(); it_k != kinds.end(); ++it_k)
      {
        if (it_k == kinds.begin())
	  sqlcmd += " kind IN (";
	else
          sqlcmd += ", ";

        char idbuf[16];
        snprintf( idbuf, 15, "%d", (int)(_type_cache.idForKind( *it_k )) );
        sqlcmd += idbuf;
      }

      if (it_k != kinds.begin())
      {
        sqlcmd += ")";
      }
    }

    std::vector<std::string>::const_iterator it_r;
    if (!repos.empty())
    {
      if (it_s == strings.begin()
	  && it_k == kinds.begin())
        sqlcmd += " WHERE";
      else
        sqlcmd += " AND";

      for (it_r = repos.begin(); it_r != repos.end(); ++it_r)
      {
        if (it_r == repos.begin())
	  sqlcmd += " (";
	else
          sqlcmd += " OR ";

        sqlcmd += "repository_id = ";
        char idbuf[16];
        snprintf( idbuf, 15, "%ld", (long)(queryRepositoryId( *it_r )) );
        sqlcmd += idbuf;
      }

      if (it_r != repos.begin())
      {
        sqlcmd += ")";
      }
    }

MIL << "sqlcmd " << sqlcmd << endl;
    sqlite3_command cmd( _con, sqlcmd );
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      fnc( reader.getint64(0), fromRow(reader) );
    }
  }

private:

  int queryNumericAttributeInternal( sqlite3_connection &con,
                                     const data::RecordId &record_id,
                                     const std::string &klass,
                                     const std::string &name,
                                     int default_value )
  {
    //con.executenonquery("BEGIN;");
    _cmd_attr_num->bind(":rid", record_id);

    _cmd_attr_num->bind(":tclass", klass);
    _cmd_attr_num->bind(":tname", name);

    sqlite3_reader reader = _cmd_attr_num->executereader();
    if ( reader.read() )
      return reader.getint(0);
    else
    {
      reader.close();
      sqlite3_reader idreader = _cmd_shared_id->executereader();
      if ( idreader.read() )
      {
        _cmd_shared_id->bind(":rid", record_id);
        data::RecordId sid = idreader.getint(0);
        idreader.close();
        return queryNumericAttributeInternal(con, sid, klass, name, default_value);
      }
    }

    return default_value;
  }
  
  TranslatedText queryTranslatedStringAttributeInternal( sqlite3_connection &con,
                                                         const data::RecordId &record_id,
                                                         const std::string &klass,
                                                         const std::string &name,
                                                         const TranslatedText &default_value )
  {
    //con.executenonquery("PRAGMA cache_size=8000;");
    //con.executenonquery("BEGIN;");

    _cmd_attr_tstr->bind(":rid", record_id);
    _cmd_attr_tstr->bind(":lclass", "lang");

    _cmd_attr_tstr->bind(":tclass", klass);
    _cmd_attr_tstr->bind(":tname", name);

    TranslatedText result;
    sqlite3_reader reader = _cmd_attr_tstr->executereader();
    
    int c = 0;
    while(reader.read())
    {
      result.setText( reader.getstring(0), Locale( reader.getstring(1) ) );
      c++;
    }

    if ( c>0 )
      return result;
    else
    {
      reader.close();
      _cmd_shared_id->bind(":rid", record_id);
      sqlite3_reader idreader = _cmd_shared_id->executereader();
      if ( idreader.read() )
      {
        data::RecordId sid = idreader.getint(0);
        idreader.close();
        return queryTranslatedStringAttributeInternal(con, sid, klass, name, default_value);
      }
    }

    return default_value;
  }

  std::string queryStringAttributeInternal( sqlite3_connection &con,
                                            const data::RecordId &record_id,
                                            const std::string &klass,
                                            const std::string &name,
                                            const std::string &default_value )
  {
    return queryStringAttributeTranslationInternal( con, record_id, Locale(), klass, name, default_value );
  }

  std::string queryStringAttributeTranslationInternal( sqlite3_connection &con,
                                                       const data::RecordId &record_id,
                                                       const Locale &locale,
                                                       const std::string &klass,
                                                       const std::string &name,
                                                        const std::string &default_value )
  {
    //con.executenonquery("BEGIN;");
    _cmd_attr_str->bind(":rid", record_id);
    _cmd_attr_str->bind(":lclass", "lang");
    if (locale == Locale() )
      _cmd_attr_str->bind(":lname", "none");
    else
      _cmd_attr_str->bind(":lname", locale.code());

    _cmd_attr_str->bind(":tclass", klass);
    _cmd_attr_str->bind(":tname", name);

    sqlite3_reader reader = _cmd_attr_str->executereader();
    
    if ( reader.read() )
      return reader.getstring(0);
    else
    {
      reader.close();
      _cmd_shared_id->bind(":rid", record_id);
      sqlite3_reader idreader = _cmd_shared_id->executereader();
      if ( idreader.read() )
      {
        data::RecordId sid = idreader.getint(0);
        idreader.close();
        return queryStringAttributeTranslationInternal( con, sid, locale, klass, name, default_value );
      }
    }

    return default_value;
  }
};

//////////////////////////////////////////////////////////////////////////////
// FORWARD TO IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////

ResolvableQuery::ResolvableQuery( const Pathname &dbdir)
  : _pimpl(new Impl(dbdir))
{
  //MIL << "Creating Resolvable query" << endl;
}

ResolvableQuery::~ResolvableQuery()
{
  //MIL << "Destroying Resolvable query" << endl;
}

//////////////////////////////////////////////////////////////////////////////

void ResolvableQuery::query( const data::RecordId &id, ProcessResolvable fnc  )
{
  _pimpl->query(id, fnc);
}

//////////////////////////////////////////////////////////////////////////////

void ResolvableQuery::query( const std::string &s, ProcessResolvable fnc  )
{
  _pimpl->query(s, fnc);
}

//////////////////////////////////////////////////////////////////////////////

int ResolvableQuery::queryNumericAttribute( const data::RecordId &record_id,
                                            const std::string &klass,
                                            const std::string &name,
                                            int default_value )
{
  return _pimpl->queryNumericAttribute(record_id, klass, name, default_value);
}

bool ResolvableQuery::queryBooleanAttribute( const data::RecordId &record_id,
                                             const std::string &klass,
                                             const std::string &name,
                                             bool default_value )
{
  return _pimpl->queryNumericAttribute(record_id, klass, name, default_value);
}


std::string ResolvableQuery::queryStringAttribute( const data::RecordId &record_id,
                                                   const std::string &klass,
                                                   const std::string &name,
                                                   const std::string &default_value )
{
  return _pimpl->queryStringAttribute(record_id, klass, name, default_value);
}

//////////////////////////////////////////////////////////////////////////////

std::string ResolvableQuery::queryStringAttributeTranslation( const data::RecordId &record_id,
                                                              const Locale &locale,
                                                              const std::string &klass,
                                                              const std::string &name,
                                                              const std::string &default_value )
{
  return _pimpl->queryStringAttributeTranslation(record_id, locale, klass, name, default_value );
}

//////////////////////////////////////////////////////////////////////////////

TranslatedText ResolvableQuery::queryTranslatedStringAttribute( const data::RecordId &record_id,
                                                                const std::string &klass,
                                                                const std::string &name,
                                                                const TranslatedText &default_value )
{
  return _pimpl->queryTranslatedStringAttribute(record_id, klass, name, default_value );
}

void ResolvableQuery::queryDiskUsage( const data::RecordId &record_id, DiskUsage &du )
{
  _pimpl->queryDiskUsage(record_id, du);
}

std::string ResolvableQuery::queryRepositoryAlias( const data::RecordId &repo_id )
{
  return _pimpl->queryRepositoryAlias( repo_id );
}

void ResolvableQuery::iterateResolvablesByKindsAndStringsAndRepos( const std::vector<zypp::Resolvable::Kind> & kinds,
                  const std::vector<std::string> &strings, int flags, const std::vector<std::string> &repos, ProcessResolvable fnc )
{
  _pimpl->iterateResolvablesByKindsAndStringsAndRepos( kinds, strings, flags, repos, fnc );
}
//////////////////////////////////////////////////////////////////////////////

} } // namespace zypp::cache