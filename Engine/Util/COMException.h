#pragma once

#include "Platform/BatWinAPI.h"

#include <exception>
#include <string>
#include <comdef.h>

#ifdef _DEBUG
#define THROW_COM_ERROR( hr, msg ) ASSERT( false, COMException( hr, msg, __FILE__, __FUNCTION__, __LINE__ ).WhatWithoutFile() )
#define COM_THROW_IF_FAILED( hr ) ASSERT( SUCCEEDED( hr ), COMException( hr, "Error", __FILE__, __FUNCTION__, __LINE__ ).WhatWithoutFile() )
#else
#define THROW_COM_ERROR( hr, msg ) throw COMException( hr, msg, __FILE__, __FUNCTION__, __LINE__ )
#define COM_THROW_IF_FAILED( hr ) do { if( FAILED( hr ) ) throw COMException( hr, __FILE__, __FUNCTION__, __LINE__ ); } while( false )
#endif

class COMException : public std::exception
{
public:
	COMException( HRESULT hr )
		:
		hr( hr )
	{
		_com_error error( hr );
		whatmsg = error.ErrorMessage();
	}

	COMException(HRESULT hr, const std::string& message)
		:
		hr( hr )
	{
		_com_error error( hr );
		whatmsg = "Error: " + message + '\n';
		whatmsg = error.ErrorMessage();
	}

	COMException( HRESULT hr, const std::string& file, const std::string& function, int line )
		:
		hr( hr )
	{
		_com_error error( hr );
		whatmsg = error.ErrorMessage();
		whatmsg += '\n';

		filemsg = "";
		filemsg += '\n';
		filemsg += "File: " + file + '\n';
		filemsg += "Function: " + function + '\n';
		filemsg += "Line: " + std::to_string( line ) + '\n';
	}

	COMException( HRESULT hr, const std::string& message, const std::string& file, const std::string& function, int line )
		:
		hr( hr )
	{
		_com_error error( hr );
		whatmsg = "Error: " + message + '\n';
		whatmsg += error.ErrorMessage();
		whatmsg += '\n';

		filemsg = "";
		filemsg += '\n';
		filemsg += "File: " + file + '\n';
		filemsg += "Function: " + function + '\n';
		filemsg += "Line: " + std::to_string( line ) + '\n';
	}

	HRESULT GetHResult() const { return hr; }

	const char* WhatWithoutFile() const
	{
		return whatmsg.c_str();
	}

	virtual const char* what() const override
	{
		return (whatmsg + filemsg).c_str();
	}
private:
	HRESULT hr;
	std::string whatmsg;
	std::string filemsg;
};