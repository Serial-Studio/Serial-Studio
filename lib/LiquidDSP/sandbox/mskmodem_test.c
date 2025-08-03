// 
// mskmodem_test.c
//

#if 0
 NOTES

  mod.    signal median(|z|)
  index                             (GMSK)
  (h)     (rcos-full)   (rcos-half) BT=0.35
  0.1     0.84078       0.72392     0.66000
  0.15    0.84900       0.73578     0.66909
  0.2     0.86020       0.75219     0.68140
  0.25    0.87410       0.77380     0.69923
  0.3     0.89051       0.80083     0.72071
  0.35    0.90936       0.83370     0.75068
  0.4     0.93050       0.87366     0.78479
  0.45    0.95379       0.92185     0.81376
  0.5     0.97901       0.97473     0.85897
  0.55    1.0055        1.0301      0.91067
  0.6     1.0324        1.0915      0.96746
  0.65    1.0582        1.1631      1.0117
  0.7     1.0813        1.2295      1.0777
  0.75    1.0998        1.3012      1.1524
  0.8     1.1116        1.3691      1.2040
  0.85    1.1146        1.4171      1.2628
  0.9     1.1074        1.4333      1.3158
 
 rcos-full: median(|z|) ~ 0.34146  * h^2  +   0.14371  * h  +  0.81937
 rcos-half: median(|z|) ~ 1.143219 * h^2  +  -0.064886 * h  +  0.718770
 GMSK:      median(|z|) ~ 0.967978 * h^2  +  -0.077964 * h  +  0.658288
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "mskmodem_test.m"

// print usage/help message
void usage()
{
    printf("mskmodem_test -- minimum-shift keying modem example\n");
    printf("options:\n");
    printf(" -h         : print help\n");
    printf(" -t <type>  : filter type: [square], rcos-full, rcos-half, gmsk\n");
    printf(" -k <sps>   : samples/symbol,         default:  8\n");
    printf(" -b <bps>   : bits/symbol,            default:  1\n");
    printf(" -H <index> : modulation index,       default:  0.5\n");
    printf(" -B <beta>  : filter roll-off,        default:  0.35\n");
    printf(" -n <num>   : number of data symbols, default: 20\n");
    printf(" -s <snr>   : SNR [dB],               default: 80\n");
    printf(" -F <freq>  : carrier freq. offset,   default:  0.0\n");
    printf(" -P <phase> : carrier phase offset,   default:  0.0\n");
}

