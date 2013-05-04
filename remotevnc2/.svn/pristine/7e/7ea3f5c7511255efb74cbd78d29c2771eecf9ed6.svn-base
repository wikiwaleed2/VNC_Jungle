#ifndef VNCJINGLE_CIRCULARBUFFER_H__
#define VNCJINGLE_CIRCULARBUFFER_H__

#include "talk/base/criticalsection.h"

namespace vncjingle {

/*
 * Thread safe circular buffer that allows efficient writing
 * and reading of small amounts of data. Supports blocking and
 * non-blocking access. Assumes that data is written to the
 * buffer in large blocks, but may be read in small increments.
 */
class CircularBuffer {
public:
  CircularBuffer(int size);
  ~CircularBuffer();

	/*
	 * Amount of data available in the buffer.
	 */
  int Length();
	/*
	 * Amount of data that can be written into the buffer without
	 * blocking.
	 */
	int AvailableCapacity();

	/*
	 * Reads the requested length of data from the buffer. If
	 * Blocking is false and insufficient data is available then no
	 * data is read and false is returned. If Peek is true then
	 * the data is read but not removed from the buffer.
	 */
  bool Read(const char *buffer, int length, bool blocking, bool peek);
  void Write(const char *buffer, int length);

	/*
	 * Skip bytes in the read queue.
	 */
	void ReadSkip(int length);

	void Done() { done_ = true; }

private:
	/*
	 * Data is stored in a single pre-allocated buffer.
	 */
	char *store_;
	int capacity_; // Size of store_.
	int read_pos_; // Offset into the store_ to read data from.
	int write_pos_; // Offset into the store_ to write data to.
	bool done_; // Whether the circular buffer is complete (i.e. socket has closed).

	int cached_length_; // Cache of the available length. At least this length
		// is available.

	talk_base::CriticalSection lock_;
};
//-----------------------------------------------------------------------------

} // namespace vncjingle

#endif // VNCJINGLE_CIRCULARBUFFER_H__