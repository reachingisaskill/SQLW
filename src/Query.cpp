
#include "Query.h"
#include "Database.h"

#include <thread>


namespace SQLW
{

  Query::Query( Connection& con, const CON::Object& config ) :
    _connection( con ),
    _connectionLock( _connection.mutex, std::defer_lock ),
    _theStatement( nullptr ),
    _description( config["description"].asString() ),
    _statementText( config["statement"].asString() ),
    _error( nullptr )
  {
    int result = sqlite3_prepare_v3( _connection.database, _statementText.c_str(), _statementText.size(), SQLITE_PREPARE_PERSISTENT, &_theStatement, nullptr );

    if ( result != SQLITE_OK || _theStatement == nullptr )
    {
      std::stringstream ss;
      ss << "Fail to prepare query: " << _description << " Error " << result << " : " << sqlite3_errmsg( _connection.database );
      throw std::runtime_error( "Failed to prepare query." );
    } 

    const CON::Object& parameters = config["parameters"];
    for ( size_t i = 0; i < parameters.getSize(); ++i )
    {
      const CON::Object& param = parameters[i];

      if ( param["type"].asString() == "text" )
      {
        _parameters.push_back( Parameter( param["name"].asString(), Parameter::Text ) );
      }
      else if ( param["type"].asString() == "int" ) 
      {
        _parameters.push_back( Parameter( param["name"].asString(), Parameter::Int ) );
      }
      else if ( param["type"].asString() == "bool" ) 
      {
        _parameters.push_back( Parameter( param["name"].asString(), Parameter::Bool ) );
      }
      else if ( param["type"].asString() == "blob" )
      {
        _parameters.push_back( Parameter( param["name"].asString(), Parameter::Blob ) );
      }
      else if ( param["type"].asString() == "double" || param["type"].asString() == "float" )
      {
        _parameters.push_back( Parameter( param["name"].asString(), Parameter::Double ) );
      }
      else
      {
        throw std::runtime_error( "Unknown parameter type." );
      }
    }

    const CON::Object& columns = config["columns"];
    for ( size_t i = 0; i < columns.getSize(); ++i )
    {
      const CON::Object& column = columns[i];

      if ( column["type"].asString() == "text" )
      {
        _columns.push_back( Parameter( column["name"].asString(), Parameter::Text ) );
      }
      else if ( column["type"].asString() == "int" ) 
      {
        _columns.push_back( Parameter( column["name"].asString(), Parameter::Int ) );
      }
      else if ( column["type"].asString() == "bool" ) 
      {
        _columns.push_back( Parameter( column["name"].asString(), Parameter::Bool ) );
      }
      else if ( column["type"].asString() == "blob" )
      {
        _columns.push_back( Parameter( column["name"].asString(), Parameter::Blob ) );
      }
      else if ( column["type"].asString() == "double" || column["type"].asString() == "float" )
      {
        _columns.push_back( Parameter( column["name"].asString(), Parameter::Double ) );
      }
      else
      {
        throw std::runtime_error( "Unknown column type." );
      }
    }
  }


