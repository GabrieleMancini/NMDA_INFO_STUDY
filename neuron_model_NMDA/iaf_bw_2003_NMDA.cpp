/*
 *  iaf_bw_2003_NMDA.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "iaf_bw_2003_NMDA.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

using namespace nest;

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< mynest::iaf_bw_2003_NMDA >
  mynest::iaf_bw_2003_NMDA::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< mynest::iaf_bw_2003_NMDA >::create()
{
  // use standard names whereever you can for consistency!
  insert_(
    names::V_m, &mynest::iaf_bw_2003_NMDA::get_y_elem_< mynest::iaf_bw_2003_NMDA::State_::V_M > );
  insert_(
    names::g_ex, &mynest::iaf_bw_2003_NMDA::get_y_elem_< mynest::iaf_bw_2003_NMDA::State_::G_EXC > );
  insert_(
    names::g_in, &mynest::iaf_bw_2003_NMDA::get_y_elem_< mynest::iaf_bw_2003_NMDA::State_::G_INH > );
  insert_(
    names::g_NMDA, &mynest::iaf_bw_2003_NMDA::get_y_elem_< mynest::iaf_bw_2003_NMDA::State_::G_EXC_NMDA > );
  insert_( names::t_ref_remaining, &mynest::iaf_bw_2003_NMDA::get_r_ );
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" inline int
mynest::iaf_bw_2003_NMDA_dynamics( double,
  const double y[],
  double f[],
  void* pnode )
{
  // a shorthand
  typedef mynest::iaf_bw_2003_NMDA::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const mynest::iaf_bw_2003_NMDA& node =
    *( reinterpret_cast< mynest::iaf_bw_2003_NMDA* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...
  const double& V = y[ S::V_M ];

  const double I_syn_inh = y[ S::G_INH ] * ( V - node.P_.E_in );
  const double I_syn_exc = (node.P_.NMDA*y[ S::G_EXC_NMDA ] / (1 + std::exp(-0.062*V)/3.57) +
                            y[ S::G_EXC]) * ( V - node.P_.E_ex );
  const double I_leak = node.P_.g_L * ( V - node.P_.E_L );

  // dV_m/dt
  f[ 0 ] = ( -I_leak - I_syn_exc - I_syn_inh + node.B_.I_stim_ + node.P_.I_e)
    / node.P_.C_m;

  // AMPA: d dg_exc/dt, dg_exc/dt
  f[ 1 ] = -y[ S::DG_EXC ] / node.P_.tau_decay_AMPA;
  f[ 2 ] =  y[ S::DG_EXC ] - ( y[ S::G_EXC ] / node.P_.tau_rise_AMPA );

  // GABA: d dg_inh/dt, dg_inh/dt
  f[ 3 ] = -y[ S::DG_INH ] / node.P_.tau_decay_GABA_A;
  f[ 4 ] = y[ S::DG_INH ] - ( y[ S::G_INH ] / node.P_.tau_rise_GABA_A );

  // NMDA: d dg_inh/dt, dg_inh/dt
  f[ 5 ] = -y[ S::DG_EXC_NMDA ] / node.P_.tau_decay_NMDA;
  f[ 6 ] =  y[ S::DG_EXC_NMDA ] - ( y[ S::G_EXC_NMDA ] / node.P_.tau_rise_NMDA );

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

mynest::iaf_bw_2003_NMDA::Parameters_::Parameters_()
  : V_th( -52.0 )       // mV
  , V_reset( -59.0 )    // mV
  , t_ref( 2.0 )        // ms
  , g_L( 25.0 )      // nS
  , C_m( 500.0 )        // pF
  , E_ex( 0.0 )         // mV
  , E_in( -80.0 )       // mV
  , E_L( -70.0 )        // mV
  , tau_rise_AMPA( 0.4 )  // ms
  , tau_decay_AMPA( 2.0 ) // ms
  , tau_rise_GABA_A( 0.25 )  // ms
  , tau_decay_GABA_A( 5.0 ) // ms
  , tau_m( 20.0 ) // ms
  , tau_rise_NMDA( 4.0 )  // ms
  , tau_decay_NMDA( 40.0 ) // ms
  , S_act_NMDA( 0.5 )
  , NMDA(1.0) // unitless
  , I_e( 0.0 )          // pA

{
}

mynest::iaf_bw_2003_NMDA::State_::State_( const Parameters_& p )
  : r( 0 )
{
  y[ V_M ] = p.E_L; // initialize to reversal potential
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = 0;
  }
}

mynest::iaf_bw_2003_NMDA::State_::State_( const State_& s )
  : r( s.r )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = s.y[ i ];
  }
}

mynest::iaf_bw_2003_NMDA::State_& mynest::iaf_bw_2003_NMDA::State_::operator=(
  const State_& s )
{
  if ( this == &s ) // avoid assignment to self
  {
    return *this;
  }
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y[ i ] = s.y[ i ];
  }

  r = s.r;
  return *this;
}

mynest::iaf_bw_2003_NMDA::Buffers_::Buffers_( mynest::iaf_bw_2003_NMDA& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

mynest::iaf_bw_2003_NMDA::Buffers_::Buffers_( const Buffers_&, mynest::iaf_bw_2003_NMDA& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
mynest::iaf_bw_2003_NMDA::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::V_reset, V_reset );
  def< double >( d, names::t_ref, t_ref );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::tau_rise_AMPA, tau_rise_AMPA );
  def< double >( d, names::tau_decay_AMPA, tau_decay_AMPA );
  def< double >( d, names::tau_rise_GABA_A, tau_rise_GABA_A );
  def< double >( d, names::tau_decay_GABA_A, tau_decay_GABA_A );
  def< double >( d, names::tau_m, tau_m );
  def< double >( d, names::tau_rise_NMDA, tau_rise_NMDA );
  def< double >( d, names::tau_decay_NMDA, tau_decay_NMDA );
  def< double >( d, names::S_act_NMDA, S_act_NMDA );
  def< double >( d, names::NMDA, NMDA );
  def< double >( d, names::I_e, I_e );
}

void
mynest::iaf_bw_2003_NMDA::Parameters_::set( const DictionaryDatum& d )
{
  // allow setting the membrane potential
  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::V_reset, V_reset );
  updateValue< double >( d, names::t_ref, t_ref );
  updateValue< double >( d, names::E_L, E_L );

  updateValue< double >( d, names::E_ex, E_ex );
  updateValue< double >( d, names::E_in, E_in );

  updateValue< double >( d, names::C_m, C_m );
  updateValue< double >( d, names::g_L, g_L );

  updateValue< double >( d, names::tau_rise_AMPA, tau_rise_AMPA );
  updateValue< double >( d, names::tau_decay_AMPA, tau_decay_AMPA );
  updateValue< double >( d, names::tau_rise_GABA_A, tau_rise_GABA_A );
  updateValue< double >( d, names::tau_decay_GABA_A, tau_decay_GABA_A );
  updateValue< double >( d, names::tau_m, tau_m );
  updateValue< double >( d, names::tau_rise_NMDA, tau_rise_NMDA );
  updateValue< double >( d, names::tau_decay_NMDA, tau_decay_NMDA );
  updateValue< double >( d, names::S_act_NMDA, S_act_NMDA );
  updateValue< double >( d, names::NMDA, NMDA );

  updateValue< double >( d, names::I_e, I_e );
  if ( V_reset >= V_th )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( t_ref < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }
  if ( NMDA < 0 )
  {
    throw BadProperty( "NMDA factor cannot be negative." );
  }
  if ( tau_rise_AMPA <= 0 || tau_decay_AMPA <= 0 || tau_rise_GABA_A <= 0
    || tau_decay_GABA_A <= 0 || tau_rise_NMDA <= 0 || tau_decay_NMDA <= 0 ||
    tau_m <= 0)
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

void
mynest::iaf_bw_2003_NMDA::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y[ V_M ] ); // Membrane potential
}

void
mynest::iaf_bw_2003_NMDA::State_::set( const DictionaryDatum& d, const Parameters_& )
{
  updateValue< double >( d, names::V_m, y[ V_M ] );
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

mynest::iaf_bw_2003_NMDA::iaf_bw_2003_NMDA()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

mynest::iaf_bw_2003_NMDA::iaf_bw_2003_NMDA( const mynest::iaf_bw_2003_NMDA& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

mynest::iaf_bw_2003_NMDA::~iaf_bw_2003_NMDA()
{
  // GSL structs may not have been allocated, so we need to protect destruction
  if ( B_.s_ )
  {
    gsl_odeiv_step_free( B_.s_ );
  }
  if ( B_.c_ )
  {
    gsl_odeiv_control_free( B_.c_ );
  }
  if ( B_.e_ )
  {
    gsl_odeiv_evolve_free( B_.e_ );
  }
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
mynest::iaf_bw_2003_NMDA::init_state_( const Node& proto )
{
  const mynest::iaf_bw_2003_NMDA& pr = downcast< mynest::iaf_bw_2003_NMDA >( proto );
  S_ = pr.S_;
}

void
mynest::iaf_bw_2003_NMDA::init_buffers_()
{
  ArchivingNode::clear_history();

  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  // includes resize

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( B_.s_ == 0 )
  {
    B_.s_ =
      gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }

  if ( B_.c_ == 0 )
  {
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );
  }

  if ( B_.e_ == 0 )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = iaf_bw_2003_NMDA_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

double
mynest::iaf_bw_2003_NMDA::get_normalisation_factor( double tau_rise,
  double tau_decay , double tau_m)
{
  /* The solution to the beta function ODE obtained by the solver is
   *
   *   g(t) = c / ( a - b ) * ( e^(-b t) - e^(-a t) )
   *
   * with a = 1/tau_rise, b = 1/tau_decay, a != b. The maximum of this function is at
   *
   *   t* = 1/(a-b) ln a/b
   *
   * We want to scale the function so that
   *
   *   max g == g(t*) == g_peak
   *
   * We thus need to set
   *
   *   c = g_peak * ( a - b ) / ( e^(-b t*) - e^(-a t*) )
   *
   * See Rotter & Diesmann, Biol Cybern 81:381 (1999) and Roth and van Rossum,
   * Ch 6, in De Schutter, Computational Modeling Methods for Neuroscientists,
   * MIT Press, 2010.
   */

  const double t_peak =
    ( tau_decay * tau_rise ) * std::log( tau_decay / tau_rise ) / ( tau_decay - tau_rise );

  const double prefactor = ( 1 / tau_rise ) - ( 1 / tau_decay );

  const double peak_value =
    ( std::exp( -t_peak / tau_decay ) - std::exp( -t_peak / tau_rise ) );

  // The peak value is adjusted according to reference [1]
  const double g_peak = (tau_m/tau_decay) * pow(tau_rise/tau_decay,tau_rise/(tau_decay-tau_rise));

  return  g_peak * prefactor / peak_value;

}

