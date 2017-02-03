//-----------------------------------------------------------------------------
// File: network.cc
// Purpose: Feed forward neural network utilities
// Created: 17-Sep-2000 Harrison B. Prosper
// Updated: 04-Jul-2005 HBP Add mean and sigmas
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

// Extract name of a file without extension
namespace {
  string nameonly(string filename)
  {
    int i = filename.rfind("/");
    int j = filename.rfind(".");
    if ( j < 0 ) j = filename.size();
    return filename.substr(i+1,j-i-1);
  }
}

extern double sigmoid(double x);
extern double sigmoidout(double x);

// Routine to calculate network outputs recursively

void nnfeed(int l, int k, 
	    vector<int>&    nodes, 
	    vector<double>& weight, 
	    vector<float>&  inp,
	    vector<float>&  out,
	    int outputType)
{
  bool last = (l == (int)(nodes.size()-1));
  double x;
  vector<float> inn(nodes[l]);

  out.clear();
  for (int i = 0; i < nodes[l]; i++)
    {
      x = weight[k]; 
      k++;
      for (int j = 0; j < nodes[l-1]; j++)
	{
	  x = x + weight[k]*inp[j]; 
	  k++;
	}
      float y;
      if ( !last )
	y = sigmoid(x);
      else if ( outputType == 0 )
	y = sigmoidout(x);
      else
	y = x;
      inn[i]  = y;
      out.push_back(y);
    }

  if ( last )
    return;
  else
    nnfeed(l+1,k,nodes,weight,inn,out,outputType);
}


// Routine to write network function recursively
///////////////////////////////////////////////////////////

void nnwrite(int l, int k, 
	     vector<int>&     nodes, 
	     vector<double>&  weight, 
	     vector<string>&  inp, 
	     ostringstream&   os,
	     int ftype,
	     int outputType)
{
  bool last = (l == (int)(nodes.size()-1));

  string prefix, delim, vtype, ctype;
  if ( ftype == 0 )
    {
      prefix = string("      ");
      delim  = string("");
      vtype  = prefix;
      ctype  = string("** ");
    }
  else
    {
      prefix = string("  ");
      delim  = string(";");
      vtype  = prefix + string("double ");
      ctype  = string("  // ");
    }

  vector<string> inn(nodes[l]);
  for (int i = 0; i < nodes[l]; i++)
    {
      os << ctype << "Layer " << l << ", Node " << i << endl;

      os << prefix << "x =     " << setw(12) << weight[k] << delim << endl; 
      k++;
      for (int j = 0; j < (int)inp.size(); j++)
	{
	  if ( weight[k] > 0 )
	    os << prefix << "x = x + " << setw(12) << weight[k];
	  else
	    os << prefix << "x = x - " << setw(12) << fabs(weight[k]);
	  os << " * " << inp[j] << delim << endl; 
	  k++;
	}
      ostringstream s; s << "x" << l << i << "";
      string xs(s.str());

      string sigtype;
      if ( !last )
	sigtype = "sigmoid(x)";
      else if ( outputType == 0 )
	sigtype = "sigmoidout(x)";
      else
	sigtype = "x";

      os   << vtype << xs << " = " << sigtype << delim << endl; 

      inn[i]  = xs;
    }
  if ( last )
    return;
  else
    nnwrite(l+1,k,nodes,weight,inn,os,ftype,outputType);
}

