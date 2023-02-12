#include <gtest/gtest.h> 
////////////////////////////////////////////////////////////////////////////////
#include <eco/Rtype.h>
#include <eco/detail/proto/Eco.pb.h>


////////////////////////////////////////////////////////////////////////////////
TEST(BaseType, Rtype)
{
    EXPECT_EQ(eco::TypeId<double>(), eco::TypeId<double>::value);
    EXPECT_NE(eco::TypeId<int>(), eco::TypeId<std::string>::value);
    EXPECT_NE(eco::TypeId<int>(), eco::TypeId<eco::Error>::value);
    const char* dd = eco::GetTypeId::name<double>();
    const char* ee = eco::GetTypeId::name<eco::proto::Error>();
    EXPECT_NE(dd, ee);
    uint32_t did = eco::GetTypeId::get_id(dd);
    uint32_t cid = eco::GetTypeId::get_id(cc);
    EXPECT_NE(did, cid);
}