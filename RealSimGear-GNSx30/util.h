#pragma once

#include "stdafx.h"
#include "resource.h"

#define TRACE_BUFFER_SIZE 1024



void logMessage(const char* fmt, ...);
#define  PRINTLINE(s)  logMessage("%s:%d:%s  %s",__FILE__,__LINE__,__func__,s)

inline void _trace(const char *fmt, ...) {
	va_list args;
	char buf[TRACE_BUFFER_SIZE];
	char buf2[TRACE_BUFFER_SIZE];

	va_start(args, fmt);
	int l = vsnprintf(buf, TRACE_BUFFER_SIZE, fmt, args);
	va_end(args);

	snprintf(buf2, TRACE_BUFFER_SIZE, "%s: %s\n", PRODUCT_NAME, buf);
	std::cout << buf2;

#ifdef WIN32
	OutputDebugString(buf2);
#endif
}

#ifdef TRACE
#define trace(fmt, ...) _trace(fmt, ##__VA_ARGS__)
#else
#define trace(fmt, ...)
#endif


inline void log(const char *fmt, ...) {
	va_list args;
	char buf[TRACE_BUFFER_SIZE];
	char buf2[TRACE_BUFFER_SIZE];

	va_start(args, fmt);
	vsnprintf(buf, TRACE_BUFFER_SIZE, fmt, args);
	va_end(args);

	snprintf(buf2, TRACE_BUFFER_SIZE, "%s: %s\n", PRODUCT_NAME, buf);
	XPLMDebugString(buf2);
	trace("%s", buf2);

	logMessage(buf2);
}


// get_plugin_path
#if defined(WIN32)
extern "C" __declspec(dllexport) void dummy_func();
#define PATH_BUFFER_SIZE 10240
inline std::string get_plugin_path() {
	HMODULE hm = NULL;
	if (!GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCSTR)&dummy_func, &hm)) {
		throw std::exception("GetModuleHandleEx failed");
	}

	char path[PATH_BUFFER_SIZE];
	DWORD len2 = GetModuleFileName(hm, path, PATH_BUFFER_SIZE);
	if (GetLastError() != ERROR_SUCCESS) {
		throw std::exception("GetModuleFileName failed");
	}

	trace("Plugin path is %s", path);
	return std::string(path);
}
#elif defined(__linux__) || defined(__APPLE__)
inline std::string get_plugin_path() {
	Dl_info dl_info;
	dladdr((void *)get_plugin_path, &dl_info);
	return std::string(dl_info.dli_fname);
}
#endif


// path
class path {
	#if defined(WIN32)
	static const char sep = '\\';
	#elif defined(__linux__) || defined(__APPLE__)
	static const char sep = '/';
	#endif	
public:
	static std::string remove_filename(const std::string &path) {
		size_t p = path.find_last_of(sep);
		if (p == std::string::npos)
			return path;
		return path.substr(0, p);
	}

	static std::string seperator() {
		return std::string(1, sep);
	}
};


// get_last_error_as_string
#if defined(WIN32)
inline std::string get_last_error_as_string() {
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); 

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
	std::string message(messageBuffer, size);
	LocalFree(messageBuffer);

	return message;
}
#elif defined(__linux__)
inline std::string get_last_error_as_string() {
	char buf[1024];
	char *res = strerror_r(errno, buf, sizeof(buf));
	return std::string(res);
}
#elif defined(__APPLE__)
inline std::string get_last_error_as_string() {
	char buf[1024];
	strerror_r(errno, buf, sizeof(buf));
	return std::string(buf);
}
#endif


// strcpy_s
#if defined(__linux__) || defined(__APPLE__)
inline int strcpy_s(char *dest, size_t destsz, const char *src) {
	strcpy(dest, src);
	return 0;
}
#endif

