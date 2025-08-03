#!/usr/bin/env python3
'''plot frame sync debug files'''
import argparse, os, sys
import numpy as np, matplotlib.pyplot as plt

def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('sources', nargs='+', help='input files')
    p.add_argument('-export', default=None, action='store_true', help='export figure')
    p.add_argument('-nodisplay', action='store_true', help='disable display')
    args = p.parse_args()

    for fname in args.sources:
        filename = framesync64_plot(fname,args.export,args.nodisplay)

def framesync64_plot(filename,export=None,nodisplay=True):
    # open file and read values
    fid         = open(filename,'rb')
    buf         = np.fromfile(fid, count=1440, dtype=np.csingle)
    tau_hat     = np.fromfile(fid, count=   1, dtype=np.single)
    dphi_hat    = np.fromfile(fid, count=   1, dtype=np.single)
    phi_hat     = np.fromfile(fid, count=   1, dtype=np.single)
    gamma_hat   = np.fromfile(fid, count=   1, dtype=np.single)
    evm         = np.fromfile(fid, count=   1, dtype=np.single)
    payload_rx  = np.fromfile(fid, count= 630, dtype=np.csingle)
    payload_sym = np.fromfile(fid, count= 600, dtype=np.csingle)
    payload_dec = np.fromfile(fid, count=  72, dtype=np.int8)

    # compute smooth spectral response in dB
    nfft = 2400
    f = np.arange(nfft)/nfft-0.5
    psd = np.abs(np.fft.fftshift(np.fft.fft(buf, nfft)))**2
    m   = int(0.01*nfft)
    w   = np.hamming(2*m+1)
    h   = np.concatenate((w[m:], np.zeros(nfft-2*m-1), w[:m])) / (sum(w) * nfft)
    H   = np.fft.fft(h)
    psd = 10*np.log10( np.real(np.fft.ifft(H * np.fft.fft(psd))) )

    # plot impulse and spectral responses
    fig, _ax = plt.subplots(2,2,figsize=(12,12))
    ax = _ax.flatten()
    t = np.arange(len(buf))
    qpsk = np.exp(0.5j*np.pi*(np.arange(4)+0.5)) # payload constellation
    ax[0].plot(t,np.real(buf), t,np.imag(buf))
    ax[0].set_title('Raw I/Q Samples')
    ax[1].plot(f,psd)
    ax[1].set(xlim=(-0.5,0.5))
    ax[1].set_title('Power Spectral Density')
    ax[2].plot(np.real(payload_rx),  np.imag(payload_rx),  '.')
    ax[2].set_title('RX Payload Syms')
    ax[3].plot(np.real(payload_sym), np.imag(payload_sym), '.')
    ax[3].set_title('Synchronized Payload Syms')
    for _ax in ax[2:]:
        _ax.set(xlim=(-1.3,1.3), ylim=(-1.3,1.3))
        _ax.plot(np.real(qpsk),np.imag(qpsk),'rx')
        _ax.set_xlabel('Real')
        _ax.set_ylabel('Imag')
    for _ax in ax:
        _ax.grid(True)
    title = '%s, tau:%9.6f, dphi:%9.6f, phi:%9.6f, rssi:%6.3f dB, evm:%6.3f' % \
        (filename, tau_hat, dphi_hat, phi_hat, 20*np.log10(gamma_hat), evm)
    print(title)
    fig.suptitle(title)
    if not nodisplay:
        plt.show()
    if export is not None:
        fig.savefig(os.path.splitext(filename)[0]+'.png',bbox_inches='tight')
    plt.close()

if __name__ == '__main__':
    sys.exit(main())

