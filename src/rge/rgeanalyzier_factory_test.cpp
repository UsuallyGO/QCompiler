
#include "gtest/gtest.h"

int sub(int a, int b)
{
	return a - b;
}

TEST(SecondTestGroup, subPositive)
{
	EXPECT_EQ(sub(2, 1), 1);
}

int main(int argc, char* argv[]){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
