#ifndef GLICKO_CONFIG_HPP
#define GLICKO_CONFIG_HPP

namespace Glicko
{
    /// The default/initial rating value
    static const double kDefaultR = 1500.0;
    
    /// The default/initial deviation value
    static const double kDefaultRD = 350.0;

    /// The default/initial volatility value
    static const double kDefaultS = 0.06;


    /// The Glicko-1 to Glicko-2 scale factor
    static const double kScale = 173.7178;

    /// The system constant (tau)
    static const double kSystemConst = 0.5;

    /// The convergence constant (epsilon)
    static const double kConvergence = 0.000001;
}

#endif
