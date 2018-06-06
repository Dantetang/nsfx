/**
 * @file
 *
 * @brief Test Tag.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2018-06-02
 *
 * @copyright Copyright (c) 2018.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#include <nsfx/test.h>
#include <nsfx/network/packet/tag-storage.h>
#include <iostream>


int k = 0;
struct Test
{
    Test(int i, double j) : i_(i), j_(j) { ++k; }
    ~Test(void) { --k; }
    int i_;
    double j_;
};


NSFX_TEST_SUITE(TagStorage)
{
    NSFX_TEST_CASE(Allocate)
    {
        nsfx::TagStorage* ts = nsfx::TagStorage::Allocate<Test>(1, 2.3);
        // Test::Test() is called.
        NSFX_TEST_EXPECT_EQ(k, 1);
        NSFX_TEST_EXPECT_EQ(ts->refCount_, 1);
        NSFX_TEST_EXPECT(ts->intf_->GetTypeId() ==
                         boost::typeindex::type_id<Test>());
        NSFX_TEST_EXPECT(nsfx::TagStorage::GetTypeId(ts) ==
                         boost::typeindex::type_id<Test>());
        const Test& t = nsfx::TagStorage::GetValue<Test>(ts);
        NSFX_TEST_EXPECT_EQ(t.i_, 1);
        NSFX_TEST_EXPECT_EQ(t.j_, 2.3);
        nsfx::TagStorage::Release(ts);
        // Test::~Test() is called.
        NSFX_TEST_EXPECT_EQ(k, 0);
    }

    NSFX_TEST_CASE(RefCount)
    {
        nsfx::TagStorage* ts = nsfx::TagStorage::Allocate<Test>(1, 2.3);
        NSFX_TEST_EXPECT_EQ(ts->refCount_, 1);
        nsfx::TagStorage::AddRef(ts);
        NSFX_TEST_EXPECT_EQ(ts->refCount_, 2);
        nsfx::TagStorage::Release(ts);
        NSFX_TEST_EXPECT_EQ(ts->refCount_, 1);
        nsfx::TagStorage::Release(ts);
        // Test::~Test() is called.
        NSFX_TEST_EXPECT_EQ(k, 0);
    }

}


int main(void)
{
    nsfx::test::runner::GetLogger()->AddStreamSink(std::cout);
    nsfx::test::runner::Run();

    return 0;
}
