#include "Digital.h"

int reach_digital(std::list<TaylorModelVec> & flowpipes, std::list<std::vector<Interval> > & domains, std::list<int> & flowpipes_safety,
		std::list<bool> & flowpipes_contracted, long & num_of_flowpipes,
		const int mode, const Flowpipe & initFp, const double step, const double miniStep, const double time, const int order, const int precondition,
		const std::vector<Interval> & estimation, const bool bPrint, const std::vector<std::string> & stateVarNames,
		std::vector<bool> & invariant_boundary_intersected, const std::vector<std::string> & modeNames, const Interval & cutoff_threshold,
		const std::vector<PolynomialConstraint> & unsafeSet, const bool bSafetyChecking, const bool bPlot, const bool bDump)
{

	std::vector<Interval> step_exp_table, step_end_exp_table;
	Interval intZero;

	construct_step_exp_table(step_exp_table, step_end_exp_table, step, 2*(order+1)+1);

	flowpipes.clear();
	domains.clear();
	flowpipes_safety.clear();
	flowpipes_contracted.clear();

	int checking_result = COMPLETED_SAFE;

	bool bContracted = true;

	TaylorModelVec tmvCompo;
	initFp.composition_normal(tmvCompo, step_exp_table, cutoff_threshold);

	std::vector<Interval> new_domains;
	Interval timeDomain(0.0,0.0);
	
	new_domains.push_back(timeDomain);

	for (int i = 1; i < initFp.domain.size(); i++){
	  new_domains.push_back(initFp.domain[i]);
	}
	  

	flowpipes_contracted.push_back(bContracted);
	num_of_flowpipes += 1;


	if(bSafetyChecking)
	{

		int safety = safetyChecking2(tmvCompo, initFp.domain, unsafeSet, order, cutoff_threshold);

		if(safety == SAFE)
		{
			flowpipes.push_back(tmvCompo);
			domains.push_back(new_domains);
			flowpipes_safety.push_back(SAFE);
		}
		else if(safety == UNSAFE && !bContracted)
		{
			flowpipes.push_back(tmvCompo);
			domains.push_back(new_domains);
			flowpipes_safety.push_back(UNSAFE);

			return COMPLETED_UNSAFE;
		}
		else
		{
			flowpipes.push_back(tmvCompo);
			domains.push_back(new_domains);
			flowpipes_safety.push_back(UNKNOWN);

			if(checking_result == COMPLETED_SAFE)
			{
				checking_result = COMPLETED_UNKNOWN;
			}
		}
	}


	return checking_result;
}

