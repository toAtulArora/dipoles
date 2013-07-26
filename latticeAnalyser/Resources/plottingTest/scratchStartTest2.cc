#include <iostream>
#include <plplot/plplot.h>
#include <plplot/plstream.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)

  #include <windows.h>

  inline void delay( unsigned long ms )
    {
    Sleep( ms );
    }

#else  /* presume POSIX */

  #include <unistd.h>

  inline void delay( unsigned long ms )
    {
    usleep( ms * 1000 );
    }

#endif 

using namespace std;

int main()
{
	plstream *pls=new plstream();
	pls->init();
	// cout<<"1"<<endl;

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


        for(int i=0;i<90;i++)
        {
        	// pls->bop();
        	// pls->eop();
	        pls->plot3d(&x[i],&y[i],(const double**) (&z[i]),1,1,DRAW_LINEXY,true);
	        // pls->col0(3);
	        pls->poin3(1,&x[i],&y[i],&x[i],1);
	        pls->box3( "bnstu", "x axis", 0.0, 0,
	                "bnstu", "y axis", 0.0, 0,
	                "bcdmnstuv", "z axis", 0.0, 4 );
	        delay(10);

        }

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

        



	cout<<endl;
	cout<<"scratchStartTest initiated";
	delete pls;
	return 0;
}