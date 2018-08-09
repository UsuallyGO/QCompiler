
#include "idstatebuilder.h"
#include "idstatebuilder_factory.h"
#include "gtest/gtest.h"

class QTestIDStateBuilder : public IDStateBuilder{
public:
	void BuildIDState(const std::string& str){ std::cout<<"In QTestIDStateBuilder"<<std::endl;}
};

class QTestIDStateBuilderFactory final : public IDStateBuilderFactory {
public:
	IDStateBuilder* CreateIDStateBuilder() {
		IDStateBuilder* builder = new QTestIDStateBuilder;//builder should be checked here...
		return builder;
	}
};

IDSTATEBUILDER_FACTORY_REGISTER("QTestIDStateBuilderFactory", QTestIDStateBuilderFactory);

TEST(FirstTestGroup, addPositive)
{
	IDStateBuilderFactory *factory = IDStateBuilderFactory::GetFactory("QIDStateBuilderFactory"); 
	IDStateBuilder* builder = factory->CreateIDStateBuilder();
}

int main(int argc, char* argv[]){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}