#pragma once
#include "arSocket.h"

inline void		SocketBuffer::DataLength(Length_t dataLength) { (*(Length_t*)m_buffer) = LengthToN(dataLength); }
inline char*	SocketBuffer::Buffer() { return m_buffer + lengthSize; }
inline char*	SocketBuffer::At(int index) { return m_buffer + lengthSize + index; }
inline char&	SocketBuffer::operator[](int index) { return *At(index); }

inline SocketBuffer::Length_t	SocketBuffer::DataLength() const { return LengthToH(*(Length_t*)m_buffer); }
inline const char*				SocketBuffer::Buffer() const { return m_buffer + lengthSize; }
inline const char*				SocketBuffer::At(int index) const { return m_buffer + lengthSize + index; }
inline const char&				SocketBuffer::operator[](int index) const { return *At(index); }
