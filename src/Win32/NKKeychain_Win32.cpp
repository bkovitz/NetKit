#include "NKKeychain_Win32.h"
#include <NetKit/NKBase64.h>
#include <NetKit/NKLog.h>
#include <WinSock2.h>
#include <Windows.h>
#include <WinCrypt.h>
#include <string>

using namespace netkit;

static keychain::ref g_instance;

keychain::ref
keychain::instance()
{
	if ( !g_instance )
	{
		g_instance = new keychain_win32;
	}

	return g_instance;
}


netkit::status
keychain_win32::store( const std::string &url, const std::string &secret, std::string &key )
{
	DATA_BLOB		data_in;
	DATA_BLOB		data_out;
	netkit::status	err = status::ok;

	data_in.pbData = ( BYTE* ) secret.c_str();
	data_in.cbData = ( DWORD ) ( secret.size() + 1 );

	if ( !CryptProtectData( &data_in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_LOCAL_MACHINE | CRYPTPROTECT_UI_FORBIDDEN, &data_out ) )
	{
		nklog( log::error, "CryptProtectData() failed: %d", ::GetLastError() );
		err = status::internal_error;
		goto exit;
	}

	key = codec::base64::encode( std::string( data_out.pbData, data_out.pbData + data_out.cbData ) );

exit:

	return err;
}


netkit::status
keychain_win32::lookup( const std::string &key, std::string &secret )
{
	std::string		decoded;
	DATA_BLOB		data_out;
	DATA_BLOB		data_verify;
	netkit::status	err = status::ok;

	decoded			= codec::base64::decode( key );
	data_out.pbData = ( LPBYTE ) decoded.c_str();
	data_out.cbData = ( DWORD ) decoded.size();

	if ( !CryptUnprotectData( &data_out, nullptr, nullptr, nullptr, nullptr, 0, &data_verify ) )
	{
		nklog( log::error, "CryptUnprotectData() failed: %d", ::GetLastError() );
		err = status::internal_error;
		goto exit;
	}

	secret.assign( data_verify.pbData, data_verify.pbData + data_verify.cbData );

exit:

	return err;
}
