
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


  // Struct to hold the database pointer and its associated mutex
  struct Connection
  {
    sqlite3* database;
    std::mutex mutex;
  };


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
      Connection _connection;

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



      // Run the query name parsing JSON data in and out
      rapidjson::Document executeJson( const char*, const rapidjson::Document& );

      // Run the query name parsing JSON data out only. Will fail if parameters are required
      rapidjson::Document executeJson( const char* );

  };

}

#endif // SQLW_DATABASE_H_

