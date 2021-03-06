/**
 * @file
 *
 * @brief Test ClassRegistry.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2017-12-28
 *
 * @copyright Copyright (c) 2017.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#include <nsfx/test.h>
#include <nsfx/component/class-registry.h>
#include <nsfx/component/ptr.h>
#include <iostream>


NSFX_TEST_SUITE(ClassRegistry)
{
    using nsfx::refcount_t;

    struct ITest : virtual nsfx::IObject/*{{{*/
    {
        virtual ~ITest(void) {}

        virtual refcount_t GetRefCount(void) = 0;
    };/*}}}*/

    NSFX_DEFINE_CLASS_UID(ITest, "edu.uestc.nsfx.test.ITest");

    struct Test : public ITest/*{{{*/
    {
        virtual ~Test(void) {}

        virtual refcount_t GetRefCount(void)
        {
            AddRef();
            return Release();
        }

        NSFX_INTERFACE_MAP_BEGIN(Test)
            NSFX_INTERFACE_ENTRY(ITest)
        NSFX_INTERFACE_MAP_END()

    };/*}}}*/

    refcount_t RefCount(nsfx::IObject* p)/*{{{*/
    {
        refcount_t result = 0;
        if (p)
        {
            p->AddRef();
            result = p->Release();
        }
        return result;
    }/*}}}*/

    typedef nsfx::ClassFactory<Test>  TestFactory;
    typedef nsfx::Object<TestFactory> TestFactoryClass;

    NSFX_TEST_CASE(Direct)/*{{{*/
    {
        nsfx::IClassRegistry* registry = nsfx::ClassRegistry::GetInstance();
        NSFX_TEST_ASSERT(registry);

        // Register with empty factory (throw).
        try
        {
            registry->Register(nullptr, "edu.uestc.nsfx.test.Test");
            NSFX_TEST_EXPECT(false);
        }
        catch (nsfx::InvalidPointer& )
        {
            // Should come here.
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Get unregistered factory (throw).
        try
        {
            registry->GetClassFactory("edu.uestc.nsfx.test.Test");
            NSFX_TEST_EXPECT(false);
        }
        catch (nsfx::ClassNotRegistered& )
        {
            // Should come here.
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Register.
        nsfx::Ptr<nsfx::IClassFactory> factory;
        try
        {
            factory = new TestFactoryClass;
            registry->Register(factory, "edu.uestc.nsfx.test.Test");
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Register already registered (throw).
        try
        {
            registry->Register(factory, "edu.uestc.nsfx.test.Test");
            NSFX_TEST_EXPECT(false);
        }
        catch (nsfx::ClassAlreadyRegistered& )
        {
            // Should come here.
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Get registered factory.
        nsfx::Ptr<nsfx::IClassFactory> f;
        try
        {
            f = registry->GetClassFactory("edu.uestc.nsfx.test.Test");
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }
        NSFX_TEST_ASSERT(f);
        NSFX_TEST_EXPECT(factory == f);

        // Use factory to create objects.
        try
        {
            // Create a non-aggregable object via class factory.
            nsfx::Ptr<nsfx::IObject> o = f->CreateObject(nullptr);
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Unregister.
        registry->Unregister("edu.uestc.nsfx.test.Test");
        try
        {
            registry->GetClassFactory("edu.uestc.nsfx.test.Test");
            NSFX_TEST_EXPECT(false);
        }
        catch (nsfx::ClassNotRegistered& )
        {
            // Should come here
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Unregister all.
        try
        {
            registry->Register(factory, "edu.uestc.nsfx.test.Test");
            registry->UnregisterAll();
            registry->GetClassFactory("edu.uestc.nsfx.test.Test");
            NSFX_TEST_EXPECT(false);
        }
        catch (nsfx::ClassNotRegistered& )
        {
            // Should come here
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

    }/*}}}*/

    NSFX_TEST_CASE(ViaFunctions)/*{{{*/
    {
        // Register class with default factory.
        try
        {
            nsfx::RegisterDefaultClassFactory<Test>("edu.uestc.nsfx.test.Test");
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Create object.
        nsfx::Ptr<ITest> t;
        try
        {
            t = nsfx::CreateObject<ITest>("edu.uestc.nsfx.test.Test", nullptr);
            NSFX_TEST_EXPECT(t);
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Unregister class (template-based).
        nsfx::UnregisterClassFactory("edu.uestc.nsfx.test.Test");
        try
        {
            t = nsfx::CreateObject<ITest>("edu.uestc.nsfx.test.Test", nullptr);
            NSFX_TEST_EXPECT(false);
        }
        catch (nsfx::ClassNotRegistered& )
        {
            // Should come here.
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Register a class with a custom factory.
        try
        {
            nsfx::RegisterClassFactory<TestFactory>("edu.uestc.nsfx.test.Test");
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Create object.
        try
        {
            t = nsfx::CreateObject<ITest>("edu.uestc.nsfx.test.Test", nullptr);
            NSFX_TEST_EXPECT(t);
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // Unregister class.
        nsfx::UnregisterClassFactory("edu.uestc.nsfx.test.Test");
        try
        {
            t = nsfx::CreateObject<ITest>("edu.uestc.nsfx.test.Test", nullptr);
            NSFX_TEST_EXPECT(false);
        }
        catch (nsfx::ClassNotRegistered& )
        {
            // Should come here.
        }
        catch (boost::exception& e)
        {
            NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        }

        // // Unregister all classes.
        // try
        // {
        //     nsfx::RegisterDefaultClassFactory<Test>("edu.uestc.nsfx.test.Test");
        //     nsfx::UnregisterAllClassFactories();
        // }
        // catch (boost::exception& e)
        // {
        //     NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        // }
        // try
        // {
        //     t = nsfx::CreateObject<ITest>("edu.uestc.nsfx.test.Test", nullptr);
        //     NSFX_TEST_EXPECT(false);
        // }
        // catch (nsfx::ClassNotRegistered& )
        // {
        //     // Should come here.
        // }
        // catch (boost::exception& e)
        // {
        //     NSFX_TEST_EXPECT(false) << diagnostic_information(e) << std::endl;
        // }

    }/*}}}*/

}


int main(void)
{
    nsfx::test::runner::GetLogger()->AddStreamSink(std::cerr);
    nsfx::test::runner::Run();

    return 0;
}

