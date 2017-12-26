/**
 * @file
 *
 * @brief Component support for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2017-12-26
 *
 * @copyright Copyright (c) 2017.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#ifndef CLASS_FACTORY_H__9BDAEBA3_E910_4BA6_8216_41E659E75C96
#define CLASS_FACTORY_H__9BDAEBA3_E910_4BA6_8216_41E659E75C96


#include <nsfx/component/config.h>
#include <nsfx/component/i-class-factory.h>
#include <nsfx/component/object.h>
#include <nsfx/component/exception.h>


NSFX_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
// ClassFactory
/**
 * @ingroup Component
 * @brief An object factory.
 *
 * @tparam Envelopable A class that conforms to \c EnvelopableConcept.
 *
 * Objects created by a factory are enveloped in \c Object or \c AggObject
 * according to whether a controller is specified.<br/>
 */
template<class Envelopable>
class ClassFactory NSFX_FINAL:/*{{{*/
    public IFactory
{
private:
    BOOST_CONCEPT_ASSERT((EnvelopableConcept<Envelopable>));

public:
    virtual ~ClassFactory(void) BOOST_NOEXCEPT {}

public:
    // IFactory /*{{{*/
    virtual void* CreateObject(const uuid& iid, IObject* controller) NSFX_FINAL NSFX_OVERRIDE
    {
        if (controller && iid != uuid_of<IObject>())
        {
            BOOST_THROW_EXCEPTION(BadAggregation());
        }
        return controller ? CreateAggregable(controller)
                          : CreateNonAggregable(iid);
    }

    virtual const uuid& GetCid(void) NSFX_FINAL NSFX_OVERRIDE
    {
        return uuid_of<Envelopable>();
    }

    void* CreateNonAggregable(const uuid& iid)
    {
        typedef Object<Envelopable>  ObjectType;
        ObjectType* o = new ObjectType;
        if (!o)
        {
            BOOST_THROW_EXCEPTION(OutOfMemory());
        }
        void* result = o->QueryInterface(iid);
        if (!result)
        {
            delete o;
            BOOST_THROW_EXCEPTION(NoInterface());
        }
        return result;
    }

    void* CreateAggregable(IObject* controller)
    {
        typedef AggObject<Envelopable>  ObjectType;
        ObjectType* o = new ObjectType(controller);
        if (!o)
        {
            BOOST_THROW_EXCEPTION(OutOfMemory());
        }
        return o->QueryInterface(uuid_of<IObject>());
    }

    /*}}}*/

    NSFX_INTERFACE_MAP_BEGIN(ClassFactory)
        NSFX_INTERFACE_ENTRY(IClassFactory)
    NSFX_INTERFACE_MAP_END()

}; // class ClassFactory /*}}}*/


NSFX_CLOSE_NAMESPACE


#endif // CLASS_FACTORY_H__9BDAEBA3_E910_4BA6_8216_41E659E75C96

