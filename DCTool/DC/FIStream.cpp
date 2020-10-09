#include "../App.h"

FIStream::FIStream()
{
	_raw = 0;
	_size = 0;
	_pos = 0;
}

FIStream::FIStream(std::istream * str)
{
	str->seekg(0, std::istream::end);
	_size = (u_int64)str->tellg();
	str->seekg(0, std::istream::beg);

	_raw = new uint8[_size];
	str->read((char*)_raw, _size);
	_pos = 0;
}

FIStream::FIStream(u_int64 size)
{
	_size = size;
	_raw = (uint8*)malloc(size);
	memset((void*)_raw, 0, _size);
	_pos = 0;
}

FIStream::FIStream(uint8 * data, u_int64 size)
{
	_size = size;
	_pos = 0;
	_raw = new uint8[_size];
	if (memcpy_s((void*)_raw, _size, (const void*)data, _size) != 0)
	{
		if (_raw)
		{
			delete[] _raw;
			_raw = 0;

		}
		return;
	}
}

FIStream::FIStream(FIStream & d)
{
	_size = _pos = 0;
	_raw = nullptr;

	Resize(d._size);
	Write(d._raw, d._size);
}

FIStream::~FIStream()
{
	if (_raw)
	{
		delete[] _raw;
		_raw = 0;
	}
	_size = 0;
	_pos = 0;
}

bool FIStream::load_from_file(const char *file){
	Clear();
	std::ifstream f = std::ifstream(file, std::fstream::binary);
	if (!f.is_open())
		return  false;
	if (!f.good()) {
		f.close();
		return  false;
	}

	f.seekg(0, std::fstream::end);
	_size = (u_int64)f.tellg();
	f.seekg(0, std::fstream::beg);

	_raw = new uint8[_size];
	if (!_raw) {
		f.close();
		return false;
	}

	f.read((char*)_raw, _size);
	f.close();
	return true;
}

bool FIStream::save_to_file(const char * fileName)
{
	std::ofstream file;
	file.open(fileName, std::fstream::binary | std::fstream::out | std::fstream::trunc);
	if (!file.is_open()) {
		return false;
	}

	file.write((const char*)_raw, _size);

	file.close();

	return true;
}


//WRITE_*******************************************************
void FIStream::Resize(u_int64 size)
{
	if (size <= 0 || _size + size > UINT64_MAX)
		return;

	if (!_raw)
	{
		_raw = new uint8[size];
		_size = size;
		memset((void*)_raw, 0, _size);
	}
	else
	{
		u_int64 newSize = _size + size;
		uint8 * newRaw = (uint8*)realloc((void*)_raw, newSize);

		memset((void*)&newRaw[_size], 0, size);
		_size = newSize;
		_raw = newRaw;
	}
}

void FIStream::Write(uint8 * data, u_int64 size)
{
	if (_size + size > UINT64_MAX) //0xffff because the packets header has only 2 bytes [00 00] for the size
		return;

	_int64 allocSzie = size - _size + _pos;

	if (allocSzie > 0)
		Resize(allocSzie);

	if (memcpy_s((void*)&_raw[_pos], size, (const void*)data, size) != 0) return;


	if (allocSzie > 0)
	{
		_pos = _size;
	}
	else {
		_pos += size;
	}
}

void FIStream::WriteString(std::string s, bool reverse)
{
	uint8 * data = new uint8[s.size() * 2];
	uint8 * pos = &data[0];

	for (size_t i = 0; i < s.size(); i++)
	{
		if (!reverse)
		{
			pos[0] = (uint8)s[i];
			pos[1] = 0x00;
		}
		else
		{
			pos[1] = (uint8)s[i];
			pos[0] = 0x00;
		}
		pos += 2;
	}

	Write(data, (uint32)(s.size() * 2));
	WriteInt16(0); // string end
}

void FIStream::WriteUInt8(uint8 val)
{
	Write(&val, 1);
}

