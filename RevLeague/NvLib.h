#pragma once

#include <Windows.h>
#include <string>
#include <format>
#include <vector>
#include <stdexcept>

using namespace std::string_literals;

// ====================================================== Generic types ======================================================

class NvNonCopyable {
public:
	inline NvNonCopyable() {}
	NvNonCopyable(NvNonCopyable const&) = delete;
	NvNonCopyable& operator=(NvNonCopyable const&) = delete;
};

class NvException : public std::exception {
	std::string msg;

protected:
	NvException(const std::string& desc) : msg(desc) {}

public:
	inline void AppendToMessage(const std::string& data) { msg += data; }
	virtual const char* what() const noexcept { return msg.c_str(); }
};

class NvSystemException : public NvException {
public:
	NvSystemException(const std::string& desc) : NvException("NvSystemException: " + desc) {}
};

class NvStreamException : public NvException {
public:
	NvStreamException(const std::string& desc) : NvException("NvStreamException: " + desc) {}
};

class NvProtocolException : public NvException {
public:
	NvProtocolException(const std::string& desc) : NvException("NvProtocolException: " + desc) {}
};

// ====================================================== Win32 wrappers ======================================================

class NvAutoCriticalSection : public NvNonCopyable {
private:
	CRITICAL_SECTION* cs;

public:
	inline void Release()
	{
		if (!cs)
			return;

		LeaveCriticalSection(this->cs);
		cs = nullptr;
	}

	inline explicit NvAutoCriticalSection(CRITICAL_SECTION* cs_) : cs(cs_) { EnterCriticalSection(this->cs); }
	inline ~NvAutoCriticalSection() { Release(); }

	inline NvAutoCriticalSection(NvAutoCriticalSection&& other) noexcept { this->cs = other.cs; other.cs = nullptr; }
	inline NvAutoCriticalSection& operator=(NvAutoCriticalSection&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			this->cs = rhs.cs;
			rhs.cs = nullptr;
		}

		return *this;
	}
};

class NvCriticalSection : public NvNonCopyable {
private:
	CRITICAL_SECTION cs;

public:
	inline NvCriticalSection() { InitializeCriticalSection(&this->cs); }
	inline ~NvCriticalSection() { DeleteCriticalSection(&this->cs); }

	[[nodiscard]] inline NvAutoCriticalSection Acquire() { return NvAutoCriticalSection(&this->cs); }
};

class NvAutoWin32Handle : public NvNonCopyable {
private:
	HANDLE handle;

	template <bool ReleaseExisting>
	inline void AssignHandle(HANDLE handle)
	{
		if constexpr (ReleaseExisting)
			ReleaseHandle();

		this->handle = handle;
	}

	inline void ReleaseHandle()
	{
		if (IsValidHandle())
			CloseHandle(this->handle);

		this->handle = INVALID_HANDLE_VALUE;
	}

public:
	inline NvAutoWin32Handle(HANDLE handle) { AssignHandle<false>(handle); }
	inline NvAutoWin32Handle() : NvAutoWin32Handle(INVALID_HANDLE_VALUE) {}
	inline ~NvAutoWin32Handle() { ReleaseHandle(); }

	inline NvAutoWin32Handle(NvAutoWin32Handle&& other) noexcept { this->handle = other.handle; other.handle = nullptr; }

	inline NvAutoWin32Handle& operator=(HANDLE rhs)
	{
		AssignHandle<true>(rhs);
		return *this;
	}

	inline bool IsValidHandle() const noexcept
	{
		return this->handle != nullptr && this->handle != INVALID_HANDLE_VALUE;
	}

	inline HANDLE Get() const noexcept { return this->handle; }
	[[nodiscard]] inline HANDLE Abandon() noexcept { HANDLE h = this->handle; this->handle = nullptr; return h; }
};

// ====================================================== Binary streams ======================================================

enum class NvStreamEndianness {
	BigEndian,
	LittleEndian
};

