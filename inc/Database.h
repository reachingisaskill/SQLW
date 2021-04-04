
#ifndef SQLW_DATABASE_H_
#define SQLW_DATABASE_H_

#include "sqlite3.h"
#include "rapidjson/document.h"
#include "CON.h"

#include <unordered_map>
#include <mutex>


namespace SQLW
{
  // Forward declare the Query class
  class Query;


  /*
   * Wrapper to the Sqlite3 database interface
   */
  class Database
  {
    // Allow the query class to access some private functions
    friend class Query;

    typedef std::unordered_map< std::string, Query* > QueryMap;

    private:
      // Name of the file
      std::string _filename;

      // Number of times to retry a busy connection
      unsigned int _busyRetries;

      // Database handle
      sqlite3* _database;

      // Serialize access to the database through this mutex
      std::mutex _theMutex;

      // Hash-map of the queries implmented for this database
      QueryMap _queries;


    public:
      // Open the database connection using the provided configuration
      Database( const CON::Object& );

      // Not copy constructable/assignable
      Database( const Database& ) = delete;
      Database& operator=( const Database& ) = delete;

      // Use the default move constructor/assignment
      Database( Database&& ) = default;
      Database& operator=( Database&& ) = default;

      // Clean up
      ~Database();



      // Executes a statement using the correct method base on the provided json document
      rapidjson::Document executeOperation( const char*, const rapidjson::Document& );



      // Implement the "BasicLockable" interface
      // Lock the database mutex
      void lock() { _theMutex.lock(); }

      // Unlock the database mutex
      void unlock() { _theMutex.unlock(); }



      // Define the lock_guard style lock for the database
      typedef std::unique_lock< Database > DatabaseLock;

      // Return a lock on this database
      DatabaseLock acquire() { return DatabaseLock( *this ); }
  };

}

#endif // SQLW_DATABASE_H_