void FIStream::WriteInt16(int16 data)
{
	uint8 shortBytes[2];
	shortBytes[0] = (data & 0xff);
	shortBytes[1] = ((data >> 8) & 0xff);

	Write(shortBytes, 2);
}

void FIStream::WriteUInt16(uint16 val)
{
	uint8 shortBytes[2];
	shortBytes[0] = (val & 0xff);
	shortBytes[1] = ((val >> 8) & 0xff);
	Write(shortBytes, 2);
}

void FIStream::WriteInt32(int32 data)
{
	uint8 intBytes[4];
	intBytes[0] = data & 0x000000ff;
	intBytes[1] = (data & 0x0000ff00) >> 8;
	intBytes[2] = (data & 0x00ff0000) >> 16;
	intBytes[3] = (data & 0xff000000) >> 24;

	Write(intBytes, 4);
}

void FIStream::WriteUInt32(int32 data)
{
	uint8 intBytes[4];
	intBytes[0] = data & 0x000000ff;
	intBytes[1] = (data & 0x0000ff00) >> 8;
	intBytes[2] = (data & 0x00ff0000) >> 16;
	intBytes[3] = (data & 0xff000000) >> 24;

	Write(intBytes, 4);
}

void FIStream::Write_int64(_int64 data)
{
	uint8 intBytes[8];

	intBytes[0] = ((data >> 0) & 0xff);
	intBytes[1] = ((data >> 8) & 0xff);
	intBytes[2] = ((data >> 16) & 0xff);
	intBytes[3] = ((data >> 24) & 0xff);
	intBytes[4] = ((data >> 32) & 0xff);
	intBytes[5] = ((data >> 40) & 0xff);
	intBytes[6] = ((data >> 48) & 0xff);
	intBytes[7] = ((data >> 56) & 0xff);

	Write(intBytes, 8);
}

void FIStream::WriteU_int64(u_int64 data)
{
	uint8 intBytes[8];

	intBytes[0] = ((data >> 0) & 0xff);
	intBytes[1] = ((data >> 8) & 0xff);
	intBytes[2] = ((data >> 16) & 0xff);
	intBytes[3] = ((data >> 24) & 0xff);
	intBytes[4] = ((data >> 32) & 0xff);
	intBytes[5] = ((data >> 40) & 0xff);
	intBytes[6] = ((data >> 48) & 0xff);
	intBytes[7] = ((data >> 56) & 0xff);

	Write(intBytes, 8);
}

void FIStream::WriteFloat(float val)
{
	Write(reinterpret_cast<uint8*>(&val), 4);
}

void FIStream::WriteDouble(double val)
{
	uint8 * doubleBytes = reinterpret_cast<uint8*>(&val);
	Write(doubleBytes, 4);
}

void FIStream::WritePos(uint16 s, int16 offset)
{
	uint16 temp = _pos;
	_pos = s;
	WriteInt16(temp + offset);
	_pos = temp;
}



//READ_********************************************************
void FIStream::Read(uint8 * out_buffer, u_int64 size)
{
	if (size > _size - _pos)
	{
		_int64 temp = (_size - _pos);

		memcpy_s((void*)out_buffer, size, (const void*)&_raw[_pos], temp);
		_pos = temp;

		return;
	}
	memcpy_s((void*)out_buffer, size, (const void*)&_raw[_pos], size);
	_pos += size;
}

char FIStream::ReadInt8()
{
	return _raw[_pos++];
}

uint8 FIStream::ReadUInt8()
{
	return _raw[_pos++];
}

int16 FIStream::ReadInt16()
{
	if (_pos > _size - 2)
		return -1;

	int16 out = (_raw[_pos + 1] << 8) | _raw[_pos];
	_pos += 2;
	return out;
}

uint16 FIStream::ReadUInt16()
{
	if (_pos > _size - 2)
		return -1;

	uint16 out = (_raw[_pos + 1] << 8) | _raw[_pos];
	_pos += 2;
	return out;
}

