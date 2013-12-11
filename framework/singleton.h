/*
 * Copyright (c) 2013 The Simutrans Community
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * Singleton
 */

#ifndef singleton_h
#define singleton_h

#include <assert.h>

template <typename T> class singleton
{
private:
	singleton(const singleton<T> &);
	singleton& operator=(const singleton<T> &);
    
protected:

	static T* static_singleton;

public:
	singleton( void )
	{
		assert( !static_singleton );
		static_singleton = static_cast< T* >( this );
	}

	~singleton( void )
	{
		assert( static_singleton );
		static_singleton = 0;
	}

	static T& get_singleton( void )
	{
		assert( static_singleton );
		return ( *static_singleton );
	}

	static T* get_singleton_ptr( void )
	{
		return static_singleton;
	}
};

#endif