int main(int argc, char*argv[]) {
    // options
    unsigned int k  = 8;        // filter samples/symbol
    unsigned int bps= 1;        // number of bits/symbol
    float h         = 0.5f;     // modulation index (h=1/2 for MSK)
    unsigned int num_data_symbols = 20; // number of data symbols
    float SNRdB     = 80.0f;    // signal-to-noise ratio [dB]
    float cfo       = 0.0f;     // carrier frequency offset
    float cpo       = 0.0f;     // carrier phase offset
    enum {
        TXFILT_SQUARE=0,
        TXFILT_RCOS_FULL,
        TXFILT_RCOS_HALF,
        TXFILT_GMSK,
    } tx_filter_type = TXFILT_SQUARE;
    float gmsk_bt = 0.35f;              // GMSK bandwidth-time factor

    int dopt;
    while ((dopt = getopt(argc,argv,"ht:k:b:H:B:n:s:F:P:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                         return 0;
        case 't':
            if (strcmp(optarg,"square")==0) {
                tx_filter_type = TXFILT_SQUARE;
            } else if (strcmp(optarg,"rcos-full")==0) {
                tx_filter_type = TXFILT_RCOS_FULL;
            } else if (strcmp(optarg,"rcos-half")==0) {
                tx_filter_type = TXFILT_RCOS_HALF;
            } else if (strcmp(optarg,"gmsk")==0) {
                tx_filter_type = TXFILT_GMSK;
            } else {
                fprintf(stderr,"error: %s, unknown filter type '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        case 'k': k = atoi(optarg);                break;
        case 'b': bps = atoi(optarg);              break;
        case 'H': h = atof(optarg);                break;
        case 'B': gmsk_bt = atof(optarg);          break;
        case 'n': num_data_symbols = atoi(optarg); break;
        case 's': SNRdB = atof(optarg);            break;
        case 'F': cfo   = atof(optarg);            break;
        case 'P': cpo   = atof(optarg);            break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int num_symbols = num_data_symbols;
    unsigned int num_samples = k*num_symbols;
    unsigned int M = 1 << bps;              // constellation size
    float nstd = powf(10.0f, -SNRdB/20.0f);

    // arrays
    unsigned char sym_in[num_symbols];      // input symbols
    float phi[num_samples];                 // transmitted phase
    float complex x[num_samples];           // transmitted signal
    float complex y[num_samples];           // received signal
    float complex z[num_samples];           // output...
    //unsigned char sym_out[num_symbols];     // output symbols

    unsigned int ht_len = 0;
    unsigned int tx_delay = 0;
    float * ht = NULL;
    switch (tx_filter_type) {
    case TXFILT_SQUARE:
        // regular MSK
        ht_len = k;
        tx_delay = 1;
        ht = (float*) malloc(ht_len *sizeof(float));
        for (i=0; i<ht_len; i++)
            ht[i] = h * M_PI / (float)k;
        break;
    case TXFILT_RCOS_FULL:
        // full-response raised-cosine pulse
        ht_len = k;
        tx_delay = 1;
        ht = (float*) malloc(ht_len *sizeof(float));
        for (i=0; i<ht_len; i++)
            ht[i] = h * M_PI / (float)k * (1.0f - cosf(2.0f*M_PI*i/(float)ht_len));
        break;
    case TXFILT_RCOS_HALF:
        // partial-response raised-cosine pulse
        ht_len = 3*k;
        tx_delay = 2;
        ht = (float*) malloc(ht_len *sizeof(float));
        for (i=0; i<ht_len; i++)
            ht[i] = 0.0f;
        for (i=0; i<2*k; i++)
            ht[i+k/2] = h * 0.5f * M_PI / (float)k * (1.0f - cosf(2.0f*M_PI*i/(float)(2*k)));
        break;
    case TXFILT_GMSK:
        ht_len = 2*k*3+1+k;
        tx_delay = 4;
        ht = (float*) malloc(ht_len *sizeof(float));
        for (i=0; i<ht_len; i++)
            ht[i] = 0.0f;
        liquid_firdes_gmsktx(k,3,gmsk_bt,0.0f,&ht[k/2]);
        for (i=0; i<ht_len; i++)
            ht[i] *= h * 2.0f / (float)k;
        break;
    default:
        fprintf(stderr,"error: %s, invalid tx filter type\n", argv[0]);
        exit(1);
    }
    for (i=0; i<ht_len; i++)
        printf("ht(%3u) = %12.8f;\n", i+1, ht[i]);
    firinterp_rrrf interp_tx = firinterp_rrrf_create(k, ht, ht_len);

    // generate symbols and interpolate
    // phase-accumulating filter (trapezoidal integrator)
    float b[2] = {0.5f,  0.5f};
    if (tx_filter_type == TXFILT_SQUARE) {
        // square filter: rectangular integration with one sample of delay
        b[0] = 0.0f;
        b[1] = 1.0f;
    }
    float a[2] = {1.0f, -1.0f};
    iirfilt_rrrf integrator = iirfilt_rrrf_create(b,2,a,2);
    float theta = 0.0f;
    for (i=0; i<num_symbols; i++) {
        sym_in[i] = rand() % M;
        float v = 2.0f*sym_in[i] - (float)(M-1);    // +/-1, +/-3, ... +/-(M-1)
        firinterp_rrrf_execute(interp_tx, v, &phi[k*i]);

        // accumulate phase
        unsigned int j;
        for (j=0; j<k; j++) {
            iirfilt_rrrf_execute(integrator, phi[i*k+j], &theta);
            x[i*k+j] = cexpf(_Complex_I*theta);
        }
    }
    iirfilt_rrrf_destroy(integrator);

    // push through channel
    for (i=0; i<num_samples; i++) {
        // add carrier frequency/phase offset
        y[i] = x[i]*cexpf(_Complex_I*(cfo*i + cpo));

        // add noise
        y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }
    
    // create decimator
    unsigned int m = 3;
    float bw = 0.0f;
    float beta = 0.0f;
    float scale = 1.0f;
    firfilt_crcf decim_rx = NULL;
    switch (tx_filter_type) {
    case TXFILT_SQUARE:
        //bw = 0.9f / (float)k;
        bw = 0.4f;
        decim_rx = firfilt_crcf_create_kaiser(2*k*m+1, bw, 60.0f, 0.0f);
        scale = 2.0f * bw;
        break;
    case TXFILT_RCOS_FULL:
        if (M==2) {
            decim_rx = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,k,m,0.5f,0);
            scale = 1.33f / (float)k;
        } else {
            decim_rx = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,k/2,2*m,0.9f,0);
            scale = 3.25f / (float)k;
        }
        break;
    case TXFILT_RCOS_HALF:
        if (M==2) {
            // rcos-half:               
            decim_rx = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,k,m,0.3f,0);
            scale = 1.1f / (float)k;
            //scale = 1.1f / (float)k * 1.0f / ( 1.24887*h*h  -0.16103*h +  0.73256 );
        } else {
            decim_rx = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,k/2,2*m,0.27f,0);
            scale = 2.90f / (float)k;
        }
        break;
    case TXFILT_GMSK:
        bw = 0.5f / (float)k;
        // TODO: figure out beta value here
        beta = (M == 2) ? 0.8*gmsk_bt : 1.0*gmsk_bt;
        decim_rx = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,k,m,beta,0);
        // compensate gain for modulation index
        scale = 2.0f * bw / (0.967978*h*h + -0.077964*h + 0.658288);
        break;
    default:
        fprintf(stderr,"error: %s, invalid tx filter type\n", argv[0]);
        exit(1);
    }
    firfilt_crcf_set_scale(decim_rx, scale);
    printf("bw = %f\n", bw);

    // run receiver
    unsigned int n=0;
    unsigned int num_errors = 0;
    unsigned int num_symbols_checked = 0;
    float complex z_prime = 0.0f;
    for (i=0; i<num_samples; i++) {
        // push through filter
        firfilt_crcf_push(decim_rx, y[i]);
        firfilt_crcf_execute(decim_rx, &z[i]);

        // decimate output
        if ( (i%k)==0 ) {
            // compute instantaneous frequency scaled by modulation index
            float phi_hat = cargf(conjf(z_prime) * z[i]) / (h * M_PI);

            // estimate transmitted symbol
            float v = (phi_hat + (M-1.0))*0.5f;
            unsigned int sym_out = ((int) roundf(v)) % M;

            // save current point
            z_prime = z[i];

            // print result to screen
            printf("%3u : %12.8f + j%12.8f, <f=%8.4f : %8.4f> (%1u)",
                    n, crealf(z[i]), cimagf(z[i]), phi_hat, v, sym_out);
            if (n >= m+tx_delay) {
                num_errors += (sym_out == sym_in[n-m-tx_delay]) ? 0 : 1;
                num_symbols_checked++;
                printf(" (%1u)\n", sym_in[n-m-tx_delay]);
            } else {
                printf("\n");
            }
            n++;
        }
    }

    // print number of errors
    printf("errors : %3u / %3u\n", num_errors, num_symbols_checked);

    // destroy objects
    firinterp_rrrf_destroy(interp_tx);
    firfilt_crcf_destroy(decim_rx);

    // compute power spectral density of transmitted signal
    unsigned int nfft = 1024;
    float psd[nfft];
    spgramcf_estimate_psd(nfft, y, num_samples, psd);

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"h = %f;\n", h);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"nfft        = %u;\n", nfft);
    fprintf(fid,"delay       = %u; %% receive filter delay\n", tx_delay);

    fprintf(fid,"x   = zeros(1,num_samples);\n");
    fprintf(fid,"y   = zeros(1,num_samples);\n");
    fprintf(fid,"z   = zeros(1,num_samples);\n");
    fprintf(fid,"phi = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(z[i]), cimagf(z[i]));
        fprintf(fid,"phi(%4u) = %12.8f;\n", i+1, phi[i]);
    }
    // save PSD
    fprintf(fid,"psd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%4u) = %12.8f;\n", i+1, psd[i]);

    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"i = 1:k:num_samples;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,4,1:3);\n");
    fprintf(fid,"  plot(t,real(x),'-', t(i),real(x(i)),'bs','MarkerSize',4,...\n");
    fprintf(fid,"       t,imag(x),'-', t(i),imag(x(i)),'gs','MarkerSize',4);\n");
    fprintf(fid,"  axis([0 num_symbols -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('x(t)');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,4,5:7);\n");
    fprintf(fid,"  plot(t-delay,real(z),'-', t(i)-delay,real(z(i)),'bs','MarkerSize',4,...\n");
    fprintf(fid,"       t-delay,imag(z),'-', t(i)-delay,imag(z(i)),'gs','MarkerSize',4);\n");
    fprintf(fid,"  axis([0 num_symbols -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('\"matched\" filter output');\n");
    fprintf(fid,"  grid on;\n");
    // plot I/Q constellations
    fprintf(fid,"subplot(3,4,4);\n");
    fprintf(fid,"  plot(real(y),imag(y),'-',real(y(i)),imag(y(i)),'rs','MarkerSize',3);\n");
    fprintf(fid,"  xlabel('I');\n");
    fprintf(fid,"  ylabel('Q');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,4,8);\n");
    fprintf(fid,"  plot(real(z),imag(z),'-',real(z(i)),imag(z(i)),'rs','MarkerSize',3);\n");
    fprintf(fid,"  xlabel('I');\n");
    fprintf(fid,"  ylabel('Q');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    // plot PSD
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"subplot(3,4,9:12);\n");
    fprintf(fid,"  plot(f,psd,'LineWidth',1.5);\n");
    fprintf(fid,"  axis([-0.5 0.5 -40 20]);\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fprintf(fid,"  grid on;\n");

#if 0
    fprintf(fid,"figure;\n");
    fprintf(fid,"  %% compute instantaneous received frequency\n");
    fprintf(fid,"  freq_rx = arg( conj(z(:)) .* circshift(z(:),-1) )';\n");
    fprintf(fid,"  freq_rx(1:(k*delay)) = 0;\n");
    fprintf(fid,"  freq_rx(end) = 0;\n");
    fprintf(fid,"  %% compute instantaneous tx/rx phase\n");
    if (tx_filter_type == TXFILT_SQUARE) {
        fprintf(fid,"  theta_tx = filter([0 1],[1 -1],phi)/(h*pi);\n");
        fprintf(fid,"  theta_rx = filter([0 1],[1 -1],freq_rx)/(h*pi);\n");
    } else {
        fprintf(fid,"  theta_tx = filter([0.5 0.5],[1 -1],phi)/(h*pi);\n");
        fprintf(fid,"  theta_rx = filter([0.5 0.5],[1 -1],freq_rx)/(h*pi);\n");
    }
    fprintf(fid,"  %% plot instantaneous tx/rx phase\n");
    fprintf(fid,"  plot(t,      theta_tx,'-b', t(i),      theta_tx(i),'sb',...\n");
    fprintf(fid,"       t-delay,theta_rx,'-r', t(i)-delay,theta_rx(i),'sr');\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('instantaneous phase/(h \\pi)');\n");
    fprintf(fid,"  legend('transmitted','syms','received/filtered','syms','location','northwest');\n");
    fprintf(fid,"  grid on;\n");
#else
    // plot filter response
    fprintf(fid,"ht_len = %u;\n", ht_len);
    fprintf(fid,"ht     = zeros(1,ht_len);\n");
    for (i=0; i<ht_len; i++)
        fprintf(fid,"ht(%4u) = %12.8f;\n", i+1, ht[i]);
    fprintf(fid,"gt1 = filter([0.5 0.5],[1 -1],ht) / (pi*h);\n");
    fprintf(fid,"gt2 = filter([0.0 1.0],[1 -1],ht) / (pi*h);\n");
    fprintf(fid,"tfilt = [0:(ht_len-1)]/k - delay + 0.5;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(tfilt,ht, '-x','MarkerSize',4,...\n");
    fprintf(fid,"     tfilt,gt1,'-x','MarkerSize',4,...\n");
    fprintf(fid,"     tfilt,gt2,'-x','MarkerSize',4);\n");
    fprintf(fid,"axis([tfilt(1) tfilt(end) -0.1 1.1]);\n");
    fprintf(fid,"legend('pulse','trap. int.','rect. int.','location','northwest');\n");
    fprintf(fid,"grid on;\n");
#endif

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);
    
    // free allocated filter memory
    free(ht);

    return 0;
}