int32 FIStream::ReadInt32()
{
	if (_size - 4 < _pos)
		return INT_MAX;

	int32 out = ((_raw[_pos + 3] << 24) | (_raw[_pos + 2] << 16) | (_raw[_pos + 1] << 8) | _raw[_pos]);
	_pos += 4;

	return out;
}

uint32 FIStream::ReadUInt32()
{
	if (_size - 4 < _pos)
		return INT_MAX;

	uint32 out = ((_raw[_pos + 3] << 24) | (_raw[_pos + 2] << 16) | (_raw[_pos + 1] << 8) | _raw[_pos]);
	_pos += 4;

	return out;
}

_int64 FIStream::ReadInt64()
{
	if (_size < _pos + 8)
		return INT64_MAX;

	_int64 l = 0;
	for (uint8 i = 0; i < 8; i++) {
		l = l | ((unsigned long long)_raw[_pos + i] << (8 * i));
	}
	_pos += 8;
	return l;
}

u_int64 FIStream::ReadUInt64()
{
	if (_size < _pos + 8)
		return INT64_MAX;

	u_int64 l = 0;
	for (uint8 i = 0; i < 8; i++) {
		l = l | ((unsigned long long)_raw[_pos + i] << (8 * i));
	}
	_pos += 8;
	return l;
}

float FIStream::ReadFloat()
{
	if (_size < _pos + 4)
		return FLT_MAX;

	float out = *(float*)(&_raw[_pos]);
	_pos += 4;
	return out;
}

double FIStream::ReadDouble()
{
	if (_size < _pos + 4)
		return FLT_MAX;

	double out = *(double*)(&_raw[_pos]);
	_pos += 4;
	return out;
}

std::string FIStream::ReadUTF16StringBigEdianToASCII() {

	std::string out;
	int32 count = 0;
	for (size_t i = _pos; i < _size; i += 2)
	{
		if (_raw[i] == 0x00 && _raw[i + 1] == 0x00)
			break;
		count += 2;
		out.push_back(_raw[i]);
	}
	count += 2;
	_pos += count;

	return out;
}

std::string FIStream::ReadASCIIString()
{
	if (_size - _pos < 1)
		return "-1";

	std::string out;
	int32 kk = 0;
	for (size_t i = _pos; i < _size; i++)
	{
		if (_raw[i] != 0x00)
		{
			if (_raw[i] != '\r' || _raw[i] != '\n')
			{
				out += (char)_raw[i];
			}
			kk++;
		}
		else
			break;
	}
	_pos += kk;
	return out;
}

bool FIStream::ReadASCIIStringTo(uint8 * out, uint16 max_len)
{
	if (!out)
		return false;

	std::string text = ReadUTF16StringBigEdianToASCII();
	if (text.size() > max_len)
		return false;


	return memcpy(out, text.c_str(), text.size() + 1) ? true : false;
}

std::string FIStream::ReadUTF16StringLittleEdianToASCII()
{
	std::string out;
	for (size_t i = _pos; i < _size; i += 2)
	{
		if (_raw[i] == 0x00 && _raw[i + 1] == 0x00)
			break;

		out.push_back(_raw[i + 1]);
	}
	_pos += (uint16)(out.size() * 2) + 2;

	return out;
}


//MISC_**************************************************
void FIStream::Clear()
{
	if (_raw)
	{
		delete[] _raw;
		_raw = 0;
	}
	_size = 0;
	_pos = 0;
}

void FIStream::Zero(){
	memset(_raw, 0, sizeof(uint8)*_size);
}

u_int64 FIStream::SetEnd()
{
	_pos = _size;
	return _pos;
}

u_int64 FIStream::SetFront()
{
	_pos = 0;
	return 0;

}

u_int64 FIStream::NextPos()
{
	uint16 tempPos = _pos;
	WriteInt16(0);
	return tempPos;
}

uint8 * FIStream::GetPtr()
{
	if (!_raw) {
		return nullptr;
	}

	return &_raw[_pos];
}

