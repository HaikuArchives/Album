/**
Copyright (c) 2008 by Matjaz Kovac

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

\file RingBuf.h
\author Matjaž Kovač
*/

#ifndef _RINGBUF_H_
#define _RINGBUF_H_

/**
	A quick and dirty circular buffer.
	Meant for small buffers (64K elements max).
*/
template <class T>
class RingBuffer {
	public:

	RingBuffer(short maxsize):
		capacity(maxsize) 
	{
		size = head = tail = 0;
		data = new T[capacity];
	}
	
	~RingBuffer() {
		delete[] data;
	}

	inline bool Full() {
		return head == tail && size == capacity;	
	}
	
	inline short Size() {
		return size;
	}
	
	/**
		Stores a new pointer if not full.		
	*/
	bool Put(T item) {
		if (size >= capacity)
			return false;
		if (++head >= capacity)
			head = 0;
		data[head] = item;
		size++;
		return true;
	}

	/**
		Gets the oldest pointer if not empty.
	*/
	T Get() {
		if (size <= 0)
			return NULL;
		if (++tail >= capacity)
			tail = 0;
		size--;
		return data[tail];
	}
	
	private:
	
	T *data;
	short capacity;
	short size;
	short head, tail;		
};

#endif	// _RINGBUF_H_
