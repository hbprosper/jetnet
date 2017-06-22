#ifndef JETNET_H
#define JETNET_H
//-----------------------------------------------------------------------------
// File: Jetnet.h
// Purpose: Simple wrapper for JETNET 3.4
// Created: 21-Jan-1999 Harrison B. Prosper
//          27-Jun-2005 HBP Adapt to CMS project structure
//          04-Jul-2005 HBP Add mean and sigmas
//-----------------------------------------------------------------------------
#include <string>
#include <vector>
#include <map>

typedef std::vector<float>  vfloat;
typedef std::vector<double> vdouble;
typedef std::vector<int>    vint;
typedef std::vector<std::string> vstring;

typedef std::vector<std::vector<float> >  vvfloat;
typedef std::vector<std::vector<double> > vvdouble;
typedef std::vector<std::vector<int> >    vvint;
//-----------------------------------------------------------------------------
///
namespace jtn {
  struct ID 
  {
    int   type;
    int   index;
    float value;
    bool  set;
  }; 
  
  typedef std::map<std::string, ID>    mid;
};

/** Feed-forward neural network using JETNET 3.4.
    This is a wrapper around one of the first well-documented neural
    network training codes.
*/
class Jetnet
{
 public:

  enum Divergence
  {
    kBYBINNING = 0,
    kBYMC  = 1
  };

  enum Sample
  {
    kTRAINING = 0,
    kTESTING  = 1
  };

  enum Output
  {
    kSIGMOID = 0,
    kLINEAR  = 1
  };

  // ERROR CODES
  enum Status
  {
    kSUCCESS       = 0,
    kFAILURE       =-1,
    kBADVARSIZE    =-2,
    kUNINITIALIZED =-3,
    kBADNAME       =-4,
    kBADSAMPLE     =-5,
    kFILEOPENERROR =-6,
    kNOWEIGHTS     =-7,
    kBADINPSIZE    =-8,
    kBADOUTSIZE    =-9
  };

  /** Constructor.
   */
  Jetnet(){}

  /** Create a network.
      The network structure is specified by giving the names of the
      input variables and the number of hidden nodes.<br>
      
      @param variab  -   Names of variables (space delimited)
      @param hidden  -   Number of hidden nodes
      @param output - Type of output node
  */
  Jetnet(std::string variab, int hidden, Output output=kSIGMOID);

  /** Create a network.
      The network structure is specified by giving the names of the
      input variables and the number of hidden nodes.<br>
      
      @param variab   -  Vector of variable names
      @param hidden   -  Number of hidden nodes
      @param output - Type of output node
  */
  Jetnet(std::vector<std::string>& variab, int hidden, Output output=kSIGMOID);
    
  /** Create an already-trained network, given a file of weights.
      @param filename - Filename of the weights
  */
  Jetnet(std::string filename);

  ///
  virtual ~Jetnet();
    
  /** Set name of parameter.
      @param name - Name of network parameter
      @param val  - Value of network parameter
  */
  void  setParameter(std::string name, float val);
    
    
  /** Set minimization method.
      <p>
      <table>
      <tr align=left> <td><b>Method Code</b></td>  <td><b>Description</b></td>
      </tr>
	
      <tr align=left> <td>0</td> <td>Stochastic minimization</td>
      </tr>
      
      <tr align=left> <td>1</td> <td>Manhattan</td>
      </tr>
	
      <tr align=left> <td>2</td> <td>Langevin</td>
      </tr>
	
      <tr align=left> <td>3</td> <td>Quickprop</td>
      </tr>
	
      <tr align=left> <td>4</td> <td>Conjugate gradient (Polak-Ribiere)</td>
      </tr>
	
      <tr align=left> <td>5</td> <td>Conjugate gradient (Hestenes-Stiefel)</td>
      </tr>
	
      <tr align=left> <td>6</td> <td>Conjugate gradient (Fletcher-Reeves)</td>
      </tr>
	
      <tr align=left> <td>7</td> <td>Conjugate gradient (Shanno)</td>
      </tr>
	
      </table>

      @param val  - Minimization code
  */
  void  setMethod(int val);
    