///////////////////////////////////////////////////////////
// Read contents of weight file
// Read weights in MLPfit format
///////////////////////////////////////////////////////////
int nnload(string filename, 
	   vector<int>&     nodes, 
	   vector<double>&  weight,
	   vector<string>&  var,
	   vector<float>&   mean,
	   vector<float>&   sigma,
	   int&             outputType)
{
  // Open weight file

  ifstream stream(filename.c_str());
  if ( !stream ) return -1;

  string header, line;

  // strip away header

  stream >> header >> header >> header;

  // Get list of nodes per layer

  getline(stream, header,'\n'); 
  istringstream str(header);

  nodes.clear();
  int n;
  while ( (str >> n) ) nodes.push_back(n);

  // Skip number of parameters
  
  getline(stream, line,'\n');

  // Read in weights

  weight.clear();
  while ( getline(stream, line, '\n') )
    {
      if ( line[0] == 'I' ) break;
      double w = atof(line.c_str());
      weight.push_back(w);
    }

  // Read names, means and sigmas
  var.clear();
  mean.clear();
  sigma.clear();
  for (int i = 0; i < nodes[0]; i++)
    {
      getline(stream, line,'\n');
      if ( line == "Sigmoid Output" )
	  outputType = 0;
      else if ( line == "Linear Output" )
	  outputType = 1;
      else
	{
	  istringstream is(line);
	  string name;
	  float x, y;
	  is >> name >> x >> y;
	  var.push_back(name);
	  mean.push_back(x);
	  sigma.push_back(y);
	}
    }
  return 0;
}

// Run neural network and return results
///////////////////////////////////////////////////////////
int nncompute(vector<int>&    nodes,
	      vector<double>& weight, 
	      vector<float>&  inp, 
	      vector<float>&  out,
	      int outputType)
{
  if ( weight.size() == 0 ) return -1;
  nnfeed(1,0, nodes, weight, inp, out, outputType);
  return 0;
}

// Write out C++ function

int nnsaveCPP(string title1, 
	      string title2,
	      string sigmoid,
	      string sigmoidout,
	      string filename, 
	      vector<int>&     nodes, 
	      vector<double>&  weight, 
	      vector<string>&  var,
	      vector<float>&   mean,
	      vector<float>&   sigma,
	      int outputType)
{
  if ( weight.size() == 0 ) return -1;

  int ninput  = nodes[0];
  int noutput = nodes[nodes.size()-1];

  time_t tt = time(0);
  string ct(ctime(&tt)); ct = ct.substr(0,24);
  string name = nameonly(filename);

  ofstream out(filename.c_str());
  out << 
    "//-------------------------------------------"
    "----------------------------\n";
  out << "// Function: " << name << endl;
  out << "//           " << title1 << endl;
  out << "//           " << title2 << endl;
  out << "//" << endl;
  for (int i = 0; i < ninput; i++)
    out << "//           " << setw(40) << var[i] 
	<< setw(12) << mean[i] << setw(12) << sigma[i] << endl;
  out << "//" << endl;
  out << "// Created:  " << ct << endl;
  out << 
    "//----------------------------------------"
    "-------------------------------\n";
  out << "#include <cmath>\n";
  out << "#include <vector>\n";
  out << 
    "//-------------------------------------------"
    "----------------------------\n";
  out << "inline\n";
  out << "double sigmoid(double x)\n";
  out << "{\n";
  out << "  return " << sigmoid << ";" << endl;
  out << "}\n";
  out << "inline\n";
  out << "double sigmoidout(double x)\n";
  out << "{\n";
  out << "  return " << sigmoidout << ";" << endl;
  out << "}\n";
  out << "\n";
  out << "void jn" << name << "(double* in, double* out)\n";
  out << "{\n";
  out << "  double x;\n";
  out << "\n";
    for (int i = 0; i < ninput; i++)
    {
      out << "  double x0" << i << " = (in[" << i << "]-" 
	  << "(" << mean[i] << ")" << ")/" << sigma[i] << ";\n"; 
    }

  vector<string> inp(ninput);
  for (int i = 0; i < ninput; i++)
    {
      ostringstream s;
      s << "x0" << i << "";
      inp[i] = string(s.str());
    }

  ostringstream os;
  nnwrite(1,0, nodes, weight, inp, os, 1, outputType);
  out << os.str();

  for (int i = 0; i < noutput; i++)
    {
      out << "  out[" << i << "] = x" 
	  << nodes.size()-1 << i << ";" << endl; 
    }
  out << "}\n\n";

  // Write out a more convenient interface


  string funsig = "double " + name + "(";
  string tab = string("                                                    "
		      "                                                    ").
    substr(0, funsig.size());

  out << "//-----------------------------------------------------------------------\n";

  out << funsig;
  for (int i = 0; i < ninput; i++)
    {
      if (ninput==1) out << "double " << var[i] << ")\n";
      else if ( i == 0 )
	out << "double " << var[i] << "," << endl;
      else if ( i < ninput-1 )
	out << tab << "double " << var[i] << "," << endl;
      else
	out << tab << "double " << var[i] << ")\n";
    }
  out << "{\n";
  out << "  double inp[" << ninput << "];" << endl;
  out << "  double out[" << noutput << "];" << endl; 
  for (int i = 0; i < ninput; i++)
    out << "  inp[" << i << "] = " << var[i] << ";" << endl;
  out << "  jn" << name << "(inp, out);\n";
  out << "  return out[0];\n"; 
  out << "}\n\n";

 funsig = "double " + name + "(std::vector<double>& inp)\n";

  out << "//--------------------------------------------------------";
  out << "--------------\n";

  out << funsig;
  out << "{\n";
  out << "  double out[" << noutput << "];" << endl; 
  out << "  jn" << name << "(&inp[0], out);\n";
  out << "  return out[0];\n"; 
  out << "}\n\n";

  funsig = "extern \"C\" double " + name + "_dl(std::vector<double>& inp)\n";

  out << "//--------------------------------------------------------";
  out << "--------------\n";

  out << funsig;
  out << "{\n";
  out << "  double out[" << noutput << "];" << endl; 
  out << "  jn" << name << "(&inp[0], out);\n";
  out << "  return out[0];\n"; 
  out << "}\n";
  out.close();
  return 0;
}

