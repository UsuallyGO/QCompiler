#include "gtest/gtest.h"

int mul(int a, int b)
{
	return a * b;
}

TEST(ThirdTestGroup, mulPositive)
{
	EXPECT_EQ(mul(2, 1), 2);
}

int main(int argc, char* argv[]){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

