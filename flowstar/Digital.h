#include "Hybrid.h"
#include "DNN.h"
#include "DNNResets.h"

using namespace flowstar;

int reach_digital(std::list<TaylorModelVec> & flowpipes, std::list<std::vector<Interval> > & domains, std::list<int> & flowpipes_safety,
		std::list<bool> & flowpipes_contracted, long & num_of_flowpipes,
		const int mode, const Flowpipe & initFp, const double step, const double miniStep, const double time, const int order, const int precondition,
		const std::vector<Interval> & estimation, const bool bPrint, const std::vector<std::string> & stateVarNames,
		std::vector<bool> & invariant_boundary_intersected, const std::vector<std::string> & modeNames, const Interval & cutoff_threshold,
		  const std::vector<PolynomialConstraint> & unsafeSet, const bool bSafetyChecking, const bool bPlot, const bool bDump);

void digital_qr_preconditioning(NNTaylorModelVec &tmv_left, NNTaylorModelVec &tmv_right, NNTaylorModelVec &tmv_composed,
				const TaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, const std::vector<Interval> domain,
				const TaylorModelVec resetMap, const Interval cutoff_threshold);

void remove_symbolic_remainders(TaylorModelVec &result, const TaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain);

void remove_symbolic_remainders(NNTaylorModelVec &result, const NNTaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain);

void add_symbolic_remainders(NNTaylorModelVec &result, const NNTaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain);

void symbolic_remainders(NNTaylorModelVec &result, const NNTaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain, const std::vector<Interval> step_exp_table);
void test();