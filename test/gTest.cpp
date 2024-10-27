#include <gtest/gtest.h>
#include <coco/nec.hpp>
#include <coco/nubert.hpp>
#include <coco/rc6.hpp>


using namespace coco;


TEST(cocoTest, necDecode) {
    const uint8_t times0[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,9,12,32,12,9,12,9,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12,9,12,10,12,32,12,9,12,32,12,9,12,9,12,9,12,32,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12};
    const uint8_t times1[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12,32,12,10,12,32,12,10,12,32,12,10,12,10,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12};
    const uint8_t timesStar[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,32,12,10,12,32,12};
    const uint8_t timesPlus[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,10,12,9,12,32,12,9,12,10,12,10,12,32,12,10,12,32,12,32,12,10,12,32,12,32,12,32,12};
    const uint8_t timesMinus[] = {0,181,88,12,32,12,10,12,10,12,9,12,10,12,9,12,32,12,9,12,9,12,32,12,10,12,32,12,9,12,32,12,32,12,32,12,32,12,32,12,9,12,9,12,32,12,9,12,10,12,10,12,9,12,10,12,32,12,32,12,9,12,32,12,32,12,32,12};

    nec::Packet packet;

    EXPECT_TRUE(nec::decode(times0, packet));
    EXPECT_EQ(packet.command, 40);

    EXPECT_TRUE(nec::decode(times1, packet));
    EXPECT_EQ(packet.command, 168);

    EXPECT_TRUE(nec::decode(timesStar, packet));
    EXPECT_EQ(packet.command, 18);

    EXPECT_TRUE(nec::decode(timesPlus, packet));
    EXPECT_TRUE(nec::decode(timesMinus, packet));
}

TEST(cocoTest, nubertDecode) {
    const uint8_t timesUp[] = {0,25,8,25,8,25,8,25,8,8,24,8,25,8,25,8,25,8,25,8,25,8,25,25};
    const uint8_t timesMiddle[] = {0,25,8,25,7,25,8,25,8,8,25,9,24,9,24,8,25,8,25,8,25,25,8,8};
    const uint8_t timesDown[] = {0,25,8,24,8,25,7,25,7,8,25,9,24,8,25,8,25,8,25,25,8,8,25,8};

    uint16_t packet;

    EXPECT_TRUE(nubert::decode(timesUp, packet));
    EXPECT_EQ(packet, 28800);

    EXPECT_TRUE(nubert::decode(timesMiddle, packet));
    EXPECT_EQ(packet, 28801);

    EXPECT_TRUE(nubert::decode(timesDown, packet));
    EXPECT_EQ(packet, 28802);
}

TEST(cocoTest, rc6Decode) {
    const uint8_t times2[] = {0,53,17,9,16,9,8,9,7,26,26,9,8,8,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,17,17,9};
    const uint8_t times3[] = {0,53,17,9,17,9,8,9,8,26,26,9,8,9,8,9,8,9,8,8,8,9,8,9,8,9,8,8,8,9,8,8,8,9,8,9,8,18,8,8};

    rc6::Packet packet;

    EXPECT_TRUE(rc6::decode(times2, packet));
    EXPECT_EQ(packet.data, 2);

    EXPECT_TRUE(rc6::decode(times3, packet));
    EXPECT_EQ(packet.data, 3);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    int success = RUN_ALL_TESTS();
    return success;
}
