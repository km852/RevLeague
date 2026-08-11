#include "NvLib.h"

#include <cstring>
#include <format>
#include <string>
#include <Windows.h>

static LARGE_INTEGER firstTick;
static LARGE_INTEGER tickFreq;
static NvCriticalSection logLock;

static double GetLogTime()
{
	LARGE_INTEGER currentTick;
	QueryPerformanceCounter(&currentTick);

	return (currentTick.QuadPart - firstTick.QuadPart) / static_cast<double>(tickFreq.QuadPart);
}

static void LogMessageRaw(const void* data, unsigned int bytes)
{
	DWORD written;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), data, bytes, &written, NULL);
}

__forceinline static void LogMessageRaw(const std::string& s)
{
	LogMessageRaw(s.data(), (unsigned int)s.size());
}

__forceinline static void LogMessageRaw(char c)
{
	LogMessageRaw(&c, 1);
}

template <int N>
__forceinline static void LogMessageRaw(const char(&str)[N])
{
	LogMessageRaw(str, N - 1);
}

template <std::size_t BufferSize, std::size_t MessageLen>
static void LogMessageFormatted(const char(&str)[MessageLen], void* dummy, ...) // "dummy" is here because va_start doesn't like to be called with a reference (i.e. "str" argument)
{
	char buf[BufferSize];

	va_list args;
	va_start(args, dummy);

	int retval = vsnprintf(buf, BufferSize, str, args);
	if (retval >= 0)
	{
		DWORD unused;
		WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buf, (DWORD)strlen(buf), &unused, nullptr);
	}

	va_end(args);
}

static const char* GetLogLevelAttribs(NvLogLevel level)
{
	switch (level)
	{
	case NvLogLevel::Debug:
		return "\033[38;2;255;255;255m";
	case NvLogLevel::Info:
		return "\033[38;2;0;255;0m";
	case NvLogLevel::Warning:
		return "\033[38;2;255;255;0m";
	case NvLogLevel::Error:
		return "\033[38;2;224;32;32m";
	case NvLogLevel::INTERNAL_FuncName:
		return "\033[38;2;96;128;128m";
	case NvLogLevel::INTERNAL_Reset:
		return "\033[0m";
	default:
		__assume(0);
	}
}

static const char* GetLogLevelString(NvLogLevel level)
{
	switch (level)
	{
	case NvLogLevel::Debug:
		return "DEBUG";
	case NvLogLevel::Info:
		return "INFO";
	case NvLogLevel::Warning:
		return "WARN";
	case NvLogLevel::Error:
		return "ERROR";
	default:
		__assume(0);
	}
}

void NvLogger::InitializeLogger()
{
	AllocConsole();

	DWORD terminalMode = 0;
	GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &terminalMode);
	terminalMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), terminalMode);

	QueryPerformanceCounter(&firstTick);
	QueryPerformanceFrequency(&tickFreq);
}

void NvLogger::PrintHexBuffer(const unsigned char* data, size_t dataLength)
{
	NvAutoCriticalSection lock = logLock.Acquire();

	for (size_t i = 0; i < dataLength; i += 16)
	{
		LogMessageRaw("\033[38;2;224;255;255m  ");

		for (size_t j = 0; j < 16; ++j)
		{
			size_t idx = i + j;
			if (idx < dataLength)
				LogMessageFormatted<8>("%02X ", nullptr, data[idx]);
			else
				LogMessageRaw("   ");
		}

		LogMessageRaw(" | ");

		for (size_t j = 0; j < 16; ++j)
		{
			size_t idx = i + j;
			if (idx < dataLength)
				LogMessageRaw(data[idx] >= 0x20 && data[idx] <= 0x7e ? (char)data[idx] : '.');
			else
				LogMessageRaw(' ');
		}

		LogMessageRaw("\n");
	}
}

