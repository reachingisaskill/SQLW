
#include "Database.h"
#include "Query.h"

#include <sys/stat.h>


namespace SQLW
{

  Database::Database( const CON::Object& config ) :
    _filename(),
    _busyRetries( 10 ),
    _database( nullptr ),
    _queries()
  {
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

    result = sqlite3_open_v2( _filename.c_str(), &_database, SQLITE_OPEN_READWRITE, nullptr );

    if ( result != SQLITE_OK )
    {
      sqlite3_close( _database );
      _database = nullptr;

      throw std::runtime_error( "Database Error" );
    }

    // Load the query interfaces
    const CON::Object& query_data = config["query_data"];

    for ( size_t i = 0; i < query_data.getSize(); ++i )
    {
      const CON::Object query_conf = query_data[i];

      Query* q = new Query( _database, query_conf );

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

    sqlite3_close_v2( _database );
  }


  rapidjson::Document Database::executeOperation( const char* name, const rapidjson::Document& data )
  {
    QueryMap::iterator found = _queries.find( name );

    if ( found == _queries.end() )
    {
      rapidjson::Document response( rapidjson::kObjectType );
      response.AddMember( "success", false, response.GetAllocator() );
      response.AddMember( "error", rapidjson::Value( "Invalid request.", response.GetAllocator() ), response.GetAllocator() );
      return response;
    }
    else
    {
      return found->second->run( data );
    }
  }

}

