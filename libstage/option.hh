#ifndef _OPTION_H_
#define _OPTION_H_

#include <string>

namespace Stg {

	class Option {
	private:
		int index;
		std::string optName;
		bool value;
	public:
		Option() { }
		Option( int i, std::string n, bool v ) : index( i ), optName( n ), value( v ) { }
		Option( const Option& o ) : index( o.index ), optName( o.optName ), value( o.value ) { }
		const std::string name() const { return optName; }
		const int id() const { return index; }
		const bool val() const { return value; }
		void set( bool val ) { value = val; }
	};

}
	
#endif
