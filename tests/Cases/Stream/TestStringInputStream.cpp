#include <array>
#include <string_view>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(StringInputStream, all) {
	return seastar::do_with(
		cpv::StringInputStream(),
		[] (auto& stream) {
		stream.reset("test data");
		return stream.read().then([&stream] (auto&& result) {
			ASSERT_TRUE(stream.sizeHint().has_value());
			ASSERT_EQ(*stream.sizeHint(), 9U);
			ASSERT_EQ(result.data, "test data");
			ASSERT_TRUE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.data, "");
			ASSERT_TRUE(result.isEnd);
		});
	});
}

