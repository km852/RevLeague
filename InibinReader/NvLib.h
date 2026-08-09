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

enum class NvStreamEndianness {
	BigEndian,
	LittleEndian
};

class NvBinaryStreamRead final {
private:
	const unsigned char* data = nullptr;
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

	inline NvBinaryStreamRead(const unsigned char* data_, size_t dataLength_) : data(data_), dataLength(dataLength_)
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
