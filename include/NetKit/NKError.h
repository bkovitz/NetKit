#ifndef _coreapp_error_h
#define _coreapp_error_h


namespace coreapp {

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
