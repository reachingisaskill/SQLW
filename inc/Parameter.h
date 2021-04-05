
#ifndef SQLW_PARAMETER_H_
#define SQLW_PARAMETER_H_

#include "sqlite3.h"
#include "rapidjson/document.h"

#include <string>


namespace SQLW
{

  class Parameter
  {
    public:
      enum Type { Text, Int, Bool, Blob, Double };

    private:
      // Name of the paramter
      const std::string _name;

      // Union of types
      union
      {
        std::string _text;
        int64_t _int;
        bool _bool;
        std::string _blob;
        double _double;
      };

      // Enumerated type identifier
      Type _type;

    public:
      // Create and initialise the type
      Parameter( std::string name, Type t );

      // Copy from another parameter
      Parameter( const Parameter& );

      // Move from another parameter
      Parameter( Parameter&& );

      // Copy assign from another parameter (Can't be arsed to define it right now)
      Parameter& operator=( const Parameter& ) = delete;

      // Move assign from another parameter (Can't be arsed to define it right now)
      Parameter& operator=( Parameter&& ) = delete;

      // Call the correct desctuctor based on the type being used
      ~Parameter();


      // sets the parameter to an sqlite statement
      void assignStatement( sqlite3_stmt*, size_t );

      // Fills the parameter value from a column from an sqlite statement
      void readStatement( sqlite3_stmt*, size_t );


      // Sets the value using the json value object returns false if the types don't match
      bool setValue( const rapidjson::Document& );

      // Gets the value and stores it in the json document 
      void getValue( rapidjson::Document& );


      // Returns the parameter type
      Type type() const { return _type; }

      // Cast data to the requested data types. Will assert type is correct!
      explicit operator std::string() const;
      explicit operator int64_t() const;
      explicit operator bool() const;
      explicit operator void*() const;
      explicit operator double() const;

      // Sets the paramter value. Will assert type is correct!
      void set( std::string );
      void set( int64_t );
      void set( bool );
      void set( void* );
      void set( double );

  };

}

#endif // SQLW_PARAMETER_H_

