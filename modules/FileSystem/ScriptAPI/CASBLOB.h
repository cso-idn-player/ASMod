#ifndef FILESYSTEM_SCRIPTAPI_CASBLOB_H
#define FILESYSTEM_SCRIPTAPI_CASBLOB_H

#include <string>
#include <vector>

#include <Angelscript/util/CASBaseClass.h>

class asIScriptEngine;

/**
*	A Binary Large OBject, a buffer that can be read from or written to.
*	Has a size and a capacity, allowing it to reserve space ahead of time to save memory allocation costs.
*/
class CASBLOB : public CASRefCountedBaseClass
{
private:
	typedef std::vector<byte> Buffer_t;

public:
	/**
	*	Creates a BLOB with the given starting capacity.
	*	@param uiSize Starting capacity, in bytes.
	*/
	CASBLOB( size_t uiCapacity = 0 );

	CASBLOB( const CASBLOB& other );
	CASBLOB& operator=( const CASBLOB& other );

	~CASBLOB();

	void Release() const;

	/**
	*	@return The data pointer.
	*/
	const byte* GetData() const { return m_Buffer.data(); }

	/**
	*	@copydoc GetData() const
	*/
	byte* GetData() { return m_Buffer.data(); }

	/**
	*	@return The total capacity of the buffer, in bytes.
	*/
	size_t GetCapacity() const { return m_Buffer.capacity(); }

	/**
	*	@return The size of the buffer, in bytes. This is the amount of data that has been written to the buffer.
	*/
	size_t GetSize() const { return m_Buffer.size(); }
	
	/**
	*	@return The current read offset.
	*/
	size_t GetReadOffset() const { return m_uiReadOffset; }

	/**
	*	@return Whether the buffer's read pointer has reached the end.
	*/
	bool ReadReachedEnd() const { return m_uiReadOffset >= m_Buffer.size(); }

	/**
	*	@return Whether reading the given number of bytes is valid.
	*/
	bool ReadSizeValid( size_t uiSizeInBytes ) const
	{
		//Could overflow if m_uiReadOffset is close to SIZE_MAX, so subtract instead.
		return uiSizeInBytes <= m_Buffer.size() && m_uiReadOffset <= ( m_Buffer.size() - uiSizeInBytes );
	}

	//Write operations
	/**
	*	Writes a number of bytes from a buffer.
	*	@param pData Buffer to copy from. Must be valid.
	*	@param uiSizeInBytes Number of bytes to write.
	*	@return Whether the data was successfully written into the buffer.
	*/
	bool Write( const void* pData, size_t uiSizeInBytes );

	/**
	*	Writes an instance of type T to the buffer.
	*	@param data Data to write.
	*	@return Whether data was successfully written.
	*	@tparam T Type of the data to write.
	*/
	template<typename T>
	bool WriteData( const T& data )
	{
		return Write( &data, sizeof( T ) );
	}

	/**
	*	Writes a number of bytes to the buffer. The bytes have the given value.
	*	@param uiSizeInBytes Number of bytes to write.
	*	@param iValue Integer value to assign. The value is converted to a uint8.
	*	@return Whether data was successfully written.
	*/
	bool WriteBytes( size_t uiSizeInBytes, int iValue );

	/**
	*	Writes a number of bytes until the given end offset is reached.
	*	@param uiEndOffset Absolute offset into the buffer where the writing should end.
	*	@param iValue Integer value to assign. The value is converted to a uint8.
	*	@return Whether data was successfully written. Returns false if uiEndOffset is positioned before or at the current write offset.
	*/
	bool WriteBytesUntil( size_t uiEndOffset, int iValue );

	/**
	*	Writes an unsigned 8 bit integer.
	*/
	bool Write( asBYTE data );

	/**
	*	Writes an unsigned 16 bit integer.
	*/
	bool Write( asWORD data );

	/**
	*	Writes an unsigned 32 bit integer.
	*/
	bool Write( asDWORD data );

	/**
	*	Writes an unsigned 64 bit integer.
	*/
	bool Write( asQWORD data );

	/**
	*	Writes a single precision float.
	*/
	bool Write( float data );

	/**
	*	Writes a double precision float.
	*/
	bool Write( double data );

	/**
	*	Writes a fixed length string. If the string is smaller than uiSizeInBytes, the remainder is padded with pad.
	*	@param szString to write.
	*	@param uiSizeInBytes maximum number of bytes to write, including the null terminator.
	*	@param pad Character to use to pad the remainder of the string.
	*	@return Whether the string could be written. Returns false if uiSizeInBytes is 0.
	*/
	bool Write( const std::string& szString, size_t uiSizeInBytes, const char pad = '\0' );

	/**
	*	Writes a variable length string.
	*	@param szString to write.
	*	@return Whether the string could be written.
	*/
	bool Write( const std::string& szString );

	//Read operations
	/**
	*	Reads a number of bytes from the buffer. If less than uiSizeInBytes data is available, returns false.
	*	@param pData Pointer to the destination buffer. Must be valid.
	*	@param uiSizeInBytes Number of bytes to read into the destination buffer.
	*	@return Whether data was successfully read into the buffer.
	*/
	bool ReadData( void* pData, size_t uiSizeInBytes );

	/**
	*	Reads an instance of type T from the buffer.
	*	@param bSuccess Whether data was successfully read.
	*	@return The data that was read from the buffer.
	*	@tparam T Type of the data to read. Must be default constructible.
	*/
	template<typename T>
	T ReadData( bool& bSuccess )
	{
		T data{};

		bSuccess = ReadData( &data, sizeof( T ) );

		return data;
	}

