/**
 * @file
 *
 * @brief Test Buffer.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2018-06-05
 *
 * @copyright Copyright (c) 2018.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#include <nsfx/test.h>
#include <nsfx/network/packet/packet.h>
#include <iostream>


NSFX_TEST_SUITE(Packet)
{
    NSFX_TEST_CASE(Ctor)/*{{{*/
    {
        nsfx::Buffer b0(1000, 700, 300);
        // [700 s zs 300 ze e 300]
        b0.AddAtStart(100);
        // [600 s 100 zs 300 ze e 300]
        b0.AddAtEnd(100);
        // [600 s 100 zs 300 ze 100 e 200]
        nsfx::Packet p0(b0);
        NSFX_TEST_EXPECT_EQ(p0.GetSize(), 500);
        NSFX_TEST_EXPECT(b0.cbegin() == p0.GetBufferBegin());
        NSFX_TEST_EXPECT(b0.cend() == p0.GetBufferEnd());
    }/*}}}*/

    NSFX_TEST_CASE(Add)/*{{{*/
    {
        nsfx::Buffer b0(1000, 700, 300);
        // [700 s zs 300 ze e 300]
        nsfx::Packet p0(b0);
        // Add header.
        nsfx::Buffer h = p0.AddHeader(100);
        auto it = h.begin();
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xfe + i;
            it.Write<uint8_t>(v);
        }
        // Add trailer.
        nsfx::Buffer t = p0.AddTrailer(100);
        it = t.begin();
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xef + i;
            it.Write<uint8_t>(v);
        }
        // Test
        auto it1 = p0.GetBufferBegin();
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xfe + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
        it1 += 300;
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xef + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
    }/*}}}*/

    NSFX_TEST_CASE(Remove)/*{{{*/
    {
        nsfx::Buffer b0(1000, 700, 300);
        // [700 s zs 300 ze e 300]
        nsfx::Packet p0(b0);
        // Add header.
        nsfx::Buffer h = p0.AddHeader(100);
        auto it = h.begin();
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xfe + i;
            it.Write<uint8_t>(v);
        }
        // Add trailer.
        nsfx::Buffer t = p0.AddTrailer(100);
        it = t.begin();
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xef + i;
            it.Write<uint8_t>(v);
        }
        // Remove header.
        p0.RemoveHeader(50);
        // Remove trailer.
        p0.RemoveTrailer(50);
        // Test
        auto it1 = p0.GetBufferBegin();
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xfe + 50 + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
        for (size_t i = 0; i < 300; ++i)
        {
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), 0);
        }
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xef + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
    }/*}}}*/

    NSFX_TEST_CASE(ReassembleFragments)/*{{{*/
    {
        nsfx::Buffer b0(1000, 700, 300);
        // [700 s zs 300 ze e 300]
        nsfx::Packet p0(b0);
        // Add header.
        nsfx::Buffer h = p0.AddHeader(100);
        auto it = h.begin();
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xfe + i;
            it.Write<uint8_t>(v);
        }
        // Add trailer.
        nsfx::Buffer t = p0.AddTrailer(100);
        it = t.begin();
        for (size_t i = 0; i < 100; ++i)
        {
            uint8_t v = 0xef + i;
            it.Write<uint8_t>(v);
        }
        // Fragmentation.
        nsfx::Packet f0 = p0.MakeFragment(50, 200);
        nsfx::Packet f1 = p0.MakeFragment(250, 200);
        // Test
        auto it1 = f0.GetBufferBegin();
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xfe + 50 + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
        for (size_t i = 0; i < 150; ++i)
        {
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), 0);
        }
        it1 = f1.GetBufferBegin();
        for (size_t i = 0; i < 150; ++i)
        {
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), 0);
        }
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xef + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
        // Reassembly.
        nsfx::Packet p1(f1);
        p1.AddHeader(f0);
        // Test
        it1 = p1.GetBufferBegin();
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xfe + 50 + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
        for (size_t i = 0; i < 300; ++i)
        {
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), 0);
        }
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xef + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
        // Reassembly.
        nsfx::Packet p2(f0);
        p2.AddTrailer(f1);
        // Test
        it1 = p2.GetBufferBegin();
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xfe + 50 + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
        for (size_t i = 0; i < 300; ++i)
        {
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), 0);
        }
        for (size_t i = 0; i < 50; ++i)
        {
            uint8_t v = 0xef + i;
            NSFX_TEST_EXPECT_EQ(it1.Read<uint8_t>(), v);
        }
    }/*}}}*/

    NSFX_TEST_CASE(Tag)/*{{{*/
    {
        static size_t k = 0;
        struct Test
        {
            Test(int i, double j) : i_(i), j_(j) { ++k; }
            ~Test(void) { --k; }
            int i_;
            double j_;
        };
        {
            nsfx::Buffer b0(1000, 700, 400);
            // [700 s zs 400 ze e 300]
            nsfx::Packet p0(b0);
            size_t tagId = 1;
            // Add 4 tags.
            // |<--------------buffer------------->|
            // 0        100      200      300      400
            // |--------|--------|--------|--------|
            // |<-tag1->|                 |<-tag4->|
            // |<------tag2----->|<------tag3----->|
            p0.AddTag(nsfx::MakeTag<Test>(tagId++, 1, 2.3),   0, 100);
            p0.AddTag(nsfx::MakeTag<Test>(tagId++, 1, 2.3),   0, 200);
            p0.AddTag(nsfx::MakeTag<Test>(tagId++, 1, 2.3), 200, 200);
            p0.AddTag(nsfx::MakeTag<Test>(tagId++, 1, 2.3), 300, 100);
            // Create fragments.
            // | f1 |
            // 0    50
            // |----|---|--------|--------|--------|
            // |<-tag1->|                 |<-tag4->|
            // |<------tag2----->|<------tag3----->|
            nsfx::Packet f1 = p0.MakeFragment(0, 50);
            NSFX_TEST_EXPECT(f1.HasTag(1, 0));
            NSFX_TEST_EXPECT(f1.HasTag(2, 0));
            //      |f2 |
            //      0   50
            // |----|---|--------|--------|--------|
            // |<-tag1->|                 |<-tag4->|
            // |<------tag2----->|<------tag3----->|
            nsfx::Packet f2 = p0.MakeFragment(50, 50);
            NSFX_TEST_EXPECT(f2.HasTag(1, 0));
            NSFX_TEST_EXPECT(f2.HasTag(2, 0));
            //          |     f3      |
            //          0             150
            // |----|---|-------------|---|--------|
            // |<-tag1->|                 |<-tag4->|
            // |<------tag2----->|<------tag3----->|
            nsfx::Packet f3 = p0.MakeFragment(100, 150);
            NSFX_TEST_EXPECT(f3.HasTag(2, 0));
            NSFX_TEST_EXPECT(f3.HasTag(3, 100));
            //                        |       f4   |
            //                        0   50       150
            // |----|---|--------|----|---|--------|
            // |<-tag1->|                 |<-tag4->|
            // |<------tag2----->|<------tag3----->|
            nsfx::Packet f4 = p0.MakeFragment(250, 150);
            NSFX_TEST_EXPECT(f4.HasTag(3, 0));
            NSFX_TEST_EXPECT(f4.HasTag(4, 50));
            // Reassemble the fragments.
            {
                nsfx::Packet r(f4);
                r.AddHeader(f3);
                r.AddHeader(f2);
                r.AddHeader(f1);
                // tag1 survives.
                NSFX_TEST_EXPECT(r.HasTag(1, 0));
                NSFX_TEST_EXPECT(r.HasTag(1, 100-1));
                // tag2 survives.
                NSFX_TEST_EXPECT(r.HasTag(2, 0));
                NSFX_TEST_EXPECT(r.HasTag(2, 200-1));
                // tag3 survives.
                NSFX_TEST_EXPECT(r.HasTag(3, 200));
                NSFX_TEST_EXPECT(r.HasTag(3, 400-1));
                // tag4 survives.
                NSFX_TEST_EXPECT(r.HasTag(4, 300));
                NSFX_TEST_EXPECT(r.HasTag(4, 400-1));
                nsfx::Tag tag4 = r.GetTag(4, 300);
                NSFX_TEST_EXPECT_EQ(tag4.GetId(), 4);
                NSFX_TEST_EXPECT(tag4.GetTypeId() ==
                                 boost::typeindex::type_id<Test>());
            }
            // Reassemble the fragments.
            {
                nsfx::Packet r(f1);
                r.AddTrailer(f2);
                r.AddTrailer(f3);
                r.AddTrailer(f4);
                // tag1 survives.
                NSFX_TEST_EXPECT(r.HasTag(1, 0));
                NSFX_TEST_EXPECT(r.HasTag(1, 100-1));
                // tag2 survives.
                NSFX_TEST_EXPECT(r.HasTag(2, 0));
                NSFX_TEST_EXPECT(r.HasTag(2, 200-1));
                // tag3 survives.
                NSFX_TEST_EXPECT(r.HasTag(3, 200));
                NSFX_TEST_EXPECT(r.HasTag(3, 400-1));
                // tag4 survives.
                NSFX_TEST_EXPECT(r.HasTag(4, 300));
                NSFX_TEST_EXPECT(r.HasTag(4, 400-1));
                nsfx::Tag tag4 = r.GetTag(4, 400-1);
                NSFX_TEST_EXPECT_EQ(tag4.GetId(), 4);
                NSFX_TEST_EXPECT(tag4.GetTypeId() ==
                                 boost::typeindex::type_id<Test>());
            }
        }
        NSFX_TEST_EXPECT_EQ(k, 0);
    }/*}}}*/
}


int main(void)
{
    nsfx::test::runner::GetLogger()->AddStreamSink(std::cout);
    nsfx::test::runner::Run();

    return 0;
}
