/**
 * @file
 *
 * @brief Random number support for Network Simulation Frameworks.
 *
 * @version 1.0
 * @author  Wei Tang <gauchyler@uestc.edu.cn>
 * @date    2018-09-23
 *
 * @copyright Copyright (c) 2018.
 *   National Key Laboratory of Science and Technology on Communications,
 *   University of Electronic Science and Technology of China.
 *   All rights reserved.
 */

#ifndef I_RANDOM_DISTRIBUTION_GENERATOR_H__9C3F214F_B1A5_4877_865D_105326654B5B
#define I_RANDOM_DISTRIBUTION_GENERATOR_H__9C3F214F_B1A5_4877_865D_105326654B5B


#include <nsfx/random/config.h>
#include <nsfx/component/i-object.h>
#include <nsfx/component/i-user.h>
#include <nsfx/component/ptr.h>

#include <nsfx/random/distribution/i-uniform-int-distribution.h>
#include <nsfx/random/distribution/i-uniform-real-distribution.h>

#include <nsfx/random/distribution/i-bernoulli-distribution.h>
#include <nsfx/random/distribution/i-binomial-distribution.h>
#include <nsfx/random/distribution/i-negative-binomial-distribution.h>
#include <nsfx/random/distribution/i-geometric-distribution.h>

#include <nsfx/random/distribution/i-poisson-distribution.h>
#include <nsfx/random/distribution/i-exponential-distribution.h>
#include <nsfx/random/distribution/i-gamma-distribution.h>
#include <nsfx/random/distribution/i-weibull-distribution.h>
#include <nsfx/random/distribution/i-extreme-value-distribution.h>

#include <nsfx/random/distribution/i-normal-distribution.h>
#include <nsfx/random/distribution/i-lognormal-distribution.h>
#include <nsfx/random/distribution/i-chi-squared-distribution.h>
#include <nsfx/random/distribution/i-cauchy-distribution.h>
#include <nsfx/random/distribution/i-fisher-f-distribution.h>
#include <nsfx/random/distribution/i-student-t-distribution.h>

#include <nsfx/random/distribution/i-discrete-distribution.h>
#include <nsfx/random/distribution/i-piecewise-constant-distribution.h>
#include <nsfx/random/distribution/i-piecewise-linear-distribution.h>

#include <nsfx/random/distribution/i-triangle-distribution.h>


NSFX_OPEN_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
/**
 * @ingroup Random
 * @brief A statistical distribution generator.
 *
 * # Supported statistical distributions
 * ## Uniform distributions
 * * \c IUniformIntDistribution
 * * \c IUniformRealDistribution
 * ## Bernoulli distributions
 * * \c IBernoulliDistribution
 * * \c IBinomialDistribution
 * * \c INegativeBinomialDistribution
 * * \c IGeometricDistribution
 * ## Poisson distributions
 * * \c IPoissonDistribution
 * * \c IExponentialDistribution
 * * \c IGammaDistribution
 * * \c IWeibullDistribution
 * * \c IExtremeValueDistribution
 * ## Normal distributions
 * * \c INormalDistribution
 * * \c ILognormalDistribution
 * * \c IChiSquaredDistribution
 * * \c ICauchyDistribution
 * * \c IFisherFDistribution
 * * \c IStudentTDistribution
 * ## Sampling distributions
 * * \c IDiscreteDistribution
 * * \c IPiecewiseConstantDistribution
 * * \c IPiecewiseLinearDistribution
 * ## Miscellaneous Distributions
 * * \c ITriangleDistribution
 */
