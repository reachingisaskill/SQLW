
#ifndef SQLW_QUERY_BASE_H_
#define SQLW_QUERY_BASE_H_

#include "Parameter.h"

#include "sqlite3.h"
#include "CON.h"
#include "rapidjson/document.h"

#include <vector>
#include <mutex>


namespace SQLW
{
  struct Connection;

  class Query
  {

    // The parameter list type
    typedef std::vector< Parameter > ParameterVector;

    private:
      // Store a pointer to the database so we can check for errors
      Connection& _connection;

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
      ParameterVector _parameters;

      // The returned columns
      ParameterVector _columns;


    public:
      // Database connection, name, description, statement
      Query( Connection&, const CON::Object& );

      // Destroy the statement
      ~Query();

      // Lazy so just make these deleted
      Query( const Query& ) = delete;
      Query( Query&& ) = delete;
      Query& operator=( const Query& ) = delete;
      Query& operator=( Query&& ) = delete;



      // Runs the query, calling the derived class functions to handle specific operations
      rapidjson::Document run( const rapidjson::Document& );


      // Flags to return if the query has/expects columns/parameters
      bool hasParameters() const { return ! _parameters.empty(); }
      bool hasColumns() const { return ! _columns.empty(); }


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
      void unlock() { _theMutex.unlock(); }
  };

}

#endif // SQLW_QUERY_BASE_H_

