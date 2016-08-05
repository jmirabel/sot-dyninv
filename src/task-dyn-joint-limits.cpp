/*
 * Copyright 2011, Nicolas Mansard, LAAS-CNRS
 *
 * This file is part of sot-dyninv.
 * sot-dyninv is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * sot-dyninv is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.  You should
 * have received a copy of the GNU Lesser General Public License along
 * with sot-dyninv.  If not, see <http://www.gnu.org/licenses/>.
 */


/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

//#define VP_DEBUG
//#define VP_DEBUG_MODE 15
#include <sot/core/debug.hh>

#include <dynamic-graph/factory.h>

#include <sot/core/feature-abstract.hh>

#include <sot-dyninv/task-dyn-joint-limits.h>
#include <sot-dyninv/commands-helper.h>

#include <boost/foreach.hpp>


/* --------------------------------------------------------------------- */
/* --- CLASS ----------------------------------------------------------- */
/* --------------------------------------------------------------------- */

namespace dynamicgraph
{
  namespace sot
  {
    namespace dyninv
    {

      namespace dg = ::dynamicgraph;

      /* --- DG FACTORY ------------------------------------------------------- */
      DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(TaskDynJointLimits,"TaskDynJointLimits");

      /* ---------------------------------------------------------------------- */
      /* --- CONSTRUCTION ----------------------------------------------------- */
      /* ---------------------------------------------------------------------- */

      TaskDynJointLimits::
      TaskDynJointLimits( const std::string & name )
	: TaskDynPD(name)

	,CONSTRUCT_SIGNAL_IN(position,dg::Vector)
	,CONSTRUCT_SIGNAL_IN(velocity,dg::Vector)
	,CONSTRUCT_SIGNAL_IN(referenceInf,dg::Vector)
	,CONSTRUCT_SIGNAL_IN(referenceSup,dg::Vector)

	,CONSTRUCT_SIGNAL_OUT(normalizedPosition,dg::Vector,positionSIN<<referenceInfSIN<<referenceSupSIN)

	,previousJ(0u,0u),previousJset(false)
      {
	taskSOUT.setFunction( boost::bind(&TaskDynJointLimits::computeTaskDynJointLimits,this,_1,_2) );
	jacobianSOUT.setFunction( boost::bind(&TaskDynJointLimits::computeTjlJacobian,this,_1,_2) );
	JdotSOUT.setFunction( boost::bind(&TaskDynJointLimits::computeTjlJdot,this,_1,_2) );
	taskSOUT.clearDependencies();
	taskSOUT.addDependency( referenceSupSIN );
	taskSOUT.addDependency( referenceInfSIN );
	taskSOUT.addDependency( dtSIN );
	taskSOUT.addDependency( positionSIN );
	taskSOUT.addDependency( velocitySIN );

	signalRegistration( referenceSupSIN << dtSIN << referenceInfSIN
			    << positionSIN << velocitySIN << normalizedPositionSOUT );
      }

      /* ---------------------------------------------------------------------- */
      /* --- COMPUTATION ------------------------------------------------------ */
      /* ---------------------------------------------------------------------- */

      dg::sot::VectorMultiBound& TaskDynJointLimits::
      computeTaskDynJointLimits( dg::sot::VectorMultiBound& res,int time )
      {
	sotDEBUGIN(45);

	sotDEBUG(45) << "# In " << getName() << " {" << std::endl;
	const dg::Vector & position = positionSIN(time);
	sotDEBUG(35) << "position = " << position << std::endl;
	const dg::Vector & velocity = velocitySIN(time);
	sotDEBUG(35) << "velocity = " << velocity << std::endl;
	const dg::Vector & refInf = referenceInfSIN(time);
	const dg::Vector & refSup = referenceSupSIN(time);
	const double & dt = dtSIN(time);
	const double kt=2/(dt*dt);

	res.resize(position.size());
	for( unsigned int i=0;i<res.size();++i )
	  {
	    res[i] = dg::sot::MultiBound (kt*(refInf(i)-position(i)-dt*velocity(i)),
					  kt*(refSup(i)-position(i)-dt*velocity(i)));
	  }

	sotDEBUG(15) << "taskU = "<< res << std::endl;
	sotDEBUGOUT(45);
	return res;
      }

      dg::Matrix& TaskDynJointLimits::
      computeTjlJacobian( dg::Matrix& J,int time )
      {
	sotDEBUG(15) << "# In {" << std::endl;

	const dg::Vector& position = positionSIN(time);
	/*
	  if( featureList.empty())
	  { throw( sotExceptionTask(sotExceptionTask::EMPTY_LIST,
	  "Empty feature list") ) ; }

	  try {
	  unsigned int dimJ = J .rows();
	  unsigned int nbc = J.cols();
	  if( 0==dimJ ){ dimJ = 1; J.resize(dimJ,nbc); }
	*/
	J.resize(position.size(),position.size());
	J.setZero();
	{
	  for  ( unsigned int j=6;j<position.size();++j )
	    J(j,j)=1;
	}

	//  catch SOT_RETHROW;
	sotDEBUG(15) << "# Out }" << std::endl;
	return J;
      }

      dg::Matrix& TaskDynJointLimits::
      computeTjlJdot( dg::Matrix& Jdot,int time )
      {
	sotDEBUGIN(15);

	const dg::Matrix& currentJ = jacobianSOUT(time);
	const double& dt = dtSIN(time);

	if( previousJ.rows()!=currentJ.rows() ) previousJset = false;

	if( previousJset )
	  {
	    assert( currentJ.rows()==previousJ.rows()
		    && currentJ.cols()==previousJ.cols() );

	    Jdot .resize( currentJ.rows(),currentJ.cols() );
	    Jdot = currentJ - previousJ;
	    Jdot *= 1/dt;
	  }
	else { Jdot.resize(currentJ.rows(),currentJ.cols() ); Jdot.setZero(); }

	previousJ = currentJ;
	previousJset = true;

	sotDEBUGOUT(15);
	return Jdot;
      }

      dg::Vector& TaskDynJointLimits::
      //computeNormalizedPosition( dg::Vector& res, int time )
      normalizedPositionSOUT_function( dg::Vector& res, int time )
      {
	const dg::Vector & position = positionSIN(time);
	//  const dg::Vector & velocity = velocitySIN(time);
	const dg::Vector & refInf = referenceInfSIN(time);
	const dg::Vector & refSup = referenceSupSIN(time);

	const unsigned int & n = position.size();
	res.resize( n );
	assert( n==refInf.size() && n==refSup.size() );
	for( unsigned int i=0;i<n;++i )
	  {
	    res(i) = ( position(i)-refInf(i) ) / ( refSup(i)-refInf(i) );
	  }
	return res;
      }

      /* ------------------------------------------------------------------------ */
      /* --- DISPLAY ENTITY ----------------------------------------------------- */
      /* ------------------------------------------------------------------------ */

      void TaskDynJointLimits::
      display( std::ostream& os ) const
      {
	os << "TaskDynJointLimits " << name << ": " << std::endl;
      }

      /*
      void TaskDynJointLimits::
      display( std::ostream& os ) const
      {
	os << "TaskDynJointLimits " << name << ": " << std::endl;
	os << "--- LIST ---  " << std::endl;
	BOOST_FOREACH( dg::sot::FeatureAbstract* feature,featureList )
	  {
	    os << "-> " << feature->getName() << std::endl;
	  }
      }
      */

    } // namespace dyninv
  } // namespace sot
} // namespace dynamicgraph