template <bool UseLastError>
static int LogAssertInternal(const char* cond, const char* funcName, const char* file, int line, unsigned int lastError)
{
	NvAutoCriticalSection lock = logLock.Acquire();

	NvLogError("Assertion failed in {}{}{} [{}{}{}]", GetLogLevelAttribs(NvLogLevel::INTERNAL_FuncName), funcName, GetLogLevelAttribs(NvLogLevel::INTERNAL_Reset),
		GetLogLevelAttribs(NvLogLevel::Warning), cond, GetLogLevelAttribs(NvLogLevel::INTERNAL_Reset));
	NvLogError("Location: {}:{}", file, line);
	if constexpr (UseLastError)
	{
		LPWSTR errorMessage = nullptr;
		std::string errorMessageUTF8;

		if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, lastError, 0, (LPWSTR)&errorMessage, 0, nullptr))
		{
			errorMessageUTF8 = NvLib::NvUtils::WideToUTF8(errorMessage);
			while (errorMessageUTF8.rbegin() != errorMessageUTF8.rend() && (*errorMessageUTF8.rbegin() == '\n' || *errorMessageUTF8.rbegin() == '\r'))
				errorMessageUTF8.pop_back();

			LocalFree(errorMessage);
		}

		NvLogError("Last OS error: {} ({})", lastError, errorMessageUTF8);
	}

	return 0;
}

int NvLogger::LogAssert2(const char* cond, const char* funcName, const char* file, int line)
{
	return LogAssertInternal<false>(cond, funcName, file, line, 0);
}

int NvLogger::LogAssert2(const char* cond, const char* funcName, const char* file, int line, unsigned int lastError)
{
	return LogAssertInternal<true>(cond, funcName, file, line, lastError);
}

int NvLogger::LogMessage(NvLogLevel logLevel, const char* funcName, const std::string& message)
{
	NvAutoCriticalSection lock = logLock.Acquire();

	LogMessageRaw(GetLogLevelAttribs(logLevel));
	LogMessageRaw(std::format("{:011.05f} {:5}", GetLogTime(), GetLogLevelString(logLevel)));
	LogMessageRaw(GetLogLevelAttribs(NvLogLevel::INTERNAL_Reset));
	LogMessageRaw(" | ");
	LogMessageRaw(GetLogLevelAttribs(NvLogLevel::INTERNAL_FuncName));
	LogMessageRaw(funcName, (unsigned int)strlen(funcName));
	LogMessageRaw(GetLogLevelAttribs(NvLogLevel::INTERNAL_Reset));
	LogMessageRaw(": ");
	LogMessageRaw(message);
	LogMessageRaw("\n");

	return 0;
}

std::vector<unsigned char> NvBinaryStreamRead::ReadRemainingBytes()
{
	std::vector<unsigned char> retval(this->data + this->currentPos, this->data + this->dataLength);
	this->currentPos = this->dataLength;

	return retval;
}

void NvBinaryStreamRead::ReadBytes(void* buf, size_t len)
{
	if (this->currentPos + len > this->dataLength)
		throw NvStreamException(std::format("Read overrun of {} bytes while at byte position {}/{}", len, this->currentPos, this->dataLength));

	memcpy(buf, this->data + this->currentPos, len);
	this->currentPos += len;
}

void NvBinaryStreamRead::ReadBytesEndianAware(void* buf, size_t len)
{
	this->ReadBytes(buf, len);
	if (this->swappedEndianness || this->swappedEndiannessForNextOp)
		std::reverse((unsigned char*)buf, (unsigned char*)buf + len);

	this->swappedEndiannessForNextOp = false;
}

std::vector<unsigned char> NvBinaryStreamRead::ReadBytes(size_t len)
{
	if (this->currentPos + len > this->dataLength)
		throw NvStreamException(std::format("Read overrun of {} bytes while at byte position {}/{}", len, this->currentPos, this->dataLength));

	std::vector<unsigned char> retval;
	retval.resize(len);

	this->ReadBytes(retval.data(), len);
	return retval;
}

void NvBinaryStreamRead::SkipBytes(size_t byteCount)
{
	if (this->currentPos + byteCount > this->dataLength)
		throw NvStreamException(std::format("Read overrun of {} bytes (skipping) while at byte position {}/{}", byteCount, this->currentPos, this->dataLength));

	this->currentPos += byteCount;
}

unsigned long long NvBinaryStreamRead::ReadVarInt64()
{
	unsigned long long val = 0;

	for (int i = 0; i < 9; ++i)
	{
		unsigned long long c = this->Read<unsigned char>();
		val |= (c & 0x7fu) << (7 * i);

		if ((c & 0x80) == 0)
			return val;
	}

	throw NvStreamException("VarInt64 read exceeded byte limit!");
}