class IRandomDistributionGenerator :
    virtual public IObject
{
public:
    virtual ~IRandomDistributionGenerator(void) BOOST_NOEXCEPT {}

    /**
     * @brief Create a discrete uniform distribution.
     *
     * @param[in] lb The lower bound of the range.
     * @param[in] ub The upper bound of the range.
     *            It <b>must</b> be greater than or equal to \c lb.
     *
     * The possible values the distribution can generate is within
     * <i>[lb, ub]</i>.
     */
    virtual Ptr<IUniformIntDistribution>
            CreateUniformIntDistribution(int32_t lb, int32_t ub) = 0;

    /**
     * @brief Create a continous uniform distribution.
     *
     * @param[in] lb The lower bound of the range.
     * @param[in] ub The upper bound of the range.
     *            It <b>must</b> be greater than or equal to \c lb.
     *
     * The possible values the distribution can generate is within
     * <i>[lb, ub)</i>.
     */
    virtual Ptr<IUniformRealDistribution>
            CreateUniformRealDistribution(double lb, double ub) = 0;

    /**
     * @brief Create a Bernoulli distribution.
     *
     * @param[in] prob Probability of producing a value of \c true.
     *                 It <b>must</b> be within <i>[0, 1]</i> .
     */
    virtual Ptr<IBernoulliDistribution>
            CreateBernoulliDistribution(double prob) = 0;

    /**
     * @brief Create a binomial distribution.
     *
     * @param[in] numTrials The number of independent Bernoulli-distributed
     *                      experiments.
     * @param[in] prob Probability of success of the independent
     *                 Bernoulli-distributed experiments.
     *                 It <b>must</b> be within <i>[0, 1]</i> .
     */
    virtual Ptr<IBinomialDistribution>
            CreateBinomialDistribution(uint32_t numTrials, double prob) = 0;

    /**
     * @brief Create a negative binomial distribution.
     *
     * @param[in] numTrials The number of successful trials that stops
     *                      the count of unsuccessful Bernoulli-distributed
     *                      experiments.
     * @param[in] prob Probability of success of the independent
     *                 Bernoulli-distributed experiments.
     *                 It <b>must</b> be within <i>[0, 1]</i> .
     */
    virtual Ptr<INegativeBinomialDistribution>
            CreateNegativeBinomialDistribution(
                uint32_t numTrials, double prob) = 0;

    /**
     * @brief Create a geometric distribution.
     *
     * @param[in] prob Probability of success.
     *                 It <b>must</b> be within <i>[0, 1]</i> .
     */
    virtual Ptr<IGeometricDistribution>
            CreateGeometricDistribution(double prob) = 0;

    /**
     * @brief Create a Poisson distribution.
     *
     * @param[in] mean The expected number of events in interval.
     *                 It <b>must</b> be positive.
     */
    virtual Ptr<IPoissonDistribution>
            CreatePoissonDistribution(double mean) = 0;

    /**
     * @brief Create an exponential distribution.
     *
     * @param[in] lambda The average rate of occurrence.
     *                   It <b>must</b> be positive.
     */
    virtual Ptr<IExponentialDistribution>
            CreateExponentialDistribution(double lambda) = 0;

    /**
     * @brief Create a gamma distribution.
     *
     * @param[in] shape The shape of the distribution.
     *                  It <b>must</b> be positive.
     * @param[in] scale The scale of the distribution.
     *                  It <b>must</b> be positive.
     */
    virtual Ptr<IGammaDistribution>
            CreateGammaDistribution(double shape, double scale) = 0;

    /**
     * @brief Create a gamma distribution.
     *
     * @param[in] shape The shape of the distribution.
     *                  It <b>must</b> be positive.
     * @param[in] scale The scale of the distribution.
     *                  It <b>must</b> be positive.
     */
    virtual Ptr<IWeibullDistribution>
            CreateWeibullDistribution(double shape, double scale) = 0;

    /**
     * @brief Create an extreme value distribution.
     *
     * @param[in] location The location (shift) of the distribution.
     * @param[in] scale    The scale of the distribution.
     *                     It <b>must</b> be positive.
     */
    virtual Ptr<IExtremeValueDistribution>
            CreateExtremeValueDistribution(double location, double scale) = 0;

    /**
     * @brief Create a normal distribution.
     *
     * @param[in] mean   The mean of the distribution (its expected value).
     * @param[in] stddev The standard deviation of the distribution.
     *                   It <b>must</b> be positive.
     */
    virtual Ptr<INormalDistribution>
            CreateNormalDistribution(double mean, double stddev) = 0;

    /**
     * @brief Create a lognormal distribution.
     *
     * @param[in] mean   The mean of the underlying normal distribution.
     * @param[in] stddev The standard deviation of the underlying normal
     *                   distribution.
     *                   It <b>must</b> be positive.
     */
    virtual Ptr<ILognormalDistribution>
            CreateLognormalDistribution(double mean, double stddev) = 0;

    /**
     * @brief Create a chi-squared distribution.
     *
     * @param[in] degreesOfFreedom The degrees of freedom.
     *                             It <b>must</b> be positive.
     */
    virtual Ptr<IChiSquaredDistribution>
            CreateChiSquaredDistribution(double degreesOfFreedom) = 0;

    /**
     * @brief Create a Cauchy distribution.
     *
     * @param[in] location The location of the peak (its mode).
     * @param[in] scale    The scale of the distribution.
     *                     It <b>must</b> be positive.
     */
    virtual Ptr<ICauchyDistribution>
            CreateCauchyDistribution(double location, double scale) = 0;

    /**
     * @brief Create a Fisher F-distribution.
     *
     * @param[in] numerator   The numerator's degrees of freedom.
     *                        It <b>must</b> be positive.
     * @param[in] denominator The denominator's degrees of freedom.
     *                        It <b>must</b> be positive.
     */
    virtual Ptr<IFisherFDistribution>
            CreateFisherFDistribution(double numerator, double denominator) = 0;

    /**
     * @brief Create a Student t-distribution.
     *
     * @param[in] degreesOfFreedom The degrees of freedom.
     *                             It <b>must</b> be positive.
     */
    virtual Ptr<IStudentTDistribution>
            CreateStudentTDistribution(double degreesOfFreedom) = 0;

    /**
     * @brief Create a discrete distribution.
     */
    virtual Ptr<IDiscreteDistribution>
            CreateDiscreteDistribution(
                Ptr<IDiscreteDistributionParam> param) = 0;

    /**
     * @brief Create a piecewise constant distribution.
     */
    virtual Ptr<IPiecewiseConstantDistribution>
            CreatePiecewiseConstantDistribution(
                Ptr<IPiecewiseConstantDistributionParam> param) = 0;

    /**
     * @brief Create a piecewise linear distribution.
     */
    virtual Ptr<IPiecewiseLinearDistribution>
            CreatePiecewiseLinearDistribution(
                Ptr<IPiecewiseLinearDistributionParam> param) = 0;

    /**
     * @brief Create a triangle distribution.
     *
     * @param[in] a The smallest value.
     * @param[in] b The most probable value.
     * @param[in] c The largest value.
     */
    virtual Ptr<ITriangleDistribution>
            CreateTriangleDistribution(double a, double b, double c) = 0;

};

NSFX_DEFINE_CLASS_UID(IRandomDistributionGenerator,
                      "edu.uestc.nsfx.IRandomDistributionGenerator");


////////////////////////////////////////
NSFX_DEFINE_USER_INTERFACE(
    IRandomDistributionGeneratorUser, "edu.uestc.nsfx.IRandomDistributionGeneratorUser",
    IRandomDistributionGenerator);


NSFX_CLOSE_NAMESPACE


#endif // I_RANDOM_DISTRIBUTION_GENERATOR_H__9C3F214F_B1A5_4877_865D_105326654B5B

