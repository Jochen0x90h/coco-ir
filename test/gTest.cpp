#include <gtest/gtest.h>
#include <coco/rc6.hpp>
#include <coco/nec.hpp>


using namespace coco;


TEST(cocoTest, rc6Decode) {
	const uint8_t data2[] = {0,53,17,9,16,9,8,9,7,26,26,9,8,8,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,17,17,9};
	const uint8_t data3[] = {0,53,17,9,17,9,8,9,8,26,26,9,8,9,8,9,8,9,8,8,8,9,8,9,8,9,8,8,8,9,8,8,8,9,8,9,8,18,8,8};

	rc6::Packet packet;

	EXPECT_TRUE(rc6::decode(data2, packet));
	EXPECT_EQ(packet.data, 2);

	EXPECT_TRUE(rc6::decode(data3, packet));
	EXPECT_EQ(packet.data, 3);
}

TEST(cocoTest, necDecode) {
    const uint8_t data0[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,9,12,32,12,9,12,9,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12,9,12,10,12,32,12,9,12,32,12,9,12,9,12,9,12,32,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12};
	const uint8_t data1[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12,32,12,10,12,32,12,10,12,32,12,10,12,10,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12};
	const uint8_t dataStar[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,32,12,10,12,32,12};
	const uint8_t dataPlus[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,10,12,9,12,32,12,9,12,10,12,10,12,32,12,10,12,32,12,32,12,10,12,32,12,32,12,32,12};
	const uint8_t dataMinus[] = {0,181,88,12,32,12,10,12,10,12,9,12,10,12,9,12,32,12,9,12,9,12,32,12,10,12,32,12,9,12,32,12,32,12,32,12,32,12,32,12,9,12,9,12,32,12,9,12,10,12,10,12,9,12,10,12,32,12,32,12,9,12,32,12,32,12,32,12};

	nec::Packet packet;

	EXPECT_TRUE(nec::decode(data1, packet));
	EXPECT_EQ(packet.command, 168);

	EXPECT_TRUE(nec::decode(data0, packet));
	EXPECT_EQ(packet.command, 40);

	EXPECT_TRUE(nec::decode(dataStar, packet));
	EXPECT_EQ(packet.command, 18);

	EXPECT_TRUE(nec::decode(dataPlus, packet));
	EXPECT_TRUE(nec::decode(dataMinus, packet));
}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	int success = RUN_ALL_TESTS();
	return success;
}
