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

#ifndef __sot_dyninv_TaskDynJointLimits_H__
#define __sot_dyninv_TaskDynJointLimits_H__
/* --------------------------------------------------------------------- */
/* --- API ------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#if defined (WIN32)
#  if defined (task_dyn_joint_limits_EXPORTS)
#    define SOTTASKDYNJOINTLIMITS_EXPORT __declspec(dllexport)
#  else
#    define SOTTASKDYNJOINTLIMITS_EXPORT __declspec(dllimport)
#  endif
#else
#  define SOTTASKDYNJOINTLIMITS_EXPORT
#endif

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

/* SOT */
#include <sot-dyninv/signal-helper.h>
#include <sot-dyninv/entity-helper.h>
#include <sot-dyninv/task-dyn-pd.h>
#include <sot/core/task.hh>

//#include <sot/sotFeatureAbstract.h>
//#include <sot/sotFlags.h>
//#include <sot/sotExceptionTask.h>


namespace dynamicgraph {
  namespace sot {
    namespace dyninv {

      /* --------------------------------------------------------------------- */
      /* --- CLASS ----------------------------------------------------------- */
      /* --------------------------------------------------------------------- */


      class SOTTASKDYNJOINTLIMITS_EXPORT TaskDynJointLimits
	:public ::dynamicgraph::sot::dyninv::TaskDynPD
	,public ::dynamicgraph::EntityHelper<TaskDynJointLimits>
	{
	  DYNAMIC_GRAPH_ENTITY_DECL();
	public: /* --- CONSTRUCTOR ---- */

	  TaskDynJointLimits( const std::string& name );

	public: /* --- ENTITY INHERITANCE --- */
	  typedef TaskDynJointLimits EntityClassName;
	  //virtual void display( std::ostream& os ) const;

	public:  /* --- SIGNALS --- */

	  DECLARE_SIGNAL_IN(position,dg::Vector);
	  DECLARE_SIGNAL_IN(velocity,dg::Vector);
	  DECLARE_SIGNAL_IN(referenceInf,dg::Vector);
	  DECLARE_SIGNAL_IN(referenceSup,dg::Vector);
	  //DECLARE_SIGNAL_IN(dt,double);

	  DECLARE_SIGNAL_OUT(normalizedPosition,dg::Vector);

	public:  /* --- COMPUTATION --- */
	  dg::sot::VectorMultiBound& computeTaskDynJointLimits( dg::sot::VectorMultiBound& res,int time );
	  dg::Matrix& computeTjlJacobian( dg::Matrix& J,int time );
	  dg::Matrix& computeTjlJdot( dg::Matrix& Jdot,int time );

	  //protected:
	  //dynamicgraph::sot::VectorMultiBound&
	  //  taskSOUT_function( dynamicgraph::sot::VectorMultiBound& task,int iter );
	  //std::list< sotFeatureAbstract* > featureList;

	protected:
	  dg::Matrix previousJ;
	  bool previousJset;

	private:   /* --- DISPLAY --- */
	  void display( std::ostream& os ) const;

	}; // class TaskDynJointLimits

    } // namespace dyninv
  } // namespace sot
} // namespace dynamicgraph


#endif // #ifndef __sot_dyninv_TaskDynJointLimits_H__
