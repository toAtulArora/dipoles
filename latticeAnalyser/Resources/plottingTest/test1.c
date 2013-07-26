#include <iostream>
#include "plplot/plplot.h"
#include "plplot/plstream.h"

using namespace std;

// g++ `pkg-config --cflags --libs plplotd-c++` prog.cpp -o prog

class x01cc
{
public:
   x01cc(int, const char**);
   void plot1();

private:
   plstream *pls;
};

void x01cc::plot1()
{
        // Make a simple function, y(x) = x^2
        int N = 100;
        double xmax = 5.0;
        double dx = xmax/(double)N;
        double x[N], y[N];
        double** z;
        pls->Alloc2dGrid(&z,N,N);
        // x[0] = -xmax/2.0; 
        x[0]=0;
        y[0] = x[0]*x[0];
        for(int i=1; i<N; i++){
                x[i] = x[i-1] + dx;
                y[i] = x[i]*x[i];
                for(int j=0;j<N;j++)
                  z[i][j]=i+j;
        }

        // Try to draw the function
        // plstream p = plstream();
        // pls->init();
        // pls->env(x[0],x[N-1],y[0],y[N-1],1, 0);
        // pls->env(x[0],x[N-1],y[0],y[N/2],0,0);
        double xmin2d = -2.5;
        double xmax2d =  2.5;
        double ymin2d = -2.5;
        double ymax2d =  4.0;
        pls->env(xmin2d, xmax2d, ymin2d, ymax2d, 0, -2);
        double basex = 2.0;
        double basey = 4.0;
        double height = 3.0;
        double xmin = -10.0;
        // double xmax = 10.0;
        double ymin = -3.0;
        double ymax = 7.0;
        double zmin = 0.0;
        double zmax = 8.0;
        double alt = 45.0;
        double az = 30.0;
        double side = 1;        
        pls->w3d(basex, basey, height, xmin, xmax, ymin, ymax, zmin, zmax, alt, az);
        pls->plot3d(x,y,(const double**) z,N,N,DRAW_LINEY,true);
        pls->col0(3);
        pls->poin3(N,x,y,x,1);

        // PLFLT *i   = new PLFLT[2];
        // PLFLT *h   = new PLFLT[2];
        // PLFLT *l   = new PLFLT[2];
        // PLFLT *s   = new PLFLT[2];
        // bool  *rev = new bool[2];

        // i[0] = 0.0;       // left boundary
        // i[1] = 1.0;       // right boundary

        // h[0] = 240;       // blue -> green -> yellow ->
        // h[1] = 0;         // -> red

        // l[0] = 0.6;
        // l[1] = 0.6;

        // s[0] = 0.8;
        // s[1] = 0.8;

        // rev[0] = false;       // interpolate on front side of colour wheel.
        // rev[1] = false;       // interpolate on front side of colour wheel.

        // pls->scmap1n( 256 );
        // pls->scmap1l( false, 2, i, h, l, s, rev );

        // delete[] i;
        // delete[] h;
        // delete[] l;
        // delete[] s;
        // delete[] rev;

        
        pls->box3( "bnstu", "x axis", 0.0, 0,
                "bnstu", "y axis", 0.0, 0,
                "bcdmnstuv", "z axis", 0.0, 4 );


        // pls->poin(N,x,y,1);
        // pls->end();
}

x01cc::x01cc( int argc, const char **argv )
{
   pls = new plstream();

   // Parse and process command line arguments.
   pls->parseopts( &argc, argv, PL_PARSE_FULL );

   // Initialize plplot.
   pls->init();
   plot1();
   delete pls;
}

int main( int argc, const char **argv )
{
   x01cc *x = new x01cc( argc, argv );
   delete x;
}