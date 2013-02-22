#ifndef _netkit_error_h
#define _netkit_error_h

namespace netkit {

enum class error
{
	none				= 0,
	invalid_license		= -32000,
	license_expired		= -32001,
	no_memory			= -32002,
	no_plugin			= -32003,
	no_function			= -32004,
	not_authorized		= -32005,
	write_failed		= -32006,
	not_implemented		= -32007,
	unexpected			= -32008,
	connection_aborted	= -32009,
	parse				= -32700,
	invalid_request		= -32600,
	method_not_found	= -32601,
	invalid_parms		= -32602,
	internal			= -32603
};

}

#endif