  Query::~Query()
  {
    // Make sure no one else is still using us
    std::lock_guard<std::mutex> lock( _theMutex );

    _parameters.clear();

    _columns.clear();

    if ( _theStatement != nullptr )
      sqlite3_finalize( _theStatement );
  }


//  rapidjson::Document Query::run( const rapidjson::Document& json_data )
//  {
//    // Json data to return
//    rapidjson::Document response( rapidjson::kObjectType );
//    rapidjson::Document::AllocatorType& alloc = response.GetAllocator();
//
//    // Load the parameters
//    size_t index = 1;
//    for ( ParameterVector::iterator p_it = _parameters.begin(); p_it != _parameters.end(); ++p_it, ++index )
//    {
//      // Load and type convert the parameter data
//      if ( ! p_it->setValue( json_data ) )
//      {
//        _error = "Required parameters not found in json documnent.";
//        response.AddMember( "success", false, alloc );
//        response.AddMember( "error", rapidjson::Value( _error, alloc ), alloc );
//        return response;
//      }
//
//      // Give it to the statement
//      p_it->assignStatement( _theStatement, index );
//    }
//
//    // Execute the query
//    int result;
//    bool done = false;
//
//    // Lock the connection mutex - no one else can access the database at the same time
//    std::unique_lock<std::mutex> connectionLock( _connection.mutex );
//
//    while( true )
//    {
//      unsigned count = 0;
//      while ( result = sqlite3_step( _theStatement ), result == SQLITE_BUSY )
//      {
//        if ( count == 10 )
//        {
//          // Set error status
//          _error = "Database busy. Failed to access after repeated retries.";
//          response.AddMember( "success", false, alloc );
//          response.AddMember( "error", rapidjson::Value( _error, alloc ), alloc );
//          sqlite3_reset( _theStatement );
//          return response;
//        }
//        count += 1;
//        std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
//      }
//
//      // Check status for our next option
//      switch( result )
//      {
//        case SQLITE_DONE :
//          done = true;
//          break;
//
//        case SQLITE_ROW :
//          break;
//
//        default:
//          _error = sqlite3_errmsg( _connection.database );
//          response.AddMember( "success", false, alloc );
//          response.AddMember( "error", rapidjson::Value( _error, alloc ), alloc );
//          sqlite3_reset( _theStatement );
//          return response;
//          break;
//      }
//
//      // Quit before the read the statement data - it will be duds
//      if ( done ) break;
//
//      // Fetch the column data
//      index = 0;
//      for ( ParameterVector::iterator c_it = _columns.begin(); c_it != _columns.end(); ++c_it, ++index )
//      {
//        // Load the returned value from the statement
//        c_it->readStatement( _theStatement, index );
//
//        // Write it to the response document
//        c_it->getValue( response );
//      }
//    }
//    // Release the connection mutex
//    connectionLock.unlock();
//
//    // Reset the statement memory
//    sqlite3_reset( _theStatement );
//
//    // Don't forget the success value
//    response.AddMember( "success", true, alloc );
//    return response;
//  }


  void Query::prepare()
  {
    size_t index = 1;
    for ( ParameterVector::iterator p_it = _parameters.begin(); p_it != _parameters.end(); ++p_it, ++index )
    {
      // Give it to the statement
      p_it->assignStatement( _theStatement, index );
    }

    // Now we lock the connection ready to run the query
    _connectionLock.lock();
  }


  bool Query::step()
  {
    size_t temp;
    unsigned count = 0;
    while ( temp = sqlite3_step( _theStatement ), temp == SQLITE_BUSY )
    {
      if ( count == 10 )
      {
        // Set error status
        _error = "Database busy. Failed to access after repeated retries.";
        return false;
      }
      count += 1;
      std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
    }


    // Check status for our next option
    if ( temp == SQLITE_DONE )
    {
      return false;
    }
    else if ( temp != SQLITE_ROW )
    {
      _error = sqlite3_errmsg( _connection.database );
      return false;
    }

    // Fetch the column data
    temp = 0;
    for ( ParameterVector::iterator c_it = _columns.begin(); c_it != _columns.end(); ++c_it, ++temp )
    {
      // Load the returned value from the statement
      c_it->readStatement( _theStatement, temp );
    }

    // Ready for the next step
    return true;
  }


  void Query::release()
  {
    // Clean up the mess and importantly release access to the connection!
    _connectionLock.unlock();
    sqlite3_reset( _theStatement );
  }


  void Query::lock()
  {
    _theMutex.lock();
    _error = nullptr;
  }


  void Query::unlock()
  {
    // Clean up in case something catastrophic happened
    if ( _connectionLock.owns_lock() )
      this->release();

    _theMutex.unlock();
  }
}