	/**
	*	Reads an instance of type T from the buffer.
	*	@return The data that was read from the buffer.
	*	@tparam T Type of the data to read. Must be default constructible.
	*/
	template<typename T>
	T ReadData()
	{
		bool bSuccess;

		return ReadData<T>( bSuccess );
	}

	/**
	*	Reads an 8 bit unsigned integer.
	*/
	asBYTE ReadUInt8();

	/**
	*	Reads a 16 bit unsigned integer.
	*/
	asWORD ReadUInt16();

	/**
	*	Reads a 32 bit unsigned integer.
	*/
	asDWORD ReadUInt32();

	/**
	*	Reads a 64 bit unsigned integer.
	*/
	asQWORD ReadUInt64();

	/**
	*	Reads a single precision floating point value.
	*/
	float ReadFloat();

	/**
	*	Reads a double precision floating point value.
	*/
	double ReadDouble();

	/**
	*	Reads an 8 bit unsigned integer.
	*	@param[ out ] bSuccess Whether the value was successfully read.
	*/
	asBYTE ReadUInt8( bool& bSuccess );

	/**
	*	Reads a 16 bit unsigned integer.
	*	@param[ out ] bSuccess Whether the value was successfully read.
	*/
	asWORD ReadUInt16( bool& bSuccess );

	/**
	*	Reads a 32 bit unsigned integer.
	*	@param[ out ] bSuccess Whether the value was successfully read.
	*/
	asDWORD ReadUInt32( bool& bSuccess );

	/**
	*	Reads a 64 bit unsigned integer.
	*	@param[ out ] bSuccess Whether the value was successfully read.
	*/
	asQWORD ReadUInt64( bool& bSuccess );

	/**
	*	Reads a single precision floating pointer value.
	*	@param[ out ] bSuccess Whether the value was successfully read.
	*/
	float ReadFloat( bool& bSuccess );

	/**
	*	Reads a double precision floating pointer value.
	*	@param[ out ] bSuccess Whether the value was successfully read.
	*/
	double ReadDouble( bool& bSuccess );

	/**
	*	Reads a fixed length string.
	*	@param uiSizeInbytes Maximu number of bytes to read, including the null terminator.
	*	@param[ out ] bSuccess Whether the string was successfully read.
	*/
	std::string ReadString( size_t uiSizeInBytes, bool& bSuccess );

	/**
	*	Reads a fixed length string.
	*	@param uiSizeInbytes Maximu number of bytes to read, including the null terminator.
	*/
	std::string ReadString( size_t uiSizeInBytes );

	/**
	*	Reads a variable length string.
	*	@param[ out ] bSuccess Whether the string was successfully read.
	*/
	std::string ReadString( bool& bSuccess );

	/**
	*	Reads a variable length string.
	*/
	std::string ReadString();

	//Size changing operations
	/**
	*	Reserves at least uiMinimumCapacity bytes in the buffer.
	*/
	void Reserve( size_t uiMinimumCapacity );

	/**
	*	Resizes the buffer to uiNewSize. If the read offset exceeds this size, it is set to the new size.
	*	If the new size is greater than the old size, the buffer is padded with zeroes.
	*/
	void Resize( size_t uiNewSize );

	/**
	*	Shrinks the buffer's capacity to fit the current size.
	*/
	void ShrinkToFit();

	/**
	*	Clears the buffer and resets the read offset.
	*/
	void Clear();

private:
	void Copy( const CASBLOB& other );

private:
	Buffer_t m_Buffer;

	size_t m_uiReadOffset = 0;
};

inline bool CASBLOB::Write( asBYTE data )
{
	return WriteData( data );
}

inline bool CASBLOB::Write( asWORD data )
{
	return WriteData( data );
}

inline bool CASBLOB::Write( asDWORD data )
{
	return WriteData( data );
}

inline bool CASBLOB::Write( asQWORD data )
{
	return WriteData( data );
}

inline bool CASBLOB::Write( float data )
{
	return WriteData( data );
}

inline bool CASBLOB::Write( double data )
{
	return WriteData( data );
}

inline asBYTE CASBLOB::ReadUInt8()
{
	return ReadData<asBYTE>();
}

inline asWORD CASBLOB::ReadUInt16()
{
	return ReadData<asWORD>();
}

inline asDWORD CASBLOB::ReadUInt32()
{
	return ReadData<asDWORD>();
}

inline asQWORD CASBLOB::ReadUInt64()
{
	return ReadData<asQWORD>();
}

inline float CASBLOB::ReadFloat()
{
	return ReadData<float>();
}

inline double CASBLOB::ReadDouble()
{
	return ReadData<double>();
}

inline asBYTE CASBLOB::ReadUInt8( bool& bSuccess )
{
	return ReadData<asBYTE>( bSuccess );
}

inline asWORD CASBLOB::ReadUInt16( bool& bSuccess )
{
	return ReadData<asWORD>( bSuccess );
}

inline asDWORD CASBLOB::ReadUInt32( bool& bSuccess )
{
	return ReadData<asDWORD>( bSuccess );
}

inline asQWORD CASBLOB::ReadUInt64( bool& bSuccess )
{
	return ReadData<asQWORD>( bSuccess );
}

inline float CASBLOB::ReadFloat( bool& bSuccess )
{
	return ReadData<float>( bSuccess );
}

inline double CASBLOB::ReadDouble( bool& bSuccess )
{
	return ReadData<double>( bSuccess );
}

/**
*	Registers the BLOB class.
*	@param scriptEngine Script engine.
*/
void RegisterScriptBLOB( asIScriptEngine& scriptEngine );

#endif //FILESYSTEM_SCRIPTAPI_CASBLOB_H
