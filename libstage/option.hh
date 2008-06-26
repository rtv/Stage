#ifndef _OPTION_H_
#define _OPTION_H_

#include <string>

namespace Stg {

	class Option {
	private:
		std::string optName;
		bool value;
	public:
		Option() { }
		Option( std::string n, bool v ) : optName( n ), value( v ) { }
		Option( const Option& o ) : optName( o.optName ), value( o.value ) { }
		const std::string name() const { return optName; }
		inline const bool val() const { return value; }
		inline operator bool() { return val(); }
		inline bool operator<( const Option& rhs ) const
			{ return optName<rhs.optName; } 
		friend bool compare( const Option* lhs, const Option* rhs );
		void set( bool val ) { value = val; }
	};
	
	struct optComp {
		inline bool operator()( const Option* lhs, const Option* rhs ) const
			{ return lhs->operator<(*rhs); } 
	};
}
	
#endif
