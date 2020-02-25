class SSAngle
{
public:

    double a;       // always radians
    
    static constexpr double kPi  = 3.14159265359;
    static constexpr double kTwoPi = 6.28318530718;
    static constexpr double kHalfPi = 1.57079632679;
    
    static constexpr double kDegPerRad = 57.2957795131;
    static constexpr double kRadPerDeg = 0.01745329251;
    
    SSAngle ( void );
    SSAngle ( double rad );
    
    double toDegrees ( void ) { return a * kDegPerRad; }
    void toDegMin ( short &deg, double &min, char &sign );
    void toDegMinSec ( short &deg, short &min, double &sec, char &sign );
    
    void fromDegrees ( double deg ) { a = deg * kRadPerDeg; }
    void fromDegMin ( short deg, short min, char sign );
    void fromDegMinSec ( short deg, short min, double sec, char sign );
    
    SSAngle modPi ( void );
    SSAngle mod2Pi ( void );
    
    SSAngle angularSeparation ( SSAngle lon0, SSAngle lat0, SSAngle lon1, SSAngle lat1 );
    SSAngle positionAngle ( SSAngle lon0, SSAngle lat0, SSAngle lon1, SSAngle lat1 );
};
