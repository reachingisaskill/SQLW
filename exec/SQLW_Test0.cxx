
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "Database.h"
#include "Query.h"

#include "CON.h"

#include <iostream>


using namespace SQLW;


int main( int, char** )
{
  std::cout << "Testing SQLW Functionality." << std::endl;

  try
  {
    CON::Object root = CON::buildFromFile( "testing/test_config.con" );

    Database db( root );

    rapidjson::StringBuffer buffer;

    {
      rapidjson::Writer<rapidjson::StringBuffer> request_writer( buffer );
      rapidjson::Document request( rapidjson::kObjectType );

      rapidjson::Writer<rapidjson::StringBuffer> response_writer( buffer );
      rapidjson::Document response = executeJson( db, "wrong", request );

      request.Accept( request_writer );
      std::cout << "REQUEST:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      response.Accept( response_writer );
      std::cout << "RESPONSE:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      std::cout << std::endl;
    }

    {
      rapidjson::Writer<rapidjson::StringBuffer> request_writer( buffer );
      rapidjson::Document request( rapidjson::kObjectType );

      rapidjson::Writer<rapidjson::StringBuffer> response_writer( buffer );
      rapidjson::Document response = executeJson( db, "all_devices", request );

      request.Accept( request_writer );
      std::cout << "REQUEST:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      response.Accept( response_writer );
      std::cout << "RESPONSE:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      std::cout << std::endl;
    }

    {
      rapidjson::Writer<rapidjson::StringBuffer> request_writer( buffer );
      rapidjson::Document request( rapidjson::kObjectType );
      request.AddMember( "index", 27, request.GetAllocator() );
      request.AddMember( "identifier", 3428975, request.GetAllocator() );
      request.AddMember( "type", "test", request.GetAllocator() );
      request.AddMember( "name", "Tester", request.GetAllocator() );
      request.AddMember( "description", "A testing device", request.GetAllocator() );

      rapidjson::Writer<rapidjson::StringBuffer> response_writer( buffer );
      rapidjson::Document response = executeJson( db, "add_device", request );

      request.Accept( request_writer );
      std::cout << "REQUEST:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      response.Accept( response_writer );
      std::cout << "RESPONSE:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      std::cout << std::endl;
    }

    {
      rapidjson::Writer<rapidjson::StringBuffer> request_writer( buffer );
      rapidjson::Document request( rapidjson::kObjectType );

      rapidjson::Writer<rapidjson::StringBuffer> response_writer( buffer );
      rapidjson::Document response = executeJson( db, "all_devices", request );

      request.Accept( request_writer );
      std::cout << "REQUEST:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      response.Accept( response_writer );
      std::cout << "RESPONSE:\n" << buffer.GetString() << '\n' << std::endl;
      buffer.Clear();

      std::cout << std::endl;
    }

  }
  catch( CON::Exception& ex )
  {
    std::cerr << "CON Exception Caught: " << ex.what() << '\n';
    for ( CON::Exception::iterator it = ex.begin(); it != ex.end(); ++it )
    {
      std::cerr << *it << std::endl;
    }
  }
  catch( std::runtime_error& ex )
  {
    std::cerr << "Unexpected runtime error occured: " << ex.what() << std::endl;
  }
  catch ( std::exception& ex )
  {
    std::cerr << "Unexpected exception occured: " << ex.what() << std::endl;
  }

  return 0;
}

