#include <gtest/gtest.h> 
////////////////////////////////////////////////////////////////////////////////
#include <eco/String.h>


////////////////////////////////////////////////////////////////////////////////
TEST(BaseType, cstring)
{
    EXPECT_EQ(eco::is_lower('a'), true);
    EXPECT_EQ(eco::is_lower('z'), true);
    EXPECT_EQ(eco::is_lower('A'), false);
    EXPECT_EQ(eco::is_lower('Z'), false);
    EXPECT_EQ(eco::is_upper('A'), true);
    EXPECT_EQ(eco::is_upper('Z'), true);
    EXPECT_EQ(eco::is_upper('a'), false);
    EXPECT_EQ(eco::is_upper('z'), false);
    EXPECT_EQ(eco::empty(""), true);
}


////////////////////////////////////////////////////////////////////////////////
TEST(BaseType, String)
{
    /*eco::String s;
    EXPECT_TRUE(s.null());
    EXPECT_TRUE(s.empty());
    const char* tmp = "123456";
    s.append(tmp);
    EXPECT_EQ(s.size(), 6);

    // flow size.
    s << tmp;*/
}