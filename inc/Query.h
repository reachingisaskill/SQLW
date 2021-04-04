
#ifndef SQLW_QUERY_BASE_H_
#define SQLW_QUERY_BASE_H_

#include "sqlite3.h"
#include "CON.h"
#include "rapidjson/document.h"

#include <list>
#include <mutex>


namespace SQLW
{

  class Query
  {
    struct Parameter
    {
      enum Type { Text, Int, Bool, Blob, Double };
      Type type;
      std::string name;
    };

    typedef std::list< Parameter > ParameterList;

    private:
      // Store a pointer to the database so we can check for errors
      sqlite3* _database;

      // Serialize access to this query
      std::mutex _theMutex;

      // The statement pointer
      sqlite3_stmt* _theStatement;

      // Very brief description for debugging
      const std::string _description;

      // The raw text that makes up the statement
      const std::string _statementText;

      // If an error occurs. This is not-null
      const char* _error;

      // The required parameters
      ParameterList _parameters;

      // The returned columns
      ParameterList _columns;


    public:
      // Database connection, name, description, statement
      Query( sqlite3*, const CON::Object& );

      // Destroy the statement
      ~Query();

      // Lazy so just make these deleted
      Query( const Query& ) = delete;
      Query( Query&& ) = delete;
      Query& operator=( const Query& ) = delete;
      Query& operator=( Query&& ) = delete;



      // Runs the query, calling the derived class functions to handle specific operations
      rapidjson::Document run( const rapidjson::Document& );



      // Return's true if an error is present after the last usage
      bool error() const { return _error != nullptr; }

      // Return the current error
      std::string getError() const { return std::string( _error ? _error : "" ); }



      // Implement the "BasicLockable" interface
      // Typedef a lock to interface with the mutex
      typedef std::unique_lock< Query > QueryLock;

      // Create and return a lock for this query
      QueryLock acquire() { return QueryLock( *this ); }

      // Lock the internal mutex
      void lock() { _theMutex.lock(); _error = nullptr; }

      // Unlock the internal mutex
      void unlock() { sqlite3_finalize( _theStatement ); _theMutex.unlock(); }
  };

}

#endif // SQLW_QUERY_BASE_H_

