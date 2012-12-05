#ifndef _CoreApp_error_h
#define _CoreApp_error_h


namespace CoreApp {

typedef enum
{
	okay			= 0,
	uninitialized,
	unimplemented,
	out_of_memory,
	write_failed,
	read_failed,
	object_unknown,
	object_exists,
	internal_error,
	unknown
} status;

}

#endif