void
mynest::iaf_bw_2003_NMDA::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.PSConInit_E = mynest::iaf_bw_2003_NMDA::get_normalisation_factor(
    P_.tau_rise_AMPA, P_.tau_decay_AMPA , P_.tau_m);
  V_.PSConInit_E_NMDA = mynest::iaf_bw_2003_NMDA::get_normalisation_factor(
    P_.tau_rise_NMDA, P_.tau_decay_NMDA , P_.tau_m);
  V_.PSConInit_I = mynest::iaf_bw_2003_NMDA::get_normalisation_factor(
    P_.tau_rise_GABA_A, P_.tau_decay_GABA_A , P_.tau_m);
  V_.RefractoryCounts = Time( Time::ms( P_.t_ref ) ).get_steps();

  // since t_ref >= 0, this can only fail in error
  assert( V_.RefractoryCounts >= 0 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
mynest::iaf_bw_2003_NMDA::update( Time const& origin,
  const long from,
  const long to )
{

  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {

    double t = 0.0;

    // numerical integration with adaptive step size control:
    // ------------------------------------------------------
    // gsl_odeiv_evolve_apply performs only a single numerical
    // integration step, starting from t and bounded by step;
    // the while-loop ensures integration over the whole simulation
    // step (0, step] if more than one integration step is needed due
    // to a small integration step size;
    // note that (t+IntegrationStep > step) leads to integration over
    // (t, step] and afterwards setting t to step, but it does not
    // enforce setting IntegrationStep to step-t; this is of advantage
    // for a consistent and efficient integration across subsequent
    // simulation intervals
    while ( t < B_.step_ )
    {
      const int status = gsl_odeiv_evolve_apply( B_.e_,
        B_.c_,
        B_.s_,
        &B_.sys_,             // system of ODE
        &t,                   // from t
        B_.step_,             // to t <= step
        &B_.IntegrationStep_, // integration step size
        S_.y );               // neuronal state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    // refractoriness and spike generation
    if ( S_.r )
    { // neuron is absolute refractory
      --S_.r;
      S_.y[ State_::V_M ] = P_.V_reset; // clamp potential
    }
    else
      // neuron is not absolute refractory
      if ( S_.y[ State_::V_M ] >= P_.V_th )
    {
      S_.r = V_.RefractoryCounts;
      S_.y[ State_::V_M ] = P_.V_reset;

      // log spike with ArchivingNode
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // add incoming spikes
    V_.sp = B_.spike_exc_.get_value( lag );

    S_.y[ State_::DG_EXC ] += (1.0 - P_.S_act_NMDA)*V_.sp * V_.PSConInit_E;
    S_.y[ State_::DG_EXC_NMDA ] += P_.S_act_NMDA*V_.sp * V_.PSConInit_E_NMDA;
    S_.y[ State_::DG_INH ] += B_.spike_inh_.get_value( lag ) * V_.PSConInit_I;

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
mynest::iaf_bw_2003_NMDA::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.spike_inh_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      -e.get_weight() * e.get_multiplicity() );
  } // ensure conductance is positive
}

void
mynest::iaf_bw_2003_NMDA::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_current() );
}

void
mynest::iaf_bw_2003_NMDA::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
