#!/usr/bin/env python
#-----------------------------------------------------------------------------
# File:        train.py
# Description: test C++ wrapper of JETNET 3.4 fortran code
# Created:     04-Feb-2006 Harrison B. Prosper
#              13-Feb-2006 HBP
#              20-Mar-2014 HBP small update
#-----------------------------------------------------------------------------
import sys, os
from string import *
from ROOT import *
#-----------------------------------------------------------------------------
# Constants
#------------------------------------------------------------------------------
SIGFILE= 'ttbar.dat'
BKGFILE= 'nonttbar.dat'
NEPOCH = 10000  # Number of epochs
NSTEP  = 10     # Plot every NSTEP epoch
NTRAIN = 10000  # Number of training events/file
NHIDDEN= 10     # Number of hidden nodes

KPLOT  = 5
NBIN   = 50
#-----------------------------------------------------------------------------
def error(message):
    sys.exit("*** Error *** %s" % message)
#-----------------------------------------------------------------------------
def readData(filename):
    print "reading data from %s..." % filename
    records = map(split, open(filename).readlines())
    header  = records[0]
    records = records[1:]
    varmap  = {}
    for index, name in enumerate(header):
        varmap[name] = index
    return (varmap, records)
#-----------------------------------------------------------------------------
def loadNetwork(title,
        varmap,
        sig,
        bkg,
        vars,
        nrows,
        sample, 
        nn):
    print title

    nrows = min(nrows, min(len(sig), len(bkg)))

    # Set either Test or Train
    nn.setSample(sample)

    input = vdouble(len(vars))

    for record in sig[:nrows]:
        for i, var in enumerate(vars):
            input[i] = atof(record[varmap[var]])
        nn.setPattern(input, 1)

    for record in bkg[:nrows]:
        for i, var in enumerate(vars):
            input[i] = atof(record[varmap[var]])
        nn.setPattern(input, 0)
#-----------------------------------------------------------------------------
class Plot:

    def __init__(self, netname, nepoch, nbin):
        
        gStyle.SetOptStat(0)
        
        kbin   = nepoch / (KPLOT*NSTEP)
        
        self.coutput = TCanvas("fig_%s_train" % netname,
                       "Training Jetnet", 700, 0, 500, 500)

        # Network output distributions

        self.hs = TH1F("hs", "", nbin, 0, 1)
        self.hs.SetLineColor(kRed)
        self.hs.GetXaxis().SetTitle("network output")
        self.hs.SetFillColor(kRed)
        self.hs.SetFillStyle(3004)

        self.hb = TH1F("hb", "", nbin, 0, 1)
        self.hb.SetLineColor(kBlue)
        self.hb.GetXaxis().SetTitle("network output")        
        self.hb.SetFillColor(kBlue)
        self.hb.SetFillStyle(3005)

        # error difference vs epoch number

        self.cerror = TCanvas("fig_%s_series" % netname,
                      "Time Series", 0, 0, 500, 500)

        self.hd = TH1F("hd", "", kbin, 0, nepoch)
        self.hd.GetXaxis().SetTitle("epoch")
        self.hd.GetYaxis().SetTitle("RMS(test) - RMS(train)")           
        self.hg = TGraph()
        self.hg.SetLineColor(kBlue)
        self.hg.SetMarkerStyle(20)
        self.hg.SetMarkerColor(kBlue)
        self.hg.SetMarkerSize(0.4)
        self.emax =-10000.0
        self.emin = 10000.0
        
    def __del__(self):
        pass

    def hist(self, nn):
        hs = self.hs
        hb = self.hb

        b  = nn.histogram(0)
        s  = nn.histogram(1)
        for i in xrange(len(s)):
            hs.SetBinContent(i+1, s[i])
            hb.SetBinContent(i+1, b[i])

        self.coutput.cd()
        if hs.GetMaximum() > hb.GetMaximum():
            hs.Draw('hist')
            hb.Draw("hist SAME")
        else:
            hb.Draw('hist')
            hs.Draw("hist SAME")
        self.coutput.Update()

    def series(self, epoch, errortrain, errortest):
        ediff = errortest-errortrain
        self.hg.SetPoint(epoch, epoch, ediff)

        if ediff > self.emax: self.emax = ediff
        if ediff < self.emin: self.emin = ediff

        if self.emin < 0:
            hd.SetMinimum(1.05*self.emin)
        else:
            hd.SetMinimum(0.95*self.emin)
        hd.SetMaximum(1.05*self.emax)
        
        self.cerror.cd()
        self.hd.Draw('hist')
        self.hg.Draw("p same")
        self.cerror.Update()

    def save(self, gtype=".png"):
        self.coutput.SaveAs(gtype)
        self.cerror.SaveAs(gtype)
#-----------------------------------------------------------------------------
def main():
    netname = 'ttbarnet'
    varfile = "%s.vars" % netname
    if not os.path.exists(varfile): error("Can't find %s" % varfile)

    # load library

    gSystem.Load('libjetnet.so')

    # Set defaults

    sigfile = SIGFILE
    bkgfile = BKGFILE
    nhidden = NHIDDEN
    nepoch  = NEPOCH
    ntrain  = NTRAIN

    if not os.path.exists(bkgfile): error("Can't find %s" % bkgfile)
    if not os.path.exists(sigfile): error("Can't find %s" % sigfile)
    #-----------------------------------------------------------------------
    # Set up plots

    plot = Plot(netname, nepoch, NBIN)

    #-----------------------------------------------------------------------
    # Get variable names and data
    #-----------------------------------------------------------------------
    vlist = filter(lambda x: x != "",
               map(strip, open(varfile).readlines()))
    varlist = vector('string')()
    print "Variables"
    for x in vlist:
        print "\t%s" % x
        varlist.push_back(x)

    varmap, sig = readData(sigfile)
    varmap, bkg = readData(bkgfile)

    ntrain = min( min(len(sig), len(bkg)) / 2 , ntrain)
    ntest  = ntrain
    print "Number of training events/file %s\n" % ntrain 
    #-----------------------------------------------------------------------
    # Define network
    #-----------------------------------------------------------------------
    nn = Jetnet(varlist, nhidden)

    loadNetwork("load training data",
            varmap,
            sig,
            bkg,
            varlist,
            ntrain,
            Jetnet.kTRAINING, nn)

    loadNetwork("load testing data",
            varmap,
            sig[ntrain:],
            bkg[ntrain:],
            varlist,
            ntest,
            Jetnet.kTESTING, nn)

    #-----------------------------------------------------------------------
    # Train!
    #-----------------------------------------------------------------------
    nn.begin()
    nn.printParameters()

    # RMS:   Error function: sqrt(Sum (t_i - net(x_i))**2 / N)
    print "%10s %10s %10s" % ("epoch", "RMS(train)", "RMS(test)")
    
    kplot = 0
    for epoch in xrange(nepoch):

        nn.train()

        if epoch % NSTEP == 0:

            rms0 = nn.test(Jetnet.kTRAINING, 0.5, NBIN)
            rms1 = nn.test(Jetnet.kTESTING, 0.5, NBIN)

            kplot += 1
            if kplot == KPLOT:
                kplot = 0
                print "%10d %10.4f %10.4f" % (epoch, rms0, rms1)
                plot.hist(nn)        
                plot.series(epoch, rms0, rms1)

                nn.end()
    plot.save()

    #-----------------------------------------------------------------------
    # Save weights and network function
    #-----------------------------------------------------------------------
    nn.save(netname)
    os.system("rm -rf *.net")
#---------------------------------------------------------------------------
try:
    main()
except KeyboardInterrupt:
    print '\nciao!'
    
