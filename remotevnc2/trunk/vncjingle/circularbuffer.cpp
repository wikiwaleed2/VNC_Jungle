#include "circularbuffer.h"

namespace vncjingle {

CircularBuffer::CircularBuffer(int size)
{
	capacity_ = size;
	store_ = new char[capacity_];
	read_pos_ = 0;
	write_pos_ = 0;
	cached_length_ = 0;
	done_ = false;
}
//-----------------------------------------------------------------------------

CircularBuffer::~CircularBuffer()
{
	delete store_;
	store_ = NULL;
}
//-----------------------------------------------------------------------------

int CircularBuffer::Length()
{
	//talk_base::CritScope lock(&lock_);

	if (write_pos_ >= read_pos_) {
		return write_pos_ - read_pos_;
	} else {
		return (capacity_ - read_pos_) + write_pos_;
	}
}
//-----------------------------------------------------------------------------

int CircularBuffer::AvailableCapacity()
{
	return capacity_ - Length();
}
//-----------------------------------------------------------------------------

bool CircularBuffer::Read(const char *buffer, int length, bool blocking, bool peek)
{
	/*
	 * Wait until enough data has arrived.
	 */
	while (cached_length_ < length && (cached_length_ = Length()) < length) {
		if (!blocking || done_) {
			return false;
		}
		::Sleep(10);
	}
	cached_length_ -= length;

	/*
	 * Read data.
	 */
	int read_end_ = read_pos_;
	if (capacity_ - read_pos_ >= length) {
		// Enough data without wrapping.
		memcpy((void*)buffer, store_ + read_pos_, length);
		read_end_ = read_pos_ + length;
	} else {
		// Handle wrap around.
		int available = capacity_ - read_pos_;
		memcpy((void*)buffer, store_ + read_pos_, available);
		length -= available;
		buffer += available;
		memcpy((void*)buffer, store_, length);
		read_end_ = length;
	}

	/*
	 * Update the read pointer.
	 */
	if (!peek) {
		//talk_base::CritScope lock(&lock_);
		read_pos_ = read_end_;
		if (read_pos_ == capacity_) {
			read_pos_ = 0;
		}
	}

	return true;
}
//-----------------------------------------------------------------------------

void CircularBuffer::ReadSkip(int length) 
{
	talk_base::CritScope lock(&lock_);

	if (read_pos_ + length > capacity_) {
		length -= capacity_ - read_pos_;
		read_pos_ = length;
	} else {
		read_pos_ += length;
	}
	if (read_pos_ == capacity_) {
		read_pos_ = 0;
	}
}
//-----------------------------------------------------------------------------

void CircularBuffer::Write(const char *buffer, int length)
{
	/*
	 * Wait until there is space for new data.
	 */
	while (AvailableCapacity() < length) {
		if (done_) {
			return;
		}
		::Sleep(10);
	}

	/*
	 * Write the data.
	 */
	if (capacity_ - write_pos_ >= length) {
		// Enough data without wrapping.
		memcpy(store_ + write_pos_, buffer, length);
		write_pos_ = write_pos_ + length;
		if (write_pos_ == capacity_) {
			write_pos_ = 0;
		}
	} else {
		// Handle wrap around.
		int available = capacity_ - write_pos_;
		memcpy(store_ + write_pos_, buffer, available);
		length -= available;
		buffer += available;
		memcpy(store_, buffer, length);
		write_pos_ = length;
		if (write_pos_ == capacity_) {
			write_pos_ = 0;
		}
	}
}
//-----------------------------------------------------------------------------

} // Namespace vncjingle