void NvBinaryStreamWrite::WriteBytesEndianAware(const void* buf, size_t len)
{
	this->WriteBytes(buf, len);
	if (this->swappedEndianness || this->swappedEndiannessForNextOp)
		std::reverse(this->buffer.rbegin(), this->buffer.rbegin() + len);

	this->swappedEndiannessForNextOp = false;
}

void NvBinaryStreamWrite::WritePaddedString(const std::string& s, size_t maxLength)
{
	if (s.size() <= maxLength)
	{
		WriteBytes(s.data(), s.size());
		WriteRepeatByte(0, maxLength - s.size());
	}
	else
	{
		WriteBytes(s.data(), maxLength);
	}
}

void NvBinaryStreamWrite::WriteVarInt64(unsigned long long val)
{
	do {
		unsigned char thisByte = (unsigned char)(val & 0x7f);
		val >>= 7;

		if (val)
			thisByte |= 0x80;

		this->buffer.push_back(thisByte);
	} while (val);
}

std::wstring NvLib::NvUtils::UTF8ToWide(const std::string& s)
{
	int wideLength = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
	if (wideLength == 0)
		return L"";

	std::wstring wide(wideLength, 0);
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &wide[0], wideLength);

	return wide;
}

std::string NvLib::NvUtils::WideToUTF8(const std::wstring& s)
{
	int utf8Length = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0, nullptr, nullptr);
	if (utf8Length == 0)
		return "";

	std::string utf8(utf8Length, 0);
	WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), &utf8[0], utf8Length, nullptr, nullptr);

	return utf8;
}

std::string NvLib::NvUtils::Base64Encode(const std::string& s, bool useUrlSafeCharacters)
{
	static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static const unsigned char base64_table_urlSafe[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

	const unsigned char* lookupTable = useUrlSafeCharacters ? base64_table_urlSafe : base64_table;

	unsigned char* src = (unsigned char*)s.data();
	size_t len = s.size();

	unsigned char* out, * pos;
	const unsigned char* end, * in;

	size_t olen;

	olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

	if (olen < len)
		return std::string(); /* integer overflow */

	std::string outStr;
	outStr.resize(olen);
	out = (unsigned char*)&outStr[0];

	end = src + len;
	in = src;
	pos = out;
	while (end - in >= 3) {
		*pos++ = lookupTable[in[0] >> 2];
		*pos++ = lookupTable[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = lookupTable[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = lookupTable[in[2] & 0x3f];
		in += 3;
	}

	if (end - in) {
		*pos++ = lookupTable[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = lookupTable[(in[0] & 0x03) << 4];
			*pos++ = useUrlSafeCharacters ? '.' : '=';
		}
		else {
			*pos++ = lookupTable[((in[0] & 0x03) << 4) |
				(in[1] >> 4)];
			*pos++ = lookupTable[(in[1] & 0x0f) << 2];
		}
		*pos++ = useUrlSafeCharacters ? '.' : '=';
	}

	return outStr;
}

std::string NvLib::NvUtils::Base64Decode(const std::string& s)
{
	static const int B64index[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 63, 62, 62, 63, 52, 53, 54, 55, 56, 57,
		58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 63, 0, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};

	unsigned char* p = (unsigned char*)s.data();
	size_t len = s.size();
	int pad = len > 0 && (len % 4 || p[len - 1] == '=');
	const size_t L = ((len + 3) / 4 - pad) * 4;
	std::string str(L / 4 * 3 + pad, '\0');

	for (size_t i = 0, j = 0; i < L; i += 4)
	{
		int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
		str[j++] = n >> 16;
		str[j++] = n >> 8 & 0xFF;
		str[j++] = n & 0xFF;
	}
	if (pad)
	{
		int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
		str[str.size() - 1] = n >> 16;

		if (len > L + 2 && p[L + 2] != '=')
		{
			n |= B64index[p[L + 2]] << 6;
			str.push_back(n >> 8 & 0xFF);
		}
	}
	return str;
}

void NvLib::InitializeMain()
{
	NvLogger::InitializeLogger();

	wchar_t exePath[MAX_PATH];
	if (GetModuleFileNameW(nullptr, exePath, MAX_PATH))
		g_ExeFullPath = exePath;
}
