// Demonstrates a 3rd-order PLL design to track to drifting carrier offset.
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

void export_octave();
int main()
{
    // options
    float        phase_in      =  3.0f;    // carrier phase offset
    float        frequency_in  = -0.20;    // carrier frequency offset
    float        alpha         =  0.08f;   // phase adjustment factor
    unsigned int n             = 1200;     // number of samples
    float        df_in         =  0.30 /(float)n;

    // initialize states
    float beta          = 0.5*alpha*alpha;      // frequency adjustment factor
    float gamma         = 2*powf(alpha,4.0f);   // frequency drift adjustment factor
    float phase_out     = 0.0f;                 // output signal phase
    float frequency_out = 0.0f;                 // output signal frequency
    float df_out        = 0.0f;                 // output signal frequency drift

    // run basic simulation
    int i;
    FILE * fid= fopen("pll_3rd_order_test.dat","w");
    fprintf(fid,"# %6s %6s %6s %6s %9s %9s %12s %12s %12s %12s %12s\n",
            "x-real","x-imag","y-real","y-imag","x-phase","y-phase",
            "x-freq","y-freq","x-dfreq","y-dfreq","phase err");
    for (i=0; i<n; i++) {
        // compute input and output signals
        float complex signal_in  = cexpf(_Complex_I * phase_in);
        float complex signal_out = cexpf(_Complex_I * phase_out);

        // compute phase error estimate
        float phase_error = cargf( signal_in * conjf(signal_out) );

        // print results for plotting
        fprintf(fid,"  %6.3f %6.3f %6.3f %6.3f %9.3f %9.3f %12.4e %12.4e %12.4e %12.4e %12.4e\n",
               crealf(signal_in),  cimagf(signal_in), crealf(signal_out), cimagf(signal_out),
               phase_in, phase_out, frequency_in, frequency_out, df_in, df_out, phase_error);

        // apply loop filter and correct output phase and frequency
        phase_out     += alpha * phase_error;    // adjust phase
        frequency_out +=  beta * phase_error;    // adjust frequency
        df_out        += gamma * phase_error;    // adjust delta frequency

        // increment input and output frequency values
        frequency_in  += df_in;
        frequency_out += df_out;

        // increment input and output phase values
        phase_in  += frequency_in;
        phase_out += frequency_out;
    }
    fclose(fid);
    export_octave();
    printf("results written to 'pll_3rd_order_test.{dat,m}'\n");
    return 0;
}

void export_octave()
{
    FILE * fid = fopen("pll_3rd_order_test.m","w");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"v = load('pll_3rd_order_test.dat');\n");
    fprintf(fid,"n = length(v);\n");
    fprintf(fid,"x = v(:,1) + 1i*v(:,2); phase_x=v(:,5); freq_x=v(:,7); dfreq_x=v(:,9);\n");
    fprintf(fid,"y = v(:,3) + 1i*v(:,4); phase_y=v(:,6); freq_y=v(:,8); dfreq_y=v(:,10);\n");
    fprintf(fid,"phase_error = v(:,11);\n");
    fprintf(fid,"figure('position',[100 100 600 800]);\n");
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"subplot(4,1,1);\n");
    fprintf(fid,"  plot(t,real(x),'LineWidth',2,t,real(y),'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"  legend('input','output');\n");
    fprintf(fid,"subplot(4,1,2);\n");
    fprintf(fid,"  plot(t,phase_error,'LineWidth',2);\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('phase error [radians]');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(4,1,3);\n");
    fprintf(fid,"  plot(t,freq_x-freq_y,'LineWidth',2);\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('frequency error [radians/sample]');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(4,1,4);\n");
    fprintf(fid,"  plot(t,dfreq_x-dfreq_y,'LineWidth',2);\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('delta frequency error [rad/s^2]');\n");
    fprintf(fid,"  grid on;\n");
    fclose(fid);
    }
