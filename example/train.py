#!/usr/bin/env python
#-----------------------------------------------------------------------------
# File:        train.py
# Description: build a shallow neural network to discriminate between ttbar
#              and non-ttbar events using the ancient JETNET 3.4 fortran code
# Created:     04-Feb-2006 Harrison B. Prosper
#              13-Feb-2006 HBP
#              20-Mar-2014 HBP small update
#              16-Jun-2016 HBP add more comments
#-----------------------------------------------------------------------------
import sys, os
from string import *
from ROOT import *
#-----------------------------------------------------------------------------
# Constants
#------------------------------------------------------------------------------
SIGFILE= 'ttbar.dat'
BKGFILE= 'nonttbar.dat'
NEPOCH = 1000   # Number of epochs
NSTEP  = 10     # Plot every NSTEP epoch
NTRAIN = 10000  # Number of training events/file
NHIDDEN= 10     # Number of hidden nodes

KPLOT  = 5
NBIN   = 50
#-----------------------------------------------------------------------------
def error(message):
    sys.exit("*** Error *** %s" % message)
#-----------------------------------------------------------------------------
# read from a text file containing a table of numbers. The first line is a
# header
def readData(filename):
    print "reading data from %s..." % filename
    records = map(split, open(filename).readlines())
    header  = records[0]
    # convert to floating point numbers
    records = map(lambda x: map(atof, x), records[1:])
    
    varmap  = {}
    for index, name in enumerate(header):
        varmap[name] = index

    return (varmap, records)

def getVarnames(varfile):
    vlist = filter(lambda x: x != "", map(strip, open(varfile).readlines()))
    varlist = vector('string')()
    print "Variables"
    for x in vlist:
        print "\t%s" % x
        varlist.push_back(x)
    return varlist
#-----------------------------------------------------------------------------
def loadNetwork(title,
        varmap,
        sig,
        bkg,
        variables,
        nrows,
        sample, 
        nn):
    print title

    nrows = min(nrows, min(len(sig), len(bkg)))

    # Set either test or training sample
    nn.setSample(sample)

    # inputs are a C++ vector of doubles
    inpvar = vector('double')(len(variables))

    # load signals
    target = 1
    for record in sig[:nrows]:
        for i, var in enumerate(variables):
            inpvar[i] = record[varmap[var]]
            
        nn.setPattern(inpvar, target)

    # load backgrounds
    target = 0
    for record in bkg[:nrows]:
        for i, var in enumerate(variables):
            inpvar[i] = record[varmap[var]]
            
        nn.setPattern(inpvar, target)
#-----------------------------------------------------------------------------
class Plot:

    def __init__(self, netname, nepoch, nbin):
        
        gStyle.SetOptStat(0)
        
        kbin   = nepoch / (KPLOT*NSTEP)
        
        self.coutput = TCanvas("fig_%s_train" % netname,
                       "Training Jetnet", 700, 0, 500, 500)

        # Network output distributions

        self.hs = TH1F("hs", "", nbin, 0, 1)
        self.hs.SetLineColor(kAzure+1)
        self.hs.GetXaxis().SetTitle("network output")
        self.hs.SetFillColor(kAzure+1)
        self.hs.SetFillStyle(3004)

        self.hb = TH1F("hb", "", nbin, 0, 1)
        self.hb.SetLineColor(kMagenta+1)
        self.hb.GetXaxis().SetTitle("network output")        
        self.hb.SetFillColor(kMagenta+1)
        self.hb.SetFillStyle(3005)

        # error difference vs epoch number

        self.cerror = TCanvas("fig_%s_series" % netname,
                      "Time Series", 0, 0, 500, 500)

        self.hd = TH1F("hd", "", nepoch, 0, nepoch)
        self.hd.GetXaxis().SetTitle("epoch")
        self.hd.GetYaxis().SetTitle("RMS")        
        self.hd.SetMinimum(0)
        self.hd.SetMaximum(1)
        
        self.htrain = TGraph()
        self.htrain.SetLineColor(kBlue)
        
        self.htest = TGraph()
        self.htest.SetLineColor(kRed)

        self.np = 0
        self.emin = 1e10
        self.emax =-1e10
        
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
            hb.Draw("hist same")
        else:
            hb.Draw('hist')
            hs.Draw("hist same")
        self.coutput.Update()
        gSystem.ProcessEvents()
        
    def series(self, epoch, errortrain, errortest):
        self.np += 1
        self.htrain.SetPoint(self.np, epoch, errortrain)
        self.htest.SetPoint(self.np,  epoch, errortest)

        errmin = min(errortrain, errortest)
        errmax = max(errortrain, errortest)
        if errmin < self.emin: self.emin = errmin
        if errmax > self.emax: self.emax = errmax
        self.hd.SetMinimum(0.95*self.emin)
        self.hd.SetMaximum(1.05*self.emax)
        
        self.cerror.cd()
        self.hd.Draw()
        self.htrain.Draw("lsame")
        self.htest.Draw("lsame")
        self.cerror.Update()
        gSystem.ProcessEvents()
        
    def save(self, gtype=".png"):
        self.coutput.SaveAs(gtype)
        self.cerror.SaveAs(gtype)
#-----------------------------------------------------------------------------
def main():
    netname = 'ttbarnet'
    varfile = "%s.vars" % netname
    if not os.path.exists(varfile): error("Can't find %s" % varfile)

    # load library

    gSystem.AddDynamicPath("$JETNET_PATH/lib")
    if gSystem.Load('libjetnet') < 0: error("unable to load libjetnet")

    # Set defaults

    sigfile = SIGFILE
    bkgfile = BKGFILE
    nhidden = NHIDDEN
    nepoch  = NEPOCH
    ntrain  = NTRAIN

    if not os.path.exists(bkgfile): error("Can't find %s" % bkgfile)
    if not os.path.exists(sigfile): error("Can't find %s" % sigfile)

    # Set up plots

    plot = Plot(netname, nepoch, NBIN)

    #-----------------------------------------------------------------------
    # Get data
    #-----------------------------------------------------------------------
    varmap, sig = readData(sigfile)
    varmap, bkg = readData(bkgfile)

    ntrain = min( min(len(sig), len(bkg)) / 2 , ntrain)
    ntest  = ntrain
    print "Number of training events/file %s\n" % ntrain 
    #-----------------------------------------------------------------------
    # Define network
    #-----------------------------------------------------------------------
    varlist =  getVarnames(varfile)
    
    nn = Jetnet(varlist, nhidden)

    loadNetwork("load training data",
            varmap,
            sig,
            bkg,
            varlist,
            ntrain,
            Jetnet.kTRAINING, nn)

    loadNetwork("load test data",
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

            # get RMS errors
            rms0 = nn.test(Jetnet.kTRAINING, 0.5, NBIN)
            rms1 = nn.test(Jetnet.kTESTING,  0.5, NBIN)

            kplot += 1
            if kplot == KPLOT:
                kplot = 0
                print "%10d %10.4f %10.4f" % (epoch, rms0, rms1)
                plot.hist(nn)        
                plot.series(epoch, rms0, rms1)
                
                # save weights to .jetnet
                nn.save()
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
    