float nnpower(vector<int>& s, vector<int>& b)
{
  float sums = 0.0;
  float sumb = 0.0;
  int   nbin = min(s.size(),b.size());
  if ( nbin < 1 ) return 0;

  for (int i = 0; i < nbin; i++)
    {
      sums += s[i];
      sumb += b[i];
    }

  float pwr = 0.0;
  for (int i = 0; i < nbin; i++)
    pwr += fabs(s[i]/sums - b[i]/sumb);

  return 0.5*pwr;
}

void nnefficiencies(vector<int>& v, vector<float>& e)
{
  int nbin = v.size();
  vector<float> c(nbin);
  c[0] = v[0];
  for(int bin=1; bin < nbin; bin++) c[bin] = c[bin-1] + v[bin];
  if ( c[nbin-1] > 0 )
    for(int bin=0; bin < nbin; bin++) e[bin] = 1.0-c[nbin-1-bin]/c[nbin-1];
  else
    for(int bin=0; bin < nbin; bin++) e[bin] = (float)bin / nbin;
}

float nnarea(vector<float>& ex, vector<float>& ey)
{
  // Compute Area Under Curve (AUC) using the trapezoid rule.
  // Make sure each interval is at least 0.04 wide

  double area=0;
  int i = 0;
  while ( i < (int)ex.size()-1 )
    {
      double xlo = ex[i];
      double ylo = ey[i];
      double xhi = 0;
      double yhi = 0;

      while ( i < (int)ex.size()-1 )
	{
	  xhi = ex[i+1];
	  yhi = ey[i+1];
	  i++;
	  if ( (xhi-xlo) >= 0.04 ) break;
	}
      area += 0.5 * (yhi+ylo) * (xhi-xlo);
    }
  return area;
}

// Compute Kullback-Leibler divergence

float nndivergence(vector<int>& s, vector<int>& b)
{
  int   nbin = min(s.size(),b.size());
  if ( nbin < 1 ) return -1;

  float sums = 0.0;
  float sumb = 0.0;
  for (int i = 0; i < nbin; i++)
    {
      sums += s[i];
      sumb += b[i];
    }

  float d = 0.0;
  for (int i = 0; i < nbin; i++)
    {
      float p = (float)s[i] / sums;
      float q = (float)b[i] / sumb;
      if ( q > 0 ) d += p*log((p+1.e-30)/q);
    }
  return d;
}













