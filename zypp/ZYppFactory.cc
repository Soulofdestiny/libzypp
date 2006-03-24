/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppFactory.cc
 *
*/

#include <sys/file.h>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"

#include "zypp/ZYppFactory.h"
#include "zypp/zypp_detail/ZYppImpl.h"

#define ZYPP_LOCK_FILE "/var/run/zypp.pid"

using std::endl;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppFactory
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYppFactory::instance
  //	METHOD TYPE : ZYppFactory
  //
  ZYppFactory ZYppFactory::instance()
  {
    return ZYppFactory();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYppFactory::ZYppFactory
  //	METHOD TYPE : Ctor
  //
  ZYppFactory::ZYppFactory()
  {

  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYppFactory::~ZYppFactory
  //	METHOD TYPE : Dtor
  //
  ZYppFactory::~ZYppFactory()
  {}

  class ZYppGlobalLock
  {
    public:
      
    ZYppGlobalLock() : _zypp_lockfile(0)
    {
    
    }
      
    private:
    FILE *_zypp_lockfile;
    
    void openLockFile(const char *mode)
    {
      Pathname lock_file = Pathname(ZYPP_LOCK_FILE);
      _zypp_lockfile = fopen(lock_file.asString().c_str(), mode);
      if (_zypp_lockfile == 0)
        ZYPP_THROW (Exception( "Cant open " + lock_file.asString() ) );
    }
    
    void closeLockFile()
    {
      fclose(_zypp_lockfile);
    }
    
    void shLockFile()
    {
      int fd = fileno(_zypp_lockfile);
      int lock_error = flock(fd, LOCK_SH);
      if (lock_error != 0)
        ZYPP_THROW (Exception( "Cant get shared lock"));
      else
        MIL << "locked (shared)" << std::endl;
    }
        
    void exLockFile()
    {
      int fd = fileno(_zypp_lockfile);
      // lock access to the file
      int lock_error = flock(fd, LOCK_EX);
      if (lock_error != 0)
        ZYPP_THROW (Exception( "Cant get exclusive lock" ));
      else
        MIL << "locked (exclusive)" << std::endl;
    }
    
    void unLockFile()
    {
      int fd = fileno(_zypp_lockfile);
      // lock access to the file
      int lock_error = flock(fd, LOCK_UN);
      if (lock_error != 0)
        ZYPP_THROW (Exception( "Cant release lock" ));
      else
        MIL << "unlocked" << std::endl;
    }
    
    bool lockFileExists()
    {
      Pathname lock_file = Pathname(ZYPP_LOCK_FILE);
      // check if the file already existed.
      bool exists = PathInfo(lock_file).isExist();
      return exists;
    }
    
    void createLockFile()
    {
      pid_t curr_pid = getpid();
      openLockFile("w");
      exLockFile();
      fprintf(_zypp_lockfile, "%ld\n", (long) curr_pid);
      fflush(_zypp_lockfile);
      unLockFile();
      MIL << "written lockfile with pid " << curr_pid << std::endl;
      closeLockFile();
    }
    
    bool isProcessRunning(pid_t pid)
    {
      // it is another program, not me, see if it is still running
      stringstream ss;
      ss << "/proc/" << pid << "/status";
      Pathname procfile = Pathname(ss.str());
      MIL << "Checking " << procfile << " to determine if pid is running: " << pid << std::endl;
      bool still_running = PathInfo(procfile).isExist();
      return still_running;
    }
    
    pid_t lockerPid()
    {
      pid_t locked_pid = 0;
      long readpid = 0;
      
      fscanf(_zypp_lockfile, "%ld", &readpid);
      MIL << "read: Lockfile " << ZYPP_LOCK_FILE << " has pid " << readpid << std::endl;
      locked_pid = (pid_t) readpid;
      return locked_pid;
    }
    
    public:
    
    bool zyppLocked()
    {
      pid_t curr_pid = getpid();
      Pathname lock_file = Pathname(ZYPP_LOCK_FILE);
      
      if ( lockFileExists() )
      {
        openLockFile("r");
        shLockFile();
        
        pid_t locker_pid = lockerPid();
        if ( locker_pid == curr_pid )
        {
          // alles ok, we are requesting the instance again
          MIL << "Lockfile found, but it is myself. Assuming same process getting zypp instance again." << std::endl;
          return false;
        }
        else
        {
          if ( isProcessRunning(locker_pid) )
          {
            MIL << locker_pid << " is running and has a ZYpp lock. Sorry" << std::endl;
            return true;
          }
          else
          {
            MIL << locker_pid << " has a ZYpp lock, but process is not running. Cleaning lock file." << std::endl;
            if ( filesystem::unlink(lock_file) == 0 )
            {
              createLockFile();
              // now open it for reading
              openLockFile("r");
              shLockFile();
              return false;
            }
            else
            {
              ERR << "Can't clean lockfile. Sorry, can't create a new lock. Zypp still locked." << std::endl;
              return true;
            }
          }
        }
      }
      else
      {
        createLockFile();
        // now open it for reading
        openLockFile("r");
        shLockFile();
        return false;
      }
    }
    
    
  };
  
  static ZYppGlobalLock globalLock;
  
  ///////////////////////////////////////////////////////////////////
  //
  ZYpp::Ptr ZYppFactory::getZYpp() const
  {
    static ZYpp::Ptr _instance( new ZYpp( ZYpp::Impl_Ptr(new ZYpp::Impl) ) );
    if ( globalLock.zyppLocked() )
      return 0;
    else
      return _instance;
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ZYppFactory & obj )
  {
    return str << "ZYppFactory";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