class NvBinaryStreamRead final {
private:
	unsigned char* data = nullptr;
	size_t dataLength = 0;
	size_t currentPos = 0;

	bool swappedEndianness = false;
	bool swappedEndiannessForNextOp = false;

	template <typename T>
	T ReadPrimitive()
	{
		T value;
		ReadBytesEndianAware(&value, sizeof(T));

		return value;
	}

public:
	inline NvStreamEndianness GetEndianness() const { return swappedEndianness ? NvStreamEndianness::BigEndian : NvStreamEndianness::LittleEndian; }
	inline void SetEndianness(NvStreamEndianness endianness) { swappedEndianness = endianness == NvStreamEndianness::BigEndian; }
	inline void SetEndiannessForNextOperation(NvStreamEndianness endianness) { swappedEndiannessForNextOp = endianness == NvStreamEndianness::BigEndian; }

	inline bool EndOfDataReached() const { return currentPos >= dataLength; }
	inline size_t GetCurrentPos() const { return currentPos; };
	inline size_t GetRemainingBytes() const { return EndOfDataReached() ? 0 : dataLength - currentPos; }

	std::vector<unsigned char> ReadRemainingBytes();

	void ReadBytes(void* buf, size_t len);
	void ReadBytesEndianAware(void* buf, size_t len);
	std::vector<unsigned char> ReadBytes(size_t len);

	void SkipBytes(size_t byteCount);

	unsigned long long ReadVarInt64();

	template <typename T>
	T Read();

	inline NvBinaryStreamRead(unsigned char* data_, size_t dataLength_) : data(data_), dataLength(dataLength_)
	{
		if (dataLength_ > 0x7fffffffu)
			throw NvStreamException(std::format("Too long NvBinaryStreamRead of length {}", 0x7fffffffu));
	}
};

template<> inline char NvBinaryStreamRead::Read() { return ReadPrimitive<char>(); }
template<> inline unsigned char NvBinaryStreamRead::Read() { return ReadPrimitive<unsigned char>(); }
template<> inline short NvBinaryStreamRead::Read() { return ReadPrimitive<short>(); }
template<> inline unsigned short NvBinaryStreamRead::Read() { return ReadPrimitive<unsigned short>(); }
template<> inline int NvBinaryStreamRead::Read() { return ReadPrimitive<int>(); }
template<> inline unsigned int NvBinaryStreamRead::Read() { return ReadPrimitive<unsigned int>(); }
template<> inline float NvBinaryStreamRead::Read() { return ReadPrimitive<float>(); }
template<> inline double NvBinaryStreamRead::Read() { return ReadPrimitive<double>(); }
template<> inline long long NvBinaryStreamRead::Read() { return ReadPrimitive<long long>(); }
template<> inline unsigned long long NvBinaryStreamRead::Read() { return ReadPrimitive<unsigned long long>(); }

class NvBinaryStreamWrite final {
private:
	std::vector<unsigned char> buffer;

	bool swappedEndianness = false;
	bool swappedEndiannessForNextOp = false;

public:
	inline NvStreamEndianness GetEndianness() const { return swappedEndianness ? NvStreamEndianness::BigEndian : NvStreamEndianness::LittleEndian; }
	inline void SetEndianness(NvStreamEndianness endianness) { swappedEndianness = endianness == NvStreamEndianness::BigEndian; }
	inline void SetEndiannessForNextOperation(NvStreamEndianness endianness) { swappedEndiannessForNextOp = endianness == NvStreamEndianness::BigEndian; }

	inline const std::vector<unsigned char>& GetUnderlyingBuffer() const { return buffer; }

	void WriteRepeatByte(unsigned char byte, size_t len);
	void WriteBytes(const void* buf, size_t len);
	void WriteBytesEndianAware(const void* buf, size_t len);
	
	void WritePaddedString(const std::string& s, size_t maxLength);
	void WriteVarInt64(unsigned long long val);

	template <typename T>
	void Write(T val);

	inline NvBinaryStreamWrite() {}
	inline NvBinaryStreamWrite(size_t reservedBytes) : buffer() { this->buffer.reserve(reservedBytes); }
};

