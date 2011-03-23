#include "weakform/weakform.h"
#include "integrals/integrals_h1.h"
#include "boundaryconditions/essential_bcs.h"
#include "weakform_library/laplace.h"

/* Right-hand side */

class CustomRightHandSide: public DefaultNonConstRightHandSide
{
public:
  CustomRightHandSide(double coeff1) : DefaultNonConstRightHandSide(coeff1) {};

  virtual double value(double x, double y) {
    return -coeff1*(-2*pow(M_PI,2)*(1 - exp(-(1 - x)/coeff1))*(1 - exp(-(1 - y)/coeff1))*cos(M_PI*(x + y)) 
           + 2*M_PI*(1 - exp(-(1 - x)/coeff1))*exp(-(1 - y)/coeff1)*sin(M_PI*(x + y))/coeff1 
           + 2*M_PI*(1 - exp(-(1 - y)/coeff1))*exp(-(1 - x)/coeff1)*sin(M_PI*(x + y))/coeff1 
           - (1 - exp(-(1 - y)/coeff1))*cos(M_PI*(x + y))*exp(-(1 - x)/coeff1)/pow(coeff1,2) 
           - (1 - exp(-(1 - x)/coeff1))*cos(M_PI*(x + y))*exp(-(1 - y)/coeff1)/pow(coeff1,2)) 
           - 3*M_PI*(1 - exp(-(1 - x)/coeff1))*(1 - exp(-(1 - y)/coeff1))*sin(M_PI*(x + y)) 
           - 2*(1 - exp(-(1 - y)/coeff1))*cos(M_PI*(x + y))*exp(-(1 - x)/coeff1)/coeff1 
           - (1 - exp(-(1 - x)/coeff1))*cos(M_PI*(x + y))*exp(-(1 - y)/coeff1)/coeff1;
  }

  virtual Ord ord(Ord x, Ord y) {
    return Ord(10);
  }
};

/* Exact solution */

class CustomExactSolution : public ExactSolutionScalar
{
public:
  CustomExactSolution(Mesh* mesh, double epsilon) 
        : ExactSolutionScalar(mesh), epsilon(epsilon) {};

  // Exact solution.
  double value(double x, double y) {
    return (1 - exp(-(1-x)/epsilon)) * (1 - exp(-(1-y)/epsilon)) * cos(M_PI * (x + y)); 
  };

  // Exact solution with derivatives.
  virtual scalar exact_function (double x, double y, scalar& dx, scalar& dy) {
    dx = -M_PI*(1 - exp(-(1 - x)/epsilon))*(1 - exp(-(1 - y)/epsilon))*sin(M_PI*(x + y)) 
         - (1 - exp(-(1 - y)/epsilon))*cos(M_PI*(x + y))*exp(-(1 - x)/epsilon)/epsilon;
    dy = -M_PI*(1 - exp(-(1 - x)/epsilon))*(1 - exp(-(1 - y)/epsilon))*sin(M_PI*(x + y)) 
         - (1 - exp(-(1 - x)/epsilon))*cos(M_PI*(x + y))*exp(-(1 - y)/epsilon)/epsilon;
    return value(x, y);
  };

  // Members.
  double epsilon;
};

/* Weak forms */

class CustomWeakForm : public WeakForm
{
public:
  CustomWeakForm(CustomRightHandSide* rhs) : WeakForm(1) {
    add_matrix_form(new CustomMatrixFormVol(0, 0, rhs->coeff1));
    add_vector_form(new DefaultVectorFormVolNonConst(0, rhs));
  };

private:
  class CustomMatrixFormVol : public WeakForm::MatrixFormVol
  {
  public:
    CustomMatrixFormVol(int i, int j, double epsilon) 
          : WeakForm::MatrixFormVol(i, j), epsilon(epsilon) { }

    template<typename Real, typename Scalar>
    Scalar matrix_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, 
                       Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) {
      Scalar val = 0;
      for (int i=0; i < n; i++) {
        val = val + wt[i] * epsilon * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i]);
        val = val + wt[i] * (2*u->dx[i] + u->dy[i]) * v->val[i];
      }

      return val;
    }

    scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u, 
                 Func<double> *v, Geom<double> *e, ExtData<scalar> *ext) {
      return matrix_form<scalar, scalar>(n, wt, u_ext, u, v, e, ext);
    }

    Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, 
            Geom<Ord> *e, ExtData<Ord> *ext) {
      return matrix_form<Ord, Ord>(n, wt, u_ext, u, v, e, ext);
    }

    double epsilon;
  };
};

/* Essential boundary conditions */

class EssentialBCNonConst : public EssentialBC
{
public:
  EssentialBCNonConst(std::string marker, CustomExactSolution* exact_solution) : 
        EssentialBC(Hermes::vector<std::string>()), exact_solution(exact_solution) 
  {
    markers.push_back(marker);
  };
  
  ~EssentialBCNonConst() {};

  virtual EssentialBCValueType get_value_type() const { 
    return BC_FUNCTION; 
  };

  virtual scalar function(double x, double y) const {
    return exact_solution->value(x, y);
  };

  CustomExactSolution* exact_solution;
};
