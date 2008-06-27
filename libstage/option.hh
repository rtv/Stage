#ifndef _OPTION_H_
#define _OPTION_H_

#include <string>
#include <iostream>

namespace Stg {

	class Option {
	private:
		friend bool compare( const Option* lhs, const Option* rhs );
		friend std::ostream& operator<<( std::ostream& os, const Option& opt );
		
		std::string optName;
		bool value;
	public:
		Option( std::string n, bool v ) : optName( n ), value( v ) { }
		Option( const Option& o ) : optName( o.optName ), value( o.value ) { }
		const std::string name() const { return optName; }
		inline bool val() const { return value; }
		inline operator bool() { return val(); }
		inline bool operator<( const Option& rhs ) const
			{ return optName<rhs.optName; } 
		void set( bool val ) { value = val; }
	};
	
//	std::ostream& operator<<( std::ostream& os, const Option& opt ) {
//		os<<opt.optName;
//		os<<": ";
//		os<<opt.value;
//		return os;
//	}
//	
	// Comparator to dereference Option pointers and compare their strings
	struct optComp {
		inline bool operator()( const Option* lhs, const Option* rhs ) const
			{ return lhs->operator<(*rhs); } 
	};
}
	
#endif
