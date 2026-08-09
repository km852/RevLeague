#include "NvLib.h"

#include <cstring>
#include <format>
#include <string>

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

void NvBinaryStreamWrite::WriteRepeatByte(unsigned char byte, size_t len)
{
	this->buffer.insert(this->buffer.end(), len, byte);
}

void NvBinaryStreamWrite::WriteBytes(const void* buf, size_t len)
{
	this->buffer.insert(this->buffer.end(), (unsigned char*)buf, (unsigned char*)buf + len);
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

template<> void NvBinaryStreamWrite::Write(std::string& val)
{
	this->WriteVarInt64(val.size());
	this->WriteBytes(val.data(), val.size());
}
