#pragma once

#include <exception>
#include <string>
#include <comdef.h>

#include "BatWinAPI.h"

#define THROW_COM_ERROR( hr, msg ) throw COMException( hr, msg, __FILE__, __FUNCTION__, __LINE__ )
#define COM_THROW_IF_FAILED( hr ) if( FAILED( hr ) ) throw COMException( hr, __FILE__, __FUNCTION__, __LINE__ )

class COMException : public std::exception
{
public:
	COMException( HRESULT hr )
	{
		_com_error error( hr );
		whatmsg = error.ErrorMessage();
	}

	COMException(HRESULT hr, const std::string& message)
	{
		_com_error error( hr );
		whatmsg = "Error: " + message + '\n';
		whatmsg = error.ErrorMessage();
	}

	COMException( HRESULT hr, const std::string& file, const std::string& function, int line )
	{
		_com_error error( hr );
		whatmsg = error.ErrorMessage();
		whatmsg += '\n';
		whatmsg += '\n';
		whatmsg += "File: " + file + '\n';
		whatmsg += "Function: " + function + '\n';
		whatmsg += "Line: " + std::to_string( line ) + '\n';
	}

	COMException( HRESULT hr, const std::string& message, const std::string& file, const std::string& function, int line )
	{
		_com_error error( hr );
		whatmsg = "Error: " + message + '\n';
		whatmsg += error.ErrorMessage();
		whatmsg += '\n';
		whatmsg += '\n';
		whatmsg += "File: " + file + '\n';
		whatmsg += "Function: " + function + '\n';
		whatmsg += "Line: " + std::to_string( line ) + '\n';
	}

	virtual const char* what() const override
	{
		return whatmsg.c_str();
	}
private:
	HRESULT hr;
	std::string whatmsg;
};