void digital_qr_preconditioning(NNTaylorModelVec &tmv_left, NNTaylorModelVec &tmv_right, NNTaylorModelVec &tmv_composed,
				const TaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, const std::vector<Interval> domain,
				const TaylorModelVec resetMap, const Interval cutoff_threshold){


        NNTaylorModelVec nntmv_left, nntmv_right, nntmv_composed;


	//NB: I am converting this to _f states because the qr preconditioning function currently only works for that setup
	std::map<int, int> stateToF, fToState, fToStateQ;

        int numVars = varNames.size();
	int num_init_conds = 0;
	std::vector<int> all_conds(numVars);


	for(int ii = 0; ii < tmvAggregation.tms.size(); ii++){
	        for(auto iter = tmvAggregation.tms[ii].expansion.monomials.begin(); iter != tmvAggregation.tms[ii].expansion.monomials.end(); iter ++){
		        std::vector<int> cur_degrees = iter->getDegrees();
			for(int d = 0; d < cur_degrees.size(); d++){
			        all_conds[d] += cur_degrees[d];
			}
		}
	}
	
	for(int d = 0; d < all_conds.size(); d++){
	        if(all_conds[d] > 0){

		        fToState[num_init_conds] = d-1;//-1 for time...
			stateToF[d-1] = num_init_conds;
			num_init_conds++;

		}
	}
	
	std::vector<double> var_degrees(numVars);
	for(int ii = 0; ii < resetMap.tms.size(); ii++){
	  
	        //if identity, don't count it
	        if(resetMap.tms[ii].expansion.monomials.size() == 1){
		        auto iter = resetMap.tms[ii].expansion.monomials.begin();
			std::vector<int> cur_degrees = iter->getDegrees();
			if(iter->degree() == 1 && cur_degrees[ii+1] == 1) continue;
		}
					  
		for(auto iter = resetMap.tms[ii].expansion.monomials.begin(); iter != resetMap.tms[ii].expansion.monomials.end(); iter ++){
		        std::vector<int> cur_degrees = iter->getDegrees();
			for(int d = 0; d < cur_degrees.size(); d++){
			        if(cur_degrees[d] > 0) var_degrees[d] += 1; //count number of appearances
			}						  

		}
	}

	// std::priority_queue<int> q;
	// for (int i = 0; i < var_degrees.size(); ++i) {
	// 	q.push(var_degrees[i]);
	// 	//printf("degrees: %.18f\n", var_degrees[i]);
	// }

	// int k_largest = q.top();

	// for (int i = 0; i < num_init_conds; ++i) {
	// 	k_largest = q.top();
	// 	//printf("k: %.18f\n", k_largest);
	// 	q.pop();
	// }
	
	NNTaylorModelVec nntmvImagePrep;
	int numVarsToPrecondition = 0;
	std::vector<Interval> newDomain;
	std::vector<bool> vars_to_precondition(numVars);
	for(int d = 0; d < all_conds.size(); d++){
	  if(var_degrees[d] >= 1 && var_degrees[d] > 0){
	                NNTaylorModel nnTemp(tmvAggregation.tms[d-1], varNames); //-1 for time			
			nntmvImagePrep.tms.push_back(nnTemp);
			fToStateQ[numVarsToPrecondition] = d-1;//-1 for time...
			numVarsToPrecondition++;
			newDomain.push_back(domain[d]);
			vars_to_precondition[d] = true;

	  }
	}
	
	std::vector<std::string> new_var_names;
	new_var_names.push_back("local_t");
	for(int i = 0; i < numVarsToPrecondition; i++) new_var_names.push_back("_f" + std::to_string(i+1));

	NNTaylorModelVec nntmvImage;
	for(int i = 0; i < numVarsToPrecondition; i++){
	        NNTaylorModel tmTemp;
		dnn::convert_TM_dimension(tmTemp, nntmvImagePrep.tms[i], numVarsToPrecondition+1, i, new_var_names, stateToF);//+1 for time
		nntmvImage.tms.push_back(tmTemp);
	}
	
	Continuous_Reachability_Setting crs;        
	crs.cutoff_threshold = cutoff_threshold;	

	
	qr_preconditioning(nntmv_left, nntmv_right, nntmv_composed,
			   nntmvImage, num_init_conds,
			   varNames, newDomain, crs);

	// bring TMs back to original dimensions
	NNTaylorModelVec big_left, big_right, big_comp;
	for(int i = 0; i < nntmv_left.tms.size(); i++){
	        NNTaylorModel tmTemp_left;
		dnn::convert_TM_dimension(tmTemp_left, nntmv_left.tms[i], numVars, i, varNames, fToStateQ);
		big_left.tms.push_back(tmTemp_left);

		NNTaylorModel tmTemp_right;
		dnn::convert_TM_dimension(tmTemp_right, nntmv_right.tms[i], numVars, i, varNames, fToState);
		big_right.tms.push_back(tmTemp_right);

		NNTaylorModel tmTemp_comp;
		dnn::convert_TM_dimension(tmTemp_comp, nntmv_composed.tms[i], numVars, i, varNames, fToState);
		big_comp.tms.push_back(tmTemp_comp);
	}


	// finally, convert NNTMs to full size
	tmv_left.tms.clear();
	tmv_right.tms.clear();
	tmv_composed.tms.clear();
	int num_changed = 0;

	for(int i = 0; i < tmvAggregation.tms.size(); i++){
	        if(vars_to_precondition[i+1] && numVarsToPrecondition >= 2 && num_init_conds >= 2){

			tmv_left.tms.push_back(big_left.tms[num_changed]);

			tmv_right.tms.push_back(big_right.tms[num_changed]);

			tmv_composed.tms.push_back(big_comp.tms[num_changed]);
			
			num_changed++;
		}
		else{ //identity precondition the rest

		        std::vector<int> degs(numVars, 0);
			degs[i+1] = 1;
			Monomial mCur(Interval(1), degs);
			Polynomial pCur(mCur);
			TaylorModel tmCur(pCur);

			tmv_right.tms.push_back(NNTaylorModel(tmCur, varNames));
			tmv_left.tms.push_back(NNTaylorModel(tmvAggregation.tms[i], varNames));
			tmv_composed.tms.push_back(NNTaylorModel(tmvAggregation.tms[i], varNames));
		}
	}
}