template<> inline void NvBinaryStreamWrite::Write(char val) { return WriteBytesEndianAware(&val, sizeof(char)); }
template<> inline void NvBinaryStreamWrite::Write(unsigned char val) { return WriteBytesEndianAware(&val, sizeof(unsigned char)); }
template<> inline void NvBinaryStreamWrite::Write(short val) { return WriteBytesEndianAware(&val, sizeof(short)); }
template<> inline void NvBinaryStreamWrite::Write(unsigned short val) { return WriteBytesEndianAware(&val, sizeof(unsigned short)); }
template<> inline void NvBinaryStreamWrite::Write(int val) { return WriteBytesEndianAware(&val, sizeof(int)); }
template<> inline void NvBinaryStreamWrite::Write(unsigned int val) { return WriteBytesEndianAware(&val, sizeof(unsigned int)); }
template<> inline void NvBinaryStreamWrite::Write(float val) { return WriteBytesEndianAware(&val, sizeof(float)); }
template<> inline void NvBinaryStreamWrite::Write(double val) { return WriteBytesEndianAware(&val, sizeof(double)); }
template<> inline void NvBinaryStreamWrite::Write(long long val) { return WriteBytesEndianAware(&val, sizeof(long long)); }
template<> inline void NvBinaryStreamWrite::Write(unsigned long long val) { return WriteBytesEndianAware(&val, sizeof(unsigned long long)); }
template<> void NvBinaryStreamWrite::Write(std::string& val);

// ====================================================== Logger ======================================================

enum class NvLogLevel {
	Debug,
	Info,
	Warning,
	Error,
	INTERNAL_FuncName,
	INTERNAL_Reset
};

namespace NvLogger {
	void InitializeLogger();

	void PrintHexBuffer(const unsigned char* data, size_t dataLength);

	int LogAssert2(const char* cond, const char* funcName, const char* file, int line);
	int LogAssert2(const char* cond, const char* funcName, const char* file, int line, unsigned int lastError);
	int LogMessage(NvLogLevel logLevel, const char* funcName, const std::string& message);
}

#define NvLogDebug(msg, ...) NvLogger::LogMessage(NvLogLevel::Debug, __FUNCTION__, std::format(msg, __VA_ARGS__))
#define NvLogInfo(msg, ...) NvLogger::LogMessage(NvLogLevel::Info, __FUNCTION__, std::format(msg, __VA_ARGS__))
#define NvLogWarning(msg, ...) NvLogger::LogMessage(NvLogLevel::Warning, __FUNCTION__, std::format(msg, __VA_ARGS__))
#define NvLogError(msg, ...) NvLogger::LogMessage(NvLogLevel::Error, __FUNCTION__, std::format(msg, __VA_ARGS__))

// returns 0 if assertion failed (note: these "assertions" are NOT to be disabled in release builds!)
#define NvLogAssert(cond) (!!(cond) || NvLogger::LogAssert2(#cond, __FUNCTION__, __FILE__, __LINE__))
#define NvLogAssertLastError(cond, err) (!!(cond) || NvLogger::LogAssert2(#cond, __FUNCTION__, __FILE__, __LINE__, (unsigned int)(err)))

// shorthands
#define LogDebug NvLogDebug
#define LogInfo NvLogInfo
#define LogWarning NvLogWarning
#define LogError NvLogError
#define LogAssert NvLogAssert
#define LogAssertLastError NvLogAssertLastError

// ====================================================== Utils ======================================================

namespace NvLib {
	namespace NvUtils {
		std::wstring UTF8ToWide(const std::string& s);
		std::string WideToUTF8(const std::wstring& s);

		std::string Base64Encode(const std::string& s, bool useUrlSafeCharacters);
		std::string Base64Decode(const std::string& s);
	}
}

// ====================================================== Runtime ======================================================

namespace NvLib {
	void InitializeMain();
}

inline std::wstring g_ExeFullPath;
