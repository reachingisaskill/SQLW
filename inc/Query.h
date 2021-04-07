
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

    public:
      // Implement the "BasicLockable" interface
      // Typedef a lock to interface with the mutex
      typedef std::unique_lock< Query > LockType;

      // Typedef the iterators
      typedef ParameterVector::iterator ParameterIterator;
      typedef ParameterVector::iterator ColumnIterator;


    private:
      // Store a pointer to the database so we can check for errors
      Connection& _connection;

      // Internal connection lock
      std::unique_lock<std::mutex> _connectionLock;

      // Serialize access to this query
      std::mutex _theMutex;

      // The statement pointer
      sqlite3_stmt* _theStatement;

      // The name of the query
      const std::string _name;

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


      // Prepare the query. Loads the parameters into the statement and locks the database connection
      void prepare();

      // Steps the query so that the columns can be read on each execution.
      // Returns true while the stepping may continue. Returns false to stop.
      // Error flag must be checked separately. If error is set false is guarenteed to be returned.
      bool step();

      // Clean up the query and release the database connection. Must be called even if an error occurs!
      void reset();


      // Interface for checking errors during processing
      // Return's true if an error is present after the last usage
      bool error() const { return _error != nullptr; }

      // Return the current error
      const char* getError() const { return _error ? _error : ""; }


      // Flags to return if the query has/expects columns/parameters
      bool hasParameters() const { return ! _parameters.empty(); }
      bool hasColumns() const { return ! _columns.empty(); }

      // Flags to return if the query has/expects columns/parameters
      size_t countParameters() const { return _parameters.size(); }
      size_t countColumns() const { return _columns.size(); }


      // Interfaces for the params and columns

      // Return a specific parameter
      Parameter& getParameter( size_t n ) { return _parameters[ n ]; }
      const Parameter& getParameter( size_t n ) const { return _parameters[ n ]; }

      // Return a specific column
      Parameter& getColumn( size_t n ) { return _columns[ n ]; }
      const Parameter& getColumn( size_t n ) const { return _columns[ n ]; }

      // Start and end of parameters
      ParameterIterator parametersBegin() { return _parameters.begin(); }
      ParameterIterator parametersEnd() { return _parameters.end(); }

      // Start and end of columns
      ColumnIterator columnsBegin() { return _columns.begin(); }
      ColumnIterator columnsEnd() { return _columns.end(); }


      // Create and return a lock for this query
      LockType acquire() { return LockType( *this ); }

      // Lock the internal mutex
      void lock();

      // Unlock the internal mutex
      void unlock();
  };

}

#endif // SQLW_QUERY_BASE_H_