// NB: domain for result is normalized to [-1,1]
void add_symbolic_remainders(NNTaylorModelVec &result, const NNTaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain){

        result.tms.clear();

	int num_vars = varNames.size();

	for(int i = 0; i < tmvAggregation.tms.size(); i++){
	        NNTaylorModel tmTemp(tmvAggregation.tms[i]);

		if(states_to_change[i]){
		        std::vector<int> new_degrees(num_vars);
			new_degrees[i+1] = 1;

			// Rado: My understanding is that these functions mean:
			// temp is now 0-centered with same width as tmvAggregation.tms[i].remainder
			// M is the constant the was removed to make temp 0-centered
			Interval temp = tmvAggregation.tms[i].remainder;
			Interval M;
			temp.remove_midpoint(M);

			Interval new_coef;
			temp.mag(new_coef);

			std::shared_ptr<NNMonomial> ptr_mono_const(new NNMonomial(M, num_vars));
			std::shared_ptr<NNMonomial> ptr_mono(new NNMonomial(new_coef, new_degrees));

			tmTemp.expansion.add_assign(ptr_mono_const);
			tmTemp.expansion.add_assign(ptr_mono);			
			
			tmTemp.remainder = Interval(0.0, 0.0);
		}

		result.tms.push_back(tmTemp);
	}
}

// not changing the domains -- assuming they're all [-1,1]
void remove_symbolic_remainders(TaylorModelVec &result, const TaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain){

        result.tms.clear();

	NNTaylorModelVec nntmvAggregation, nnresult;
	
	for(int i = 0; i < tmvAggregation.tms.size(); i++){
	        NNTaylorModel tmTemp(tmvAggregation.tms[i], varNames);
		nntmvAggregation.tms.push_back(tmTemp);
	}

	remove_symbolic_remainders(nnresult, nntmvAggregation, varNames, states_to_change, domain);

	for(int i = 0; i < nnresult.tms.size(); i++){
	        TaylorModel tmTemp(nnresult.tms[i]);
		result.tms.push_back(tmTemp);
	}

}


// not changing the domains -- assuming they're all [-1,1]
void remove_symbolic_remainders(NNTaylorModelVec &result, const NNTaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain){

        result.tms.clear();

	int num_vars = varNames.size();
	
	for(int i = 0; i < tmvAggregation.tms.size(); i++){
	        NNTaylorModel tmTemp;
		tmTemp.remainder = tmvAggregation.tms[i].remainder;
		NNPolynomial tempP;
	        for(auto iter = tmvAggregation.tms[i].expansion.monomials_map.begin();
		    iter != tmvAggregation.tms[i].expansion.monomials_map.end(); iter++){

			bool no_z = true;
			Interval z_mono_range = iter->second->getCoefficient();
			std::map<int, int> mono_degrees = iter->second->getDegrees();
			for(auto iter_degrees = mono_degrees.begin(); iter_degrees != mono_degrees.end(); iter_degrees++){
				if(states_to_change[iter_degrees->first-1]){ //this if case will break if the monomial has time (shouldn't be true)
				        no_z = false;
				}
			}

			if(no_z){
			        std::shared_ptr<NNMonomial> ptr_mono(new NNMonomial(iter->second));
				tmTemp.expansion.add_assign(ptr_mono);
			}
			else{ // add to remainder
			        std::shared_ptr<NNMonomial> ptr_mono(new NNMonomial(iter->second));
				tempP.add_assign(ptr_mono);
			}

		        
		}

		if(tempP.monomials_map.size() > 0){
		        Interval new_bounds;
			tempP.intEvalNormal(new_bounds, domain);//second variable isn't used     
			Interval M;
			new_bounds.remove_midpoint(M);
			std::shared_ptr<NNMonomial> ptr_mono_const(new NNMonomial(M, num_vars));
			tmTemp.expansion.add_assign(ptr_mono_const);
			tmTemp.remainder += new_bounds;
		}
		result.tms.push_back(tmTemp);
	}
}


void symbolic_remainders(NNTaylorModelVec &result, const NNTaylorModelVec &tmvAggregation, const std::vector<std::string> varNames, std::vector<bool> states_to_change, const std::vector<Interval> domain, const std::vector<Interval> step_exp_table){
  
        std::vector<Interval> all_ranges;
	tmvAggregation.polyRange(all_ranges, domain);
	
	bool largeRemainder = false;
	
	//NB: this only works for state names that begin with z
	for(int varInd = 0; varInd < tmvAggregation.tms.size(); varInd++){
	        if(states_to_change[varInd]){
			if(tmvAggregation.tms[varInd].remainder.width() > 0.000001 &&
			   tmvAggregation.tms[varInd].remainder.width() > 0.01 * all_ranges[varInd].width()){
						  
			  largeRemainder = true;
			}
		}
	}

	if(largeRemainder){
	        //first, consolidate current symbolic and non-symbolic remainders
	        NNTaylorModelVec no_sym_tm;
		remove_symbolic_remainders(no_sym_tm, tmvAggregation, varNames, states_to_change, domain);

		//then, add remainders symbolically again
		add_symbolic_remainders(result, no_sym_tm, varNames, states_to_change, domain);

	}
	else{
	        result = tmvAggregation;
	}
}
void test() {
	printf("testing");
	return;
}
