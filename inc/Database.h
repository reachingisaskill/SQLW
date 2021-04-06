
#ifndef SQLW_DATABASE_H_
#define SQLW_DATABASE_H_

#include "sqlite3.h"
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

#if defined RAPIDJSON_VERSION_STRING
    // Json wrapper interface.
    friend rapidjson::Document executeJson( Database&, const char*, const rapidjson::Document& );
#endif

    // Container to store the queries
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



      // Return a reference to a stored query for access to the manual interface
      // References are valid for the lifetime of the database object.
      Query& requestQuery( const char* );


      // Return true if the query exists. For runtime assertion that the configuration was loaded correctly
      bool queryExists( const char* ) const;

  };


////////////////////////////////////////////////////////////////////////////////
  // Optional functions for different libraries

#if defined RAPIDJSON_VERSION_STRING

  // Run the query name parsing JSON data in and out
  rapidjson::Document executeJson( Database&, const char*, const rapidjson::Document& );

#endif

}

#endif // SQLW_DATABASE_H_

