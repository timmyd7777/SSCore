#include "SSVector.hpp"

class SSMatrix
{
    public:
    
    double m00, m01, m02;
    double m10, m11, m12;
    double m20, m21, m22;
    
    SSMatrix ( void );
    SSMatrix ( double m00, double m01, double m02, double m10, double m11, double m12, double m210, double m21, double m22 );
    
    SSMatrix transpose ( void );

    static SSMatrix identity ( void );
    static SSMatrix rotation ( int axis, double angle );
    
    SSVector multiply ( SSVector vec );
    SSMatrix multiply ( SSMatrix mat );
};