  ///
  void  setEta(float val);

  ///
  void  setAlpha(float val);
    
  ///
  void  setEpsilon(float val);
    
  ///
  void  setWidth(float val);

  ///
  void  setDalpha(float val);
    
  ///
  void  setDeta(float val);
    
  /// Set sample (0 for training, 1 for testing).
  void  setSample(Sample sample=kTRAINING);
    
  /** Set pattern.
      @param inp    - Input values
      @param out    - Output value
  */
  void  setPattern(vfloat& inp, float out);

  void  setPattern(vdouble& inp, double out);
 
  ///
  void  loadPatterns(vvfloat& inp,
  		     vfloat&  out);
    
  /// Return value of network training parameter.
  float    parameter(std::string name);

  /// Return names of network inputs.
  vstring  names();
    
  /// Save network weights and, by default, the network function.
  void  save (std::string name, bool savecpp=true);
    
  ///
  void  printParameters(int flag=0);
    
  /// Initialize network training.
  bool  begin(std::string name="");

  /// Save network weights to .jetnet
  void  save();
    
  /// Train network.
  float train();

  /** Test on specified sample.
      Note: The error returned is the mean squared error.
  */
  float test(Sample sample=kTRAINING, float cutpoint=0.5, int numberBins=50);

  /** Compute network power.
	
      \f$\mbox{power} = \frac{1}{2}\int_0^1 |f(x|1) - f(x|0)| \, dx\f$
    
      where 
    
      \f$f(x|1)\f$ 
      
      is the network output probability density for the
      class with desired output &gt; 0.5 and 
      
      \f$f(x|0)\f$ 
      
      is the corresponding
      density for &lt; 0.5. This quantity was	- flag = 0 
      read MLPfit formatted weight file (extension .net).
      - flag = 1 read MLPfit and Jetnet formatted file (extension .jetnet).
      proposed by Dr. Chip Stewart</a>.
  */
  float power();

  /** Compute area under the ROC curve. */
  float area();

  /** Compute network divergence.
	
      \f$\mbox{divergence} = min(D(p|q), D(q|p)) \f$
    
      where 
    
      \f$\kappa(q|p) = \int p(x) ln p(x) / q(x) dx \f$ 
      
      is the Kullback-Leibler divergence between two densities \f$q(x)\f$
      and \f$p(x)\f$. The symmetric version was introduced by 
      Professor Jose Bernardo.
  */
  float divergence(Divergence how=kBYBINNING);

  /** Mis-classification rate.
   */
  float error();

  ///
  vint histogram(int target);
    
  ///
  vfloat efficiencies(int target);

  /// Compute network output for a single output network.
  float evaluate(vfloat& inp);
  
  ///
  float evaluate(vdouble& inp);

  /** False on error.
      @see status
  */
  bool  good();
    
  /** Return status code.
      @see error
  */
  Status status(){ return _status; }

 private:
  Status  _status;
  Sample  _sample;
  int     _outputType;
  bool    _initialized;

  vstring _var;    
  vint    _nodes;
  vfloat  _mean;
  vfloat  _sigma;
  vdouble _wgt;

  vint    _s;
  vint    _b;
  vfloat  _es;
  vfloat  _eb;
  float   _power;
  float   _divergence;
  float   _divergencebyMC;
  float   _error;
  float   _rms;
  float   _area;

  int     _ninput;
  int     _nhidden;
  int     _nlayer;
  int     _noutput;

  jtn::mid     _id;

  std::map<Jetnet::Sample, vvfloat> _input;
  std::map<Jetnet::Sample, vfloat>  _output;

  bool _load (std::string filename, int which=1);    
  void _findscale();
  void _init(std::string vars="", int hidden=-1, Output outType=kSIGMOID);
  void _setpattern(Sample sample);
  void _setParameter(std::string name);
  void _saveCPP(std::string filename);
};

#endif
