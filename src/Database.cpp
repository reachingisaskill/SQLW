
#include "rapidjson/document.h"

#include "Database.h"
#include "Query.h"

#include <iostream>
#include <sys/stat.h>


namespace SQLW
{

  Database::Database( const CON::Object& config ) :
    _filename(),
    _busyRetries( 10 ),
    _connection(),
    _queries()
  {
    _connection.database = nullptr;
    // Use the schema to validate the config data

    // Load the config data
    _filename = config["database_file"].asString();

    if ( config.has( "busy_retries" ) )
    {
      _busyRetries = config["busy_retries"].asInt();
    }


    // Initialise the database connection
    struct stat file_stat;
    int result = stat( _filename.c_str(), &file_stat );
    if ( result != 0 )
    {
      throw std::runtime_error( "Database Not Found" );
    }
    else if ( ! S_ISREG( file_stat.st_mode ) )
    {
      throw std::runtime_error( "Database Not Found" );
    }

    result = sqlite3_open_v2( _filename.c_str(), &_connection.database, SQLITE_OPEN_READWRITE, nullptr );

    if ( result != SQLITE_OK )
    {
      sqlite3_close( _connection.database );
      _connection.database = nullptr;

      throw std::runtime_error( "Database Error" );
    }

    // Load the query interfaces
    const CON::Object& query_data = config["query_data"];

    for ( size_t i = 0; i < query_data.getSize(); ++i )
    {
      const CON::Object query_conf = query_data[i];

      Query* q = new Query( _connection, query_conf );

      _queries.insert( std::make_pair( query_conf["name"].asString(), q ) );
    }
  }


  Database::~Database()
  {
    for ( QueryMap::iterator it = _queries.begin(); it != _queries.end(); ++it )
    {
      delete it->second;
    }
    _queries.clear();

    sqlite3_close_v2( _connection.database );
  }


  Query& Database::requestQuery( const char* name )
  {
    QueryMap::iterator found = _queries.find( name );

    if ( found == _queries.end() )
    {
      std::cerr << "SQLW Error - Requested query not found: " << name << std::endl;
      throw std::runtime_error( "Requested query does not exist" );
    }

    return *found->second;
  }


  bool Database::queryExists( const char* name ) const
  {
    if ( _queries.find( name ) == _queries.end() )
      return false;
    else
      return true;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Optional Friend functions

////////////////////////////////////////////////////////////////////////////////
  // RapidJson Library

  bool setParameter( Parameter& param, const rapidjson::Document& data )
  {
    if ( ! data.HasMember( param.name().c_str() ) )
      return false;

    switch( param.type() )
    {
      case Parameter::Text :
        if ( ! data[param.name().c_str()].IsString() )
          return false;
        else
          param.set( std::string( data[param.name().c_str()].GetString() ) );
        break;
    
      case Parameter::Int :
        if ( ! data[param.name().c_str()].IsInt64() )
          return false;
        else
          param.set( data[param.name().c_str()].GetInt64() );
        break;
    
      case Parameter::Bool :
        if ( ! data[param.name().c_str()].IsBool() )
          return false;
        else
          param.set( data[param.name().c_str()].GetBool() );
        break;
    
      case Parameter::Blob :
        if ( ! data[param.name().c_str()].IsString() )
          return false;
        else
          param.set( std::string( data[param.name().c_str()].GetString() ) );
        break;
    
      case Parameter::Double :
        if ( ! data[param.name().c_str()].IsDouble() )
          return false;
        else
          param.set( data[param.name().c_str()].GetDouble() );
        break;
    }
    return true;
  }


  void getParameter( Parameter& param, rapidjson::Value& data, rapidjson::Document::AllocatorType& alloc )
  {
    switch( param.type() )
    {
      case Parameter::Text :
        data.AddMember( rapidjson::Value( param.name().c_str(), alloc ).Move(),
                        rapidjson::Value( static_cast<std::string>( param ).c_str(), alloc ), alloc );
        break;
    
      case Parameter::Int :
        data.AddMember( rapidjson::Value( param.name().c_str(), alloc ).Move(),
                        static_cast< int64_t >( param ), alloc );
        break;
    
      case Parameter::Bool :
        data.AddMember( rapidjson::Value( param.name().c_str(), alloc ).Move(),
                        static_cast< bool >( param ), alloc );
        break;
    
      case Parameter::Blob :
        data.AddMember( rapidjson::Value( param.name().c_str(), alloc ).Move(),
                        rapidjson::Value( static_cast< std::string >( param ).c_str(), alloc ), alloc );
        break;
    
      case Parameter::Double :
        data.AddMember( rapidjson::Value( param.name().c_str(), alloc ).Move(),
                        static_cast< double >( param ), alloc );
        break;
    }
  }


  rapidjson::Document executeJson( Database& db, const char* name, const rapidjson::Document& data )
  {
    Database::QueryMap::iterator found = db._queries.find( name );
    rapidjson::Document response( rapidjson::kObjectType );
    rapidjson::Document::AllocatorType& alloc = response.GetAllocator();

    if ( found == db._queries.end() )
    {
      response.AddMember( "success", false, alloc );
      response.AddMember( "error", rapidjson::Value( "Invalid request. Does not exist.", alloc ), alloc );
      response.AddMember( "data", rapidjson::Value( rapidjson::kArrayType ), alloc );
      return response;
    }

    Query& query = *found->second;

    // Lock the query. We're using it now
    auto query_lock = query.acquire();

    // Load the parameters
    for ( Query::ParameterIterator pit = query.parametersBegin(); pit != query.parametersEnd(); ++pit )
    {
      if ( ! setParameter( *pit, data ) )
      {
        std::string err_string( "Invalid request parameter: " );
        err_string += pit->name();

        response.AddMember( "success", false, response.GetAllocator() );
        response.AddMember( "error", rapidjson::Value( err_string.c_str(), alloc ), alloc );
        response.AddMember( "data", rapidjson::Value( rapidjson::kArrayType ), alloc );
        return response;
      }
    }

    rapidjson::Value column_data( rapidjson::kArrayType );

    // Lock the database connection
    query.prepare();

    // Step through the query
    while ( query.step() )
    {
      rapidjson::Value col( rapidjson::kObjectType );
      for ( Query::ColumnIterator cit = query.columnsBegin(); cit != query.columnsEnd(); ++cit )
      {
        getParameter( *cit, col, alloc );
      }
      column_data.PushBack( col, alloc );
    }

    // Release the database connection
    query.reset();

    if ( query.error() )
    {
      response.AddMember( "success", false, alloc );
      response.AddMember( "error", rapidjson::Value( query.getError(), alloc ), alloc );
      response.AddMember( "data", rapidjson::Value( rapidjson::kArrayType ), alloc );
    }
    else
    {
      response.AddMember( "success", true, alloc );
      response.AddMember( "data", column_data, alloc );
    }

    return response;
  }

}

