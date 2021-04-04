
#include "Query.h"
#include "Database.h"

#include <thread>
#include <iostream>


namespace SQLW
{

  Query::Query( sqlite3* db, const CON::Object& config ) :
    _database( db ),
    _theStatement( nullptr ),
    _description( config["description"].asString() ),
    _statementText( config["statement"].asString() ),
    _error( nullptr )
  {
    int result = sqlite3_prepare_v3( db, _statementText.c_str(), _statementText.size(), SQLITE_PREPARE_PERSISTENT, &_theStatement, nullptr );

    if ( result != SQLITE_OK || _theStatement == nullptr )
    {
      std::stringstream ss;
      ss << "Fail to prepare query: " << _description << " Error " << result << " : " << sqlite3_errmsg( db );
      throw std::runtime_error( "Failed to prepare query." );
    } 

    std::cout << "STATEMENT = " << _statementText << std::endl << std::endl;

    const CON::Object& parameters = config["parameters"];
    for ( size_t i = 0; i < parameters.getSize(); ++i )
    {
      const CON::Object& param = parameters[i];

      Parameter p;
      p.name = param["name"].asString();

      if ( param["type"].asString() == "text" )
      {
        p.type = Parameter::Text;
      }
      else if ( param["type"].asString() == "int" ) 
      {
        p.type = Parameter::Int;
      }
      else if ( param["type"].asString() == "blob" )
      {
        p.type = Parameter::Blob;
      }
      else if ( param["type"].asString() == "double" || param["type"].asString() == "float" )
      {
        p.type = Parameter::Double;
      }
      else
      {
        throw std::runtime_error( "Unknown parameter type." );
      }

      _parameters.push_back( p );
    }

    const CON::Object& columns = config["columns"];
    for ( size_t i = 0; i < columns.getSize(); ++i )
    {
      const CON::Object& column = columns[i];

      Parameter c;
      c.name = column["name"].asString();
      
      if ( column["type"].asString() == "text" )
      {
        c.type = Parameter::Text;
      }
      else if ( column["type"].asString() == "int" ) 
      {
        c.type = Parameter::Int;
      }
      else if ( column["type"].asString() == "blob" )
      {
        c.type = Parameter::Blob;
      }
      else if ( column["type"].asString() == "double" || column["type"].asString() == "float" )
      {
        c.type = Parameter::Double;
      }
      else
      {
        throw std::runtime_error( "Unknown column type." );
      }

      _columns.push_back( c );
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


  rapidjson::Document Query::run( const rapidjson::Document& json_data )
  {
    // Lock the query mutex
    std::lock_guard<std::mutex> lock( _theMutex );

    std::cout << "Running Query" << std::endl;

    // Json data to return
    rapidjson::Document response( rapidjson::kObjectType );
    rapidjson::Document::AllocatorType& alloc = response.GetAllocator();

    // Load the parameters
    size_t index = 1;
    for ( ParameterList::const_iterator p_it = _parameters.begin(); p_it != _parameters.end(); ++p_it, ++index )
    {
      if ( json_data.HasMember( p_it->name.c_str() ) )
      {
        std::cout << index << " Parameter: " << p_it->name << std::endl;
        switch( p_it->type )
        {
          case Parameter::Text :
            if ( json_data[p_it->name.c_str()].IsString() )
            {
              std::string param_value = json_data[p_it->name.c_str()].GetString();
              std::cout << " TEXT PARAM : " << param_value << "(" << param_value.size() << ")" << std::endl;
              std::cout << "First char = " << (int)param_value.c_str()[0] << std::endl;
              sqlite3_bind_text( _theStatement, index, param_value.c_str(), param_value.size(), nullptr );
              continue;
            }
            break;

          case Parameter::Int :
            if ( json_data[p_it->name.c_str()].IsInt64() )
            {
              sqlite3_bind_int64( _theStatement, index, json_data[p_it->name.c_str()].GetInt64() );
              continue;
            }
            break;

          case Parameter::Bool :
            if ( json_data[p_it->name.c_str()].IsBool() )
            {
              sqlite3_bind_int64( _theStatement, index, ( json_data[p_it->name.c_str()].GetBool() ? 1 : 0 ) );
              continue;
            }
            break;

          case Parameter::Blob :
            if ( json_data[p_it->name.c_str()].IsString() )
            {
              std::string param_value = json_data[p_it->name.c_str()].GetString();
              sqlite3_bind_blob( _theStatement, index, (const void*)param_value.c_str(), param_value.size(), nullptr );
              continue;
            }
            break;

          case Parameter::Double :
            if ( json_data[p_it->name.c_str()].IsDouble() )
            {
              sqlite3_bind_int64( _theStatement, index, json_data[p_it->name.c_str()].GetDouble() );
              continue;
            }
            break;
        }
      }

      _error = "Required parameters not found in json documnent.";
      response.AddMember( "success", false, alloc );
      response.AddMember( "error", rapidjson::Value( _error, alloc ), alloc );
      return response;
    }

    // Execute the query
    int result;
    bool done = false;

    while( true )
    {
      unsigned count = 0;
      while ( result = sqlite3_step( _theStatement ), result == SQLITE_BUSY )
      {
        std::cout << "BUSY" << std::endl;

        if ( count == 10 )
        {
          // Set error status
          _error = "Database busy. Failed to access after repeated retries.";
          response.AddMember( "success", false, alloc );
          response.AddMember( "error", rapidjson::Value( _error, alloc ), alloc );
          sqlite3_reset( _theStatement );
          return response;
        }
        count += 1;
        std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
      }

      // Check status for our next option
      switch( result )
      {
        case SQLITE_DONE :
          std::cout << "DONE" << std::endl;
          done = true;
          break;

        case SQLITE_ROW :
          std::cout << "ROW" << std::endl;
          break;

        default:
          std::cout << "SOMETHING ELSE? " << result << std::endl;
          _error = sqlite3_errmsg( _database );
          response.AddMember( "success", false, alloc );
          response.AddMember( "error", rapidjson::Value( _error, alloc ), alloc );
          sqlite3_reset( _theStatement );
          return response;
          break;
      }

      if ( done ) break;

      // Fetch the column data
      index = 0;
      for ( ParameterList::const_iterator c_it = _columns.begin(); c_it != _columns.end(); ++c_it, ++index )
      {
        std::cout << "Column: " << c_it->name << std::endl;
        switch( c_it->type )
        {
          case Parameter::Text :
            {
              const char* value = (const char*)sqlite3_column_text( _theStatement, index );
              response.AddMember( rapidjson::Value( c_it->name.c_str(), alloc ).Move(),
                                  rapidjson::Value( ( value ? value : "" ), alloc ),
                                  alloc );
            }
            break;

          case Parameter::Int :
            response.AddMember( rapidjson::Value( c_it->name.c_str(), alloc ).Move(),
                                (int64_t)sqlite3_column_int64( _theStatement, index ), alloc );
            break;

          case Parameter::Bool :
            response.AddMember( rapidjson::Value( c_it->name.c_str(), alloc ).Move(),
                                ( sqlite3_column_int64( _theStatement, index ) == 0 ? false : true ), alloc );
            break;

          case Parameter::Blob :
            {
              const char* value = (const char*)sqlite3_column_blob( _theStatement, index );
              response.AddMember( rapidjson::Value( c_it->name.c_str(), alloc ).Move(),
                                  rapidjson::Value( ( value ? value : 0 ), alloc ),
                                  alloc );
            }
            break;

          case Parameter::Double :
            response.AddMember( rapidjson::Value( c_it->name.c_str(), alloc ).Move(),
                                sqlite3_column_double( _theStatement, index ), alloc );
            break;
        }
      }
    }
    sqlite3_reset( _theStatement );

    response.AddMember( "success", true, alloc );
    return response;
  }

}

