#pragma once
#include <optional>
#include <seastar/core/future.hh>
#include "../Utility/SharedString.hpp"

namespace cpv {
	/** Contains data read from stream */
	struct InputStreamReadResult {
		/** Data read from stream */
		SharedString data;
		/** Whether data is the last part of stream */
		bool isEnd;
		
		/** Constructor */
		InputStreamReadResult(SharedString&& dataVal, bool isEndVal) :
			data(std::move(dataVal)), isEnd(isEndVal) { }
		
		/** Constructor */
		InputStreamReadResult() : InputStreamReadResult({}, true) { }
	};
	
	/**
	 * Interface of simple input stream.
	 * The read function will return a mutable buffer contains data.
	 * The mutable buffer is useful for pass to an inplace (in-situ) parser.
	 * Seek is not supported so it's easy to implement and with less overhead.
	 */
	class InputStreamBase {
	public:
		/** Virtual destructor */
		virtual ~InputStreamBase() = default;
		
		/** Read data from stream */
		virtual seastar::future<InputStreamReadResult> read() = 0;
		
		/**
		 * Get the hint of total size of stream, may return empty if not supported
		 * Warning:
		 * The actual read size may greater than this size because some
		 * implementation allows client side to control it (e.g. Content-Length),
		 * you can use this size to pre allocate buffer, but you must check
		 * the actual size after each read to avoid buffer overflow.
		 */
		virtual std::optional<std::size_t> sizeHint() const { return { }; }
	};
}

