
#include "Parameter.h"

#include <cassert>


namespace SQLW
{

  Parameter::Parameter( std::string name, Parameter::Type t ) :
    _name( name ),
    _type( t )
  {
    switch( _type )
    {
      case Text :
        new (&_text) std::string();
        break;
      case Int :
        new (&_int) int( 0 );
        break;
      case Bool :
        new (&_bool) bool( false );
        break;
      case Blob :
        new (&_blob) std::string();
        break;
      case Double :
        new (&_double) double( 0.0 );
        break;
    }
  }


  Parameter::Parameter( const Parameter& p ) :
    _name( p._name ),
    _type( p._type )
  {
    switch( _type )
    {
      case Text :
        new (&_text) std::string( p._text );
        break;
      case Int :
        new (&_int) int( p._int );
        break;
      case Bool :
        new (&_bool) bool( p._bool );
        break;
      case Blob :
        new (&_blob) std::string( p._blob );
        break;
      case Double :
        new (&_double) double( p._double );
        break;
    }
  }


  Parameter::Parameter( Parameter&& p ) :
    _name( std::move( p._name ) ),
    _type( std::move( p._type ) )
  {
    switch( _type )
    {
      case Text :
        new (&_text) std::string( std::move( p._text ) );
        break;
      case Int :
        new (&_int) int( std::move( p._int ) );
        break;
      case Bool :
        new (&_bool) bool( std::move( p._bool ) );
        break;
      case Blob :
        new (&_blob) std::string( std::move( p._blob ) );
        break;
      case Double :
        new (&_double) double( std::move( p._double ) );
        break;
    }
  }


  Parameter::~Parameter()
  {
    using std::string;
    switch( _type )
    {
      case Text :
        _text.~string();
        break;
      case Int :
        break;
      case Bool :
        break;
      case Blob :
        _blob.~string();
        break;
      case Double :
        break;
    }
  }


  void Parameter::assignStatement( sqlite3_stmt* stmt, size_t index )
  {
    switch( _type )
    {
      case Text :
        sqlite3_bind_text( stmt, index, _text.c_str(), _text.size(), nullptr );
        break;

      case Int :
        sqlite3_bind_int64( stmt, index, _int );
        break;

      case Bool :
        sqlite3_bind_int64( stmt, index, ( _bool ? 1 : 0 ) );
        break;

      case Blob :
        sqlite3_bind_blob( stmt, index, (const void*)_blob.c_str(), _blob.size(), nullptr );
        break;

      case Double :
        sqlite3_bind_int64( stmt, index, _double );
        break;
    }
  }


  void Parameter::readStatement( sqlite3_stmt* stmt, size_t index )
  {
    switch( _type )
    {
      case Parameter::Text :
        _text = (const char*)sqlite3_column_text( stmt, index );
        break;

      case Parameter::Int :
        _int = sqlite3_column_int64( stmt, index );
        break;

      case Parameter::Bool :
        _bool = sqlite3_column_int64( stmt, index );
        break;

      case Parameter::Blob :
        _blob = (const char*)sqlite3_column_blob( stmt, index );
        break;

      case Parameter::Double :
        _double = sqlite3_column_int64( stmt, index );
        break;
    }
  }


  bool Parameter::setValue( const rapidjson::Document& data )
  {
    if ( ! data.HasMember( _name.c_str() ) )
      return false;

    switch( _type )
    {
      case Parameter::Text :
        if ( ! data[_name.c_str()].IsString() )
          return false;
        else
          _text = data[_name.c_str()].GetString();
        break;
    
      case Parameter::Int :
        if ( ! data[_name.c_str()].IsInt64() )
          return false;
        else
          _int = data[_name.c_str()].GetInt64();
        break;
    
      case Parameter::Bool :
        if ( ! data[_name.c_str()].IsBool() )
          return false;
        else
          _bool = data[_name.c_str()].GetBool();
        break;
    
      case Parameter::Blob :
        if ( ! data[_name.c_str()].IsString() )
          return false;
        else
          _blob = data[_name.c_str()].GetString();
        break;
    
      case Parameter::Double :
        if ( ! data[_name.c_str()].IsDouble() )
          return false;
        else
          _double = data[_name.c_str()].GetDouble();
        break;
    }
    return true;
  }


  void Parameter::getValue( rapidjson::Document& data )
  {
    switch( _type )
    {
      case Parameter::Text :
        data.AddMember( rapidjson::Value( _name.c_str(), data.GetAllocator() ).Move(),
                        rapidjson::Value( _text.c_str(), data.GetAllocator() ), data.GetAllocator() );
        break;
    
      case Parameter::Int :
        data.AddMember( rapidjson::Value( _name.c_str(), data.GetAllocator() ).Move(),
                        _int, data.GetAllocator() );
        break;
    
      case Parameter::Bool :
        data.AddMember( rapidjson::Value( _name.c_str(), data.GetAllocator() ).Move(),
                        _bool, data.GetAllocator() );
        break;
    
      case Parameter::Blob :
        data.AddMember( rapidjson::Value( _name.c_str(), data.GetAllocator() ).Move(),
                        rapidjson::Value( _blob.c_str(), data.GetAllocator() ), data.GetAllocator() );
        break;
    
      case Parameter::Double :
        data.AddMember( rapidjson::Value( _name.c_str(), data.GetAllocator() ).Move(),
                        _double, data.GetAllocator() );
        break;
    }
  }


  Parameter::operator std::string() const
  {
    assert( _type == Parameter::Text || _type == Blob );
    if ( _type == Parameter::Text )
      return _text;
    else
      return _blob;
  }


  Parameter::operator int64_t() const
  {
    assert( _type == Parameter::Int );
    return _int;
  }


  Parameter::operator bool() const
  {
    assert( _type == Parameter::Bool );
    return _bool;
  }


  Parameter::operator void*() const
  {
    assert( _type == Parameter::Blob );
    return (void*)( _blob.c_str() );
  }


  Parameter::operator double() const
  {
    assert( _type == Parameter::Double );
    return _double;
  }


  void Parameter::set( std::string val )
  {
    assert( _type == Parameter::Text || _type == Parameter::Blob );
    if ( _type == Text )
      _text = val;
    else
      _blob = val;
  }


  void Parameter::set( int64_t val )
  {
    assert( _type == Parameter::Int );
    _int = val;
  }


  void Parameter::set( bool val )
  {
    assert( _type == Parameter::Bool );
    _bool = val;
  }


  void Parameter::set( void* val )
  {
    assert( _type == Parameter::Blob );
    _blob = (const char*) val;
  }


  void Parameter::set( double val )
  {
    assert( _type == Parameter::Double );
    _double = val;
  }

